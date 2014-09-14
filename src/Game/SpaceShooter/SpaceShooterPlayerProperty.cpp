/// Emil Hedemalm
/// 2014-07-25
/// Player properties for a space-shooter game.
/// Applicable to both human and "enemy"/AI-players.

#include "SpaceShooterPlayerProperty.h"

#include "Entity/Entity.h"

#include "Game/SpaceShooter/SpaceShooter.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/Shapes/Ray.h"

#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"

#include "Input/InputManager.h"
#include "Window/WindowManager.h"

SpaceShooterPlayerProperty::SpaceShooterPlayerProperty(SpaceShooter * game, Entity * owner)
	: EntityProperty("SpaceShooterPlayerProperty", ID(), owner), game(game), weaponType(SpaceShooterWeaponType::RAILGUN)
{
	allied = false;
	sleeping = false;
	isPlayer = false;

	millisecondsPassedSinceLastFire = 0;

	useMouseInput = false;
}

int SpaceShooterPlayerProperty::ID()
{
	return EntityPropertyID::MINI_GAME_3 + 0;
}

void SpaceShooterPlayerProperty::Remove()
{
	if (sleeping)
		return;
	Graphics.QueueMessage(new GMUnregisterEntity(owner));
	Physics.QueueMessage(new PMUnregisterEntity(owner));
	sleeping = true;
}
	
/// D:
void SpaceShooterPlayerProperty::Destroy()
{
	if (sleeping)
		return;
	Remove();
}

// Reset sleep.
void SpaceShooterPlayerProperty::OnSpawn()
{
	sleeping = false;
	weaponType = SpaceShooterWeaponType(SpaceShooterWeaponType::RAILGUN);
	weaponType.initialVelocity = 65.f;
	if (isPlayer)
		weaponType.coolDown /= 2.f;
}
	

/// Time passed in seconds..!
void SpaceShooterPlayerProperty::Process(int timeInMs)
{
	if (sleeping)
		return;

	// If game is paused.
	if (game->paused)
		return;

	if (useMouseInput)
	{
		// Grab mouse co-odinates.
		Vector2i mousePos = Input.GetMousePosition();

		// Project them onto the relevant window.
		Ray ray;
		Window * window = HoverWindow();
		bool good = false;
		if (window)
			good = window->GetRayFromScreenCoordinates(mousePos, ray);
		if (good)
		{
			// Set position
			Vector3f position = ray.start;
			Physics.QueueMessage(new PMSetEntity(owner, PT_SET_POSITION, position));
		}
	}

	// Sleep if outside frame too.
	if (game->IsPositionOutsideFrame(owner->position))
	{
		// If player, just move it inside the frame?
		if (isPlayer)
		{
		
		}
		// Enemy? Destroy it?
		else if (owner->position.x < -game->gameSize.x * 0.5f - owner->scale.MaxPart())
		{
			// Sleep
			Remove();
			game->OnPlayerDestroyed(owner);
			return;
		}
	}

	// Get look-at direction.
	Vector4f minusZ(0,0,-1,0);
	Vector4f lookAt = owner->rotationMatrix.Product(minusZ);
//	std::cout<<"\nLook at: "<<lookAt;

	/// Check if within active game area.
	/// Don't fire anything if not.
	if (owner->position.x > game->right + 10.f)
		return;

	millisecondsPassedSinceLastFire += timeInMs;
	if (isPlayer)
	{
//		std::cout<<"\nMilliseconds since last fire: "<<millisecondsPassedSinceLastFire;
	}
	if (millisecondsPassedSinceLastFire < weaponType.coolDown)
		return;

	if (isPlayer)
	;//	std::cout<<"\nPew!";

	millisecondsPassedSinceLastFire = 0;

	// Spawn a projectile? Or senda message to do so?
	Vector3f position = owner->position;
	position += lookAt * owner->scale.MaxPart();

	Entity * projectile = game->NewProjectile(weaponType, position);
	
	// If not ok request, skip it then.
	if (!projectile)
	{
		if (isPlayer)
			std::cout<<"..No?";
		return;
	}

	//	MapMan.CreateEntity("Projectile", ModelMan.GetModel("Cube"), TexMan.GetTexture("Red"));


	Physics.QueueMessage(new PMSetEntity(projectile, PT_POSITION, position));
	/// Add own velocity to the ammo velocity?
	Vector3f initialVelocity = lookAt * weaponType.initialVelocity;
	if (owner && owner->physics)
		initialVelocity += owner->physics->velocity;
	Physics.QueueMessage(new PMSetEntity(projectile, PT_VELOCITY, initialVelocity));

	game->SetupCollisionFilter(projectile, allied);
}


/// If reacting to collisions...
void SpaceShooterPlayerProperty::OnCollision(Collision & data)
{
	// Check what we are colliding with.
	Entity * other = 0;
	if (data.one == owner)
		other = data.two;
	else if (data.two == owner)
		other = data.one;

	SpaceShooterPlayerProperty * sspp = (SpaceShooterPlayerProperty *) other->GetProperty(SpaceShooterPlayerProperty::ID());
	
	// Player-player collision? Sleep 'em both.
	if (sspp)
	{
		Destroy();
		sspp->Destroy();
		game->OnPlayerDestroyed(owner);
		return;
	}

	SpaceShooterProjectileProperty * sspp2 = (SpaceShooterProjectileProperty *) other->GetProperty(SpaceShooterProjectileProperty::ID());
	if (sspp2)
	{
		// Take damage? D:
		// Die?!
		Destroy();
		game->OnPlayerDestroyed(owner);
	}
}
	
