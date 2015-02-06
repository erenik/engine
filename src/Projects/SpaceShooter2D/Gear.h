/// Emil Hedemalm
/// 2015-02-06
/// o.o

#ifndef GEAR_H
#define GEAR_H

#include "String/AEString.h"

class Gear 
{
public:
	String name;
	int price;
	enum {
		WEAPON,
		SHIELD_GENERATOR,
		ARMOR,
	};
	int type;
	// Weapon stats?

	// Shield stats.
	int maxShield;
	int shieldRegen;
	// Armor stats
	int maxHP;

	String description;

	/// o.o
	static bool Load(String fromFile);
	static List<Gear> GetType(int type);
	static Gear Get(String byName);

	/// Available to buy!
	static List<Gear> availableGear;
};

#endif
