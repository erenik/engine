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
	static Weapon Get(int type, int level); // for player-based
	static bool LoadTypes(String fromFile);
	/// Moves the aim of this weapon turrent.
	void Aim(Ship * ship);
	/// Shoots using previously calculated aim.
	void Shoot(Ship * ship);
	/// Called to update the various states of the weapon, such as reload time, making lightning arcs jump, etc.
	void Process(Ship * ship);
	void ProcessLightning(Ship * ship, bool initial = false);

	List<LightningArc*> arcs;
	List<Ship*> shipsStruckThisArc; /// For skipping

	/// Delay in milliseconds between bounces for lightning
	int arcDelay;
	int maxBounces; /// Used to make lightning end prematurely.

	String name;
	int type; // mainly for player-based weapons.
	int level; // Also mainly for player-based weapons.
	/// -1 = Infinite, >= 0 = Finite
	int ammunition;
	/// Cooldown.
	Time cooldown;
	enum {
		STRAIGHT,
		SPINNING_OUTWARD,
		HOMING,
	};
	int projectilePath;
	float projectileSpeed;
	float homingFactor; // For heat-seaking/auto-aiming missiles.
	String projectileShape;
	float projectileScale;
	float maxRange; // Used for Lightning among other things
	// Damage inflicted.
	int damage;
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
	/// Last show, format Time::Now().Milliseconds()
	Time lastShot;
	static List<Weapon> types;
	/// Sound effects (SFX)
	String shootSFX, hitSFX;

	/// For aiming weapons, mainly. Normalized vector.
	Vector3f currentAim;
};

#endif
