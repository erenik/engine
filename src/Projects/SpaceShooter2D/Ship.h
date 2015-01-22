/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#ifndef SHIP_H
#define SHIP_H

#include "Weapon.h"
#include "MathLib.h"

class Entity;

class Ship 
{
public:
	Ship();
	~Ship();
	void Damage(int amount, bool ignoreShield);
	void Destroy();
	// Load ship-types.
	static bool LoadTypes(String file);
	/// Creates new ship of specified type.
	static Ship New(String shipByName);
	// Name or type. 
	String name;
	// Faction.
	String type;
	// Bools
	bool canMove;
	bool canShoot;
	bool hasShield;
	/// In seconds
	String movementPattern;
	// Parsed value divided by 5.
	float speed;
	float shieldValue, maxShieldValue;
	/// Regen per millisecond
	float shieldRegenRate;
	int hitPoints;
	int maxHitPoints;
	List<String> abilities;
	List<float> abilityCooldown;
	String graphicModel;
	String other;

	List<Weapon> weapons;

	/// o.o 
	bool allied;
	bool ai;
	bool spawnInvulnerability;
	/// Yielded when slaying it.
	int score; 

	// Default false
	bool spawned;
	Entity * entity;
	// Data details.
	// Spawn position.
	Vector3f position;
	/// As loaded.
	static List<Ship> types;
private:
};

#endif
