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

#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"

SpaceShooterPlayerProperty::SpaceShooterPlayerProperty(SpaceShooter * game, Entity * owner)
	: EntityProperty("SpaceShooterPlayerProperty", ID(), owner), game(game), weaponType(SpaceShooterWeaponType::RAILGUN)
{
	allied = false;
	sleeping = false;
	isPlayer = false;

	lastFire = Time::Now();
}

int SpaceShooterPlayerProperty::ID()
{
	return EntityPropertyID::MINI_GAME_3 + 0;
}

void SpaceShooterPlayerProperty::Remove()
{
	Graphics.QueueMessage(new GMUnregisterEntity(owner));
	Physics.QueueMessage(new PMUnregisterEntity(owner));
	sleeping = true;
	game->OnPlayerDestroyed(owner);
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
}
	

/// Time passed in seconds..!
void SpaceShooterPlayerProperty::Process(int timeInMs)
{
	if (sleeping)
		return;

	// If game is paused.
	if (game->paused)
		return;

	// Sleep if outside frame too.
	if (game->IsPositionOutsideFrame(owner->position))
	{
		if (owner->position.x < -game->gameSize.x * 0.5f - owner->scale.MaxPart())
		{
			// Sleep
			Remove();
		}
		return;
	}

	// Get look-at direction.
	Vector4f minusZ(0,0,-1,0);
	Vector4f lookAt = owner->rotationMatrix.Product(minusZ);
//	std::cout<<"\nLook at: "<<lookAt;

	Time now = Time::Now();
	if (lastFire.Type() == 0)
		lastFire = Time::Now();
	int millisecondsPassedSinceLastFire = (now - lastFire).Milliseconds();
	if (millisecondsPassedSinceLastFire < weaponType.coolDown)
		return;

	lastFire = 0;

	// Spawn a projectile? Or senda message to do so?
	Entity * projectile = game->NewProjectile(weaponType);
	
	// If not ok request, skip it then.
	if (!projectile)
		return;

	//	MapMan.CreateEntity("Projectile", ModelMan.GetModel("Cube"), TexMan.GetTexture("Red"));


	Vector3f position = owner->position;
	position += lookAt * owner->scale.MaxPart();
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
		return;
	}

	SpaceShooterProjectileProperty * sspp2 = (SpaceShooterProjectileProperty *) other->GetProperty(SpaceShooterProjectileProperty::ID());
	if (sspp2)
	{
		// Take damage? D:
		// Die?!
		Destroy();
	}
}
	
