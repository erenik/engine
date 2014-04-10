/// Emil Hedemalm
/// 2014-04-08
/// General item class. Subclass to add further data.

#ifndef ITEM_H
#define ITEM_H

#include "String/AEString.h"
#include "MathLib.h"

class Item 
{
public:
	/// Default constructor.
	Item(int type, String name);
	/// Name of the item.
	String name;
	/// Type of item, used primarily for sorting. Default is 0.
	int type;
	/// Sub-type of item.
	int subType;

	/// Price for this specific item. E.g. player selling to others or specific store's prices.
	int price;
	/// Default prices for buying from and selling to NPCs.
	int defaultBuyPrice;
	int defaultSellPrice;

	/// If true, will/must support stacking. Default is false.
	bool stackable;
	/// If a limit is wanted, set to any positive number. 0 will be considered infinite/no limit. Default is 0.
	int stackLimit;
	/// If the inventory system supports stacking items, this will define how many exist of this specific item(type) in the inventory.
	int quantity;

	/// Volumetrics 
	float weight;
	/// Volume in meter-dimensions or slots if using 2D. May be omitted.
	Vector3f volume;
};

#endif
