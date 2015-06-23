/// Emil Hedemalm
/// 2015-01-21
/// Weapon..

#ifndef WEAPON_H
#define WEAPON_H

#include "String/AEString.h"
#include "MathLib.h"
#include "Time/Time.h"

class Ship;

namespace WeaponType
{
	enum 
	{
		BAD_TYPE = -1,
		TYPE_0 = 0, // + 0 to 9
		TYPE_1,
		TYPE_2,
		TYPE_3, // etc.
	};
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
	String projectileShape;
	// Damage inflicted.
	int damage;
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

	/// For aiming weapons, mainly. Normalized vector.
	Vector3f currentAim;
};

#endif
