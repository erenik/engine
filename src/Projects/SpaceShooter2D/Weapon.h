/// Emil Hedemalm
/// 2015-01-21
/// Weapon..

#ifndef WEAPON_H
#define WEAPON_H

#include "String/AEString.h"

class Ship;

class Weapon 
{
public:
	Weapon();
	static Weapon Get(String byName);
	static bool LoadTypes(String fromFile);
	void Shoot(Ship * ship);
	String name;
	/// -1 = Infinite, >= 0 = Finite
	int ammunition;
	/// Cooldown in milliseconds.
	int cooldownMs;
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
	/// Angle in degrees.
	int angle;
	bool burst;
	/// Last show, format Time::Now().Milliseconds()
	int lastShotMs;
	/// For burst.
	int rounds;
	/// Delay between each round within the burst.
	int burstRoundDelayMs;
	static List<Weapon> types;
};

#endif
