/// Emil Hedemalm
/// 2015-01-21
/// Ship.

#ifndef SHIP_H
#define SHIP_H

#include "Weapon.h"
#include "MathLib.h"
#include "Movement.h"
#include "Rotation.h"
#include "SpaceShooter2D/Base/Gear.h"

class Entity;
class Model;
class SpawnGroup;
class WeaponScript;

enum 
{
	NO_SKILL,
	ATTACK_FRENZY,
	SPEED_BOOST,
	POWER_SHIELD,
};

class Ship 
{
public:
	Ship();
	~Ship();

	/// Call on spawning.
	void RandomizeWeaponCooldowns();
	/// Spawns, creating entities, registering for movement, etc. Returns it and all children spawned with it.
	List<Entity*> Spawn(ConstVec3fr atPosition, Ship * parent);
	/// Handles spawning of children as needed.
	List<Entity*> SpawnChildren();
	void Despawn();

	void Process(int timeInMs);
	void ProcessAI(int timeInMs);
	void ProcessWeapons(int timeInMs);

	/// Prepends the source with '/obj/Ships/' and appends '.obj'. Uses default 'Ship.obj' if needed.
	Model * GetModel();
	/// o.o
	void DisableMovement();
	void Damage(Weapon & usingWeapon);
	void Damage(float amount, bool ignoreShield);
	void Destroy();
	// Load ship-types.
	static bool LoadTypes(String file);
	/// E.g. "Straight(10), MoveTo(X Y 5 20, 5)"
	void ParseMovement(String fromString);
	/// E.g. "DoveDir(3), RotateToFace(player, 5)"
	void ParseRotation(String fromString);

	/// Creates new ship of specified type.
	static Ship * New(String shipByName);
	
	/// Returns speed, accounting for active skills, weights, etc.
	float Speed();
	/// Accounting for boosting skills.
	float MaxShield();

	/// Checks weapon's latest aim dir.
	Vector3f WeaponTargetDir();

	/// If using Armor and Shield gear (Player mainly).
	void UpdateStatsFromGear();
	bool SwitchToWeapon(int index);

	/// Calls OnEnter for the initial movement pattern.
	void StartMovement();

	/// For player ship.
	void SetWeaponLevel(int weaponType, int level);
	Weapon * GetWeapon(int ofType);
	void ActivateSkill();

	/// Names of children to spawn along side it.
	List<String> childrenStrings;

	// Name or type. 
	String name;
	// Faction.
	String type;
	SpawnGroup * spawnGroup;
	Ship * parent;
	List<Ship*> children;
	// Bools
	bool canMove;
	bool movementDisabled; // temporarily.
	bool canShoot;
	bool hasShield;
	bool shoot; // if shooting is requested.
	bool weaponScriptActive; // Default false.
	int skill; // Default 0, see above.
	String skillName;
	int timeSinceLastSkillUseMs;
	int skillCooldownMs;
	int activeSkill;
	int skillDurationMs;
	float skillCooldownMultiplier; // default 1, lower in situations or tutorial
	String onCollision;
	bool spawned;
	bool destroyed;

	/// In order to not take damage allllll the time (depending on processor speed, etc. too.)
	Time lastShipCollision;
	Time collisionDamageCooldown; // default 100 ms.

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
	float hp;
	float armorRegenRate;
	int maxHP;
	int collideDamage;
	float heatDamageTaken;
	List<String> abilities;
	List<float> abilityCooldown;
	String graphicModel;
	String other;

	WeaponSet weapons;
	Weapon * activeWeapon; // One active weapon at a time.. for the player at least.

	/// o.o 
	bool allied;
	bool ai;
	bool spawnInvulnerability;
	/// Yielded when slaying it.
	int score; 

	// Default false
	Entity * entity;
	// Data details.
	// Spawn position.
	Vector3f position;
	/// As loaded.
	static List<Ship*>  types;

	/// Used by player, mainly.
	Gear weapon, shield, armor;

	WeaponScript * weaponScript;

private:
};

#endif
