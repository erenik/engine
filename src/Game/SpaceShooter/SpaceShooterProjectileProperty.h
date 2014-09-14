/// Emil Hedemalm
/// 2014-07-25
/// Properties for the projectiles in a space-shooter game.

#ifndef SPACE_SHOOTER_PROJECTILE_PROPERTY_H
#define SPACE_SHOOTER_PROJECTILE_PROPERTY_H

#include "Entity/EntityProperty.h"
#include "Game/SpaceShooter/SpaceShooterWeaponType.h"
#include "Time/Time.h"

class SpaceShooter;

class SpaceShooterProjectileProperty : public EntityProperty 
{
public:
	SpaceShooterProjectileProperty(SpaceShooter * game, Entity * owner, SpaceShooterWeaponType type);
	// Static version.
	static int ID();

	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);

	// Fall asleep.. unregistering it from physics, graphics, etc.
	void Sleep();

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	/// Resets sleep-flag, among other things
	void OnSpawn();

	SpaceShooterWeaponType type;

	// Whose side the projectile belongs to. Determines who it will react to/damage.
	bool player;
	bool enemy;

	/// If not currently active (available for re-use).
	bool sleeping;

	int timeAliveMs;

private:
	SpaceShooter * game;
};

#endif
