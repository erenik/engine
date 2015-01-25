/// Emil Hedemalm
/// 2014-07-25
/// Player properties for a space-shooter game.
/// Applicable to both human and "enemy"/AI-players.

#include "SpaceShooterPlayerProperty.h"
#include "SpaceShooterExplosionProperty.h"

#include "Entity/Entity.h"

#include "Game/SpaceShooter/SpaceShooter.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/Shapes/Ray.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
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

	score = 100;
	hp = maxHP = 100;
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
	hp = 0;
	if (allied)
	{
		game->UpdatePlayerHP();
	}
	else {
		game->score += this->score;
		game->OnScoreUpdated();
	}

	// Explode
	Entity * explosionEntity = game->NewExplosion(owner->position, ExplosionType::SHIP);
	game->explosions.Add(explosionEntity);

	// Update the game that we died. T_T
	game->OnPlayerDestroyed(this->owner);
}

// Reset sleep.
void SpaceShooterPlayerProperty::OnSpawn()
{
	hp = maxHP;
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
		if ((game->flipX > 0 && owner->position[0] < -game->gameSize[0] * 0.5f - owner->scale.MaxPart()) ||
			game->flipX < 0 && owner->position[0] > game->gameSize[0] * 0.5f + owner->scale.MaxPart())
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
	if (owner->position[0] > game->right + 10.f)
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

	/// Add own velocity to the ammo velocity?
	Vector3f initialVelocity = lookAt * weaponType.initialVelocity;
	if (owner && owner->physics)
		initialVelocity += owner->physics->velocity;
	Entity * projectile = game->NewProjectile(weaponType, position, initialVelocity);
	
	// If not ok request, skip it then.
	if (!projectile)
	{
		if (isPlayer)
			std::cout<<"..No?";
		return;
	}

	//	MapMan.CreateEntity("Projectile", ModelMan.GetModel("Cube"), TexMan.GetTexture("Red"));


	Physics.QueueMessage(new PMSetEntity(projectile, PT_POSITION, position));
	
	Physics.QueueMessage(new PMSetEntity(projectile, PT_VELOCITY, initialVelocity));

	game->SetupCollisionFilter(projectile, allied);
}


/// If reacting to collisions...
void SpaceShooterPlayerProperty::OnCollision(Collision & data)
{
	if (sleeping)
		return;
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
		return;
	}

	SpaceShooterProjectileProperty * sspp2 = (SpaceShooterProjectileProperty *) other->GetProperty(SpaceShooterProjectileProperty::ID());
	if (sspp2)
	{
		// Take damage? D:
		this->Damage(sspp2->type);
	}
}
	
void SpaceShooterPlayerProperty::Damage(SpaceShooterWeaponType & type)
{
	if (sleeping)
		return;
	hp -= type.damage;
	if (hp < 0)
	{
		// Die?!
		hp = 0;
		Destroy();
	}
	if (this->allied)
	{
		game->UpdatePlayerHP();
	}
}
