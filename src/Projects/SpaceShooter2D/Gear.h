/// Emil Hedemalm
/// 2015-02-06
/// o.o

#ifndef GEAR_H
#define GEAR_H

#include "String/AEString.h"
#include "Time/Time.h"

class Gear 
{
public:
	Gear();
	String name;
	int price;
	enum {
		WEAPON,
		SHIELD_GENERATOR,
		ARMOR,
	};
	int type;
	// Weapon stats?
	int damage;
	Time reloadTime;
	// Shield stats.
	int maxShield;
	float shieldRegen;
	// Armor stats
	int maxHP;
	int toughness;
	int reactivity;

	String description;

	/// o.o
	static bool Load(String fromFile);
	static List<Gear> GetType(int type);
	static Gear Get(String byName);

	static Gear StartingWeapon();
	static Gear StartingArmor();
	static Gear StartingShield();

	/// Available to buy!
	static List<Gear> availableGear;
};

#endif
