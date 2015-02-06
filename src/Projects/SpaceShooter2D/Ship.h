/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#ifndef SHIP_H
#define SHIP_H

#include "Weapon.h"
#include "MathLib.h"
#include "Movement.h"
#include "Rotation.h"
#include "Gear.h"

class Entity;
class Model;

class Ship 
{
public:
	Ship();
	~Ship();
	/// Prepends the source with '/obj/Ships/' and appends '.obj'. Uses default 'Ship.obj' if needed.
	Model * GetModel();

	void Damage(int amount, bool ignoreShield);
	void Destroy();
	// Load ship-types.
	static bool LoadTypes(String file);
	/// E.g. "Straight(10), MoveTo(X Y 5 20, 5)"
	void ParseMovement(String fromString);
	/// E.g. "DoveDir(3), RotateToFace(player, 5)"
	void ParseRotation(String fromString);

	/// Creates new ship of specified type.
	static Ship New(String shipByName);
	
	/// Checks weapon's latest aim dir.
	Vector3f WeaponTargetDir();

	/// Calls OnEnter for the initial movement pattern.
	void StartMovement();
	// Name or type. 
	String name;
	// Faction.
	String type;
	// Bools
	bool canMove;
	bool canShoot;
	bool hasShield;

	/// In order to not take damage allllll the time (depending on processor speed, etc. too.)
	int64 lastShipCollisionMs;

	/// Mooovemeeeeeeent
	List<Movement> movementPatterns;
	int currentMovement; // Index of which pattern is active at the moment.
	int timeInCurrentMovement; // Also milliseconds.
	List<Rotation> rotationPatterns;
	int currentRotation;
	int timeInCurrentRotation;
	/// Maximum amount of radians the ship may rotate per second.
	float maxRadiansPerSecond;

	// Parsed value divided by 5.
	float speed;
	float shieldValue, maxShieldValue;
	/// Regen per millisecond
	float shieldRegenRate;
	int hitPoints;
	int maxHitPoints;
	int collideDamage;
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

	/// Used by player, mainly.
	Gear shield, armor;
private:
};

#endif
