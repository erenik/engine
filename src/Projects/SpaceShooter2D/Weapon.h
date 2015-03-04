/// Emil Hedemalm
/// 2015-01-21
/// Weapon..

#ifndef WEAPON_H
#define WEAPON_H

#include "String/AEString.h"
#include "MathLib.h"
#include "Time/Time.h"

class Ship;

class Weapon 
{
public:
	Weapon();
	static bool Get(String byName, Weapon & weapon);
	static bool LoadTypes(String fromFile);
	/// Moves the aim of this weapon turrent.
	void Aim(Ship * ship);
	/// Shoots using previously calculated aim.
	void Shoot(Ship * ship);
	String name;
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
