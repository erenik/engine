/// Emil Hedemalm
/// 2015-01-21
/// Weapon..

#ifndef WEAPON_H
#define WEAPON_H

#include "String/AEString.h"
#include "MathLib.h"
#include "Time/Time.h"

class Ship;
class Weapon;
class Entity;

class WeaponSet : public List<Weapon*> 
{
public:
	/// Handles dynamic allocation of the weapons, both adding and clearing.
	WeaponSet();
	virtual ~WeaponSet();
	WeaponSet(WeaponSet & otherWeaponSet);
};

namespace WeaponType
{
	enum 
	{
		BAD_TYPE = -1,
		TYPE_0 = 0, // + 0 to 9
		TYPE_1,
		TYPE_2,
		TYPE_3, // etc.
		TYPE_4,
		TYPE_5,
		TYPE_6,
		TYPE_7,
		TYPE_8,
		MAX_TYPES,
	};
};

#define BULLETS WeaponType::TYPE_0
#define SMALL_ROCKETS WeaponType::TYPE_1
#define BIG_ROCKETS WeaponType::TYPE_2
#define LIGHTNING WeaponType::TYPE_3
#define LASER_BEAM WeaponType::TYPE_4
#define LASER_BURST WeaponType::TYPE_5
#define HEAT_WAVE WeaponType::TYPE_6
#define ION_FLAK WeaponType::TYPE_7


class LightningArc
{
public:
	LightningArc();
	int damage;
	Vector3f position;
	Entity * graphicalEntity;
	Entity * targetEntity;
	bool struckEntity;
	bool arcFinished; // When time expires or range has been reached.
	float maxRange;
	int maxBounces;
	Time arcTime;
	LightningArc * child;
};

class Weapon
{
public:
	Weapon();
	// Sets
	static bool Get(String byName, Weapon * weapon);
	/** For player-based, returns pointer, but should be used as reference only (*-dereference straight away). 
		Returns 0 if it doesn't exist. */
	static Weapon * Get(int type, int level); 
	static bool LoadTypes(String fromFile);
	/// Moves the aim of this weapon turrent.
	void Aim(Ship * ship);
	/// Shoots using previously calculated aim.
	void Shoot(Ship * ship);
	/// Called to update the various states of the weapon, such as reload time, making lightning arcs jump, etc.
	void Process(Ship * ship, int timeInMs);
	void ProcessLightning(Ship * ship, bool initial = false);
	/// Based on ship.
	Vector3f WorldPosition(Entity * basedOnShipEntity);
	Vector3f location;

	List<LightningArc*> arcs;
	List<Ship*> shipsStruckThisArc; /// For skipping

	/// Delay in milliseconds between bounces for lightning
	int arcDelay;
	int maxBounces; /// Used to make lightning end prematurely.
	int currCooldownMs; /// Used instead of flyTime.
	float stability;
	String name;
	bool enabled; // default true.
	int cost; // o-o
	int type; // mainly for player-based weapons.
	int level; // Also mainly for player-based weapons.
	/// -1 = Infinite, >= 0 = Finite
	int ammunition;
	int numberOfProjectiles; // Per 'firing'
	int distribution; // Default CONE?
	float linearDamping; // Applied for slowing bullets (Ion Flak).
	enum {
		CONE,
	};
	/// Cooldown.
	Time cooldown;
	enum {
		STRAIGHT,
		SPINNING_OUTWARD,
		HOMING,
	};
	/// For boss-spam stuff.
	bool circleSpam;
	int projectilePath;
	float projectileSpeed;
	float homingFactor; // For heat-seaking/auto-aiming missiles.
	String projectileShape;
	float projectileScale;
	float maxRange; // Used for Lightning among other things
	// Damage inflicted.
	float damage;
	float relativeStrength; /// Used to apply distance attentuation to damage, etc. (e.g. Heat-wave) default 1.0
	float explosionRadius; // o.o' More damage closer to the center of impact.
	// Penetration rate.
	float penetration;
	// If true, auto-aims at the player.
	bool aim;
	/// Not just looking, thinking.
	bool estimatePosition;
	/// Angle in degrees.
	int angle;
	// Burst stuff.
	bool burst;
	// Start time of last/active burst.
	Time burstStart;
	/// Delay between each round within the burst.
	Time burstRoundDelay;
	/// For burst.
	int burstRounds;
	// Restarts
	int burstRoundsShot;
	/// Linear scaling over time (multiplied with initial scale?) 
	float linearScaling;
	/// Last show, format Time::Now().Milliseconds()
//	Time lastShot;
	static List<Weapon> types;
	/// Sound effects (SFX)
	String shootSFX, hitSFX;

	/// For aiming weapons, mainly. Normalized vector.
	Vector3f currentAim;
	/// Recalculated every call to Aim()
	Vector3f weaponWorldPosition;
};

#endif
