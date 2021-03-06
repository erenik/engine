/// Emil Hedemalm
/// 2014-07-25
/// Properties for the projectiles in a space-shooter game.

#include "SpaceShooterProjectileProperty.h"
#include "Game/SpaceShooter/SpaceShooter.h"

#include "SpaceShooterExplosionProperty.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Entity/Entity.h"
#include "Maps/MapManager.h"

#include "Model/ModelManager.h"
#include "TextureManager.h"


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
	/// Already exploded! o.o
	if (sleeping)
		return;

	// If projectile-projectile, remove them both?
	Entity * other = 0;
	if (data.one == owner)
		other = data.two;
	else if (data.two == owner)
		other = data.one;

	// Add some explosive effects?
	this->SleepThread();

	// Explode
	Entity * explosionEntity = game->NewExplosion(owner->position, ExplosionType::PROJECTILE);
	game->explosions.Add(explosionEntity);
	
}


void SpaceShooterProjectileProperty::SleepThread()
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
	// If game is paused.
	if (game->paused)
		return;

	if (sleeping)
		return;

	// Check life-time too. So we don't remove it straight away on spawn.
	timeAliveMs += timeInMs;
	// Assume there is some max life-time to all projectiles.!
	if (/*game->IsPositionOutsideFrame(owner->position) && alive >= 3 || */
		timeAliveMs >= 10000 ||
		(owner->position[0] < game->left  && owner->Velocity()[0] < 0) ||
		(owner->position[0] > game->right && owner->Velocity()[0] > 0) )
	{
		SleepThread();
	}
}

/// Resets sleep-flag, among other things
void SpaceShooterProjectileProperty::OnSpawn()
{
	sleeping = false;
	timeAliveMs = 0;
}
