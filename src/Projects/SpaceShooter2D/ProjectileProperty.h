/// Emil Hedemalm
/// 2015-01-21
/// Projectile.

#ifndef PROJ_PROP_H
#define PROJ_PROP_H

#include "Weapon.h"
#include "Entity/EntityProperty.h"
#include "Time/Time.h"

class ProjectileProperty : public EntityProperty 
{
public:
	ProjectileProperty(const Weapon & weaponThatSpawnedIt, Entity * owner);
	// Static version.
	static int ID();

	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);

	void Destroy();

	// Fall asleep.. unregistering it from physics, graphics, etc.
	void SleepThread();

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	/// Resets sleep-flag, among other things
	void OnSpawn();

	// Whose side the projectile belongs to. Determines who it will react to/damage.
	bool player;
	bool enemy;

	Weapon weapon;

	/// If not currently active (available for re-use).
	bool sleeping;

	int timeAliveMs;
	Vector4f color;
};

#endif
