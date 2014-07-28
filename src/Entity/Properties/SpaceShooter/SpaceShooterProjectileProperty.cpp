/// Emil Hedemalm
/// 2014-07-25
/// Properties for the projectiles in a space-shooter game.

#include "SpaceShooterProjectileProperty.h"
#include "Game/SpaceShooter/SpaceShooter.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"

#include "Entity/Entity.h"
#include "Maps/MapManager.h"


SpaceShooterProjectileProperty::SpaceShooterProjectileProperty(SpaceShooter * game, Entity * owner, SpaceShooterWeaponType type)
	: EntityProperty("SpaceShooterProjectileProperty", ID(), owner), game(game), type(type)
{
	sleeping = false;
}

int SpaceShooterProjectileProperty::ID()
{
	return EntityPropertyID::MINI_GAME_3 + 1;
}

/// If reacting to collisions...
void SpaceShooterProjectileProperty::OnCollision(Collision & data)
{
	// If projectile-projectile, remove them both?
	Entity * other = 0;
	if (data.one == owner)
		other = data.two;
	else if (data.two == owner)
		other = data.one;

	// Add some explosive effects?
	this->Sleep();
}


void SpaceShooterProjectileProperty::Sleep()
{
	if (sleeping)
		return;
	Graphics.QueueMessage(new GMUnregisterEntity(owner));
	Physics.QueueMessage(new PMUnregisterEntity(owner));
	sleeping = true;
}

/// Time passed in seconds..!
void SpaceShooterProjectileProperty::Process(int timeInMs)
{
	if (game->IsPositionOutsideFrame(owner->position))
	{
		// Remove ourselves.
		Physics.QueueMessage(new PMUnregisterEntity(owner));
		Graphics.QueueMessage(new GMUnregisterEntity(owner));
		sleeping = true;
	}
}

/// Resets sleep-flag, among other things
void SpaceShooterProjectileProperty::OnSpawn()
{
	sleeping = false;
}
