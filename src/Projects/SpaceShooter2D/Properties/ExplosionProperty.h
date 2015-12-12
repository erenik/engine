/// Emil Hedemalm
/// 2015-12-12
/// For explosions, simulated in physics.

#ifndef EXPLOSION_PROPERTY_H
#define EXPLOSION_PROPERTY_H

#include "Entity/EntityProperty.h"

class ExplosionProperty : public EntityProperty 
{
public:
	ExplosionProperty(const Weapon & weaponThatSpawnedIt, Entity * owner);
	// Static version.
	static int ID();
	/// Based on radius
	float CurrentDamage();
	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);
	/// Despawning/deleting
	void Remove();
	// Fall asleep.. unregistering it from physics, graphics, etc.
	void SleepThread();
	void UpdateVelocity();

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);

	bool ShouldDamage(Ship * ship);

	/// Resets sleep-flag, among other things
	void OnSpawn();

	/// in ms
	int duration; 
	/// PASTED
	int totalDamageInflicted;
	Weapon weapon;
	Vector3f position;
	float currentRadius;
	List<Ship*> affectedShips; // Only affect a ship once.
	Time startTime;


	// Whose side the projectile belongs to. Determines who it will react to/damage.
	bool player;
	bool enemy;

	List<Ship*> damagedTargets;
	/// If not currently active (available for re-use).
	bool sleeping;

	/// Used for various effects, such as laser burst wobbling, fading damage/removal for heat-wave/ion flak, etc.
	int timeAliveMs;
	Vector4f color;
	String onCollisionMessage;
};

#endif
