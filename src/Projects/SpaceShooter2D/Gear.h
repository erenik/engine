/// Emil Hedemalm
/// 2015-02-06
/// o.o

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

	/// Available to buy!
	static List<Gear> availableGear;
};

