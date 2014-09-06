/// Emil Hedemalm
/// 2014-04-10
/// Item class for the RuneRPG project.

#ifndef RUNE_ITEM_H
#define RUNE_ITEM_H

#include "String/AEString.h"
#include "MathLib.h"
#include "RuneRPG/Battle/BattleEffect.h"


namespace Slot 
{
	enum slots 
	{
		HEAD,
		TORSO,
		HANDS,
		FEET,
		OFF_HAND,
		SLOTS,
	};
};

int GetSlot(String byName);
String GetSlotName(int slot);

/// Base item class. Sub-class to add extra features for any item-type!
class RuneItem
{
public:
	/// Built-in static manager functions
	static bool LoadWeaponsFromCSV(String fileName);
	static bool LoadArmourFromCSV(String fileName);
	static bool LoadConsumablesFromCSV(String fileName);
	/// Loads items from target CSV. Used by the previous 3 functions.
	static List<RuneItem> LoadFromCSV(String fileName);

	/// Returns a list of gear as corresponding to the demanded string of comma-separated names.
	static List<RuneItem> GetGearByString(String str);

	/// Returns reference gear by name.
	static const RuneItem * GetWeapon(String byName);
	static const RuneItem * GetArmor(String byName);

	/// Default weapon-slot: Unarmed
	static RuneItem DefaultWeapon();
	/// Default 1 item/gear in each slot and 1 weapon.
	static List<RuneItem> DefaultGear();

	/// All kinds of items in the game.
	static List<RuneItem> allWeapons, allArmor, allConsumables, allKeyItems;

	
	RuneItem();
	virtual ~RuneItem();

	/// Name of the item.
	String name;
	/// Type of item, see ItemTypes.h
	int type;

	/// For armor/gear, defines the slot it occupies. See Slot enum above.
	int slot;
	/// Weapon-based stats.
	int weaponModifier, actionCost, parryModifier;
	/// Various stats.
	int armorRating, magicArmorRating, blockModifier;

	/// Price for this specific item. E.g. player selling to others or specific store's prices.
	int price;
	/// Default prices for buying from and selling to NPCs.
	int defaultBuyPrice;
	int defaultSellPrice;

	/// If the inventory system supports stacking items, this will define how many exist of this specific item(type) in the inventory.
	int quantity;

	/// Volumetrics 
	int weight;

	/// For consumables and use- items.
	int targetFilter;
	/// D:
	float freezeTimeInSeconds, castTimeInSeconds;

	/// Effect if used in battle?
	List<BattleEffect> useEffects;
	/// Constant effect when equipped.
	List<BattleEffect> equipEffects;

private:




};


#endif

