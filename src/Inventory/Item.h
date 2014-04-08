/// Emil Hedemalm
/// 2014-04-08
/// General item class. Subclass to add further data.

#ifndef ITEM_H
#define ITEM_H

class Item 
{
	/// Name of the item.
	String name;
	/// Type of item, used primarily for sorting. Default is 0.
	int type;
	/// Sub-type of item.
	int subType;
	
	/// If true, will/must support stacking. Default is false.
	bool stackable;
	/// If a limit is wanted, set to any positive number. 0 will be considered infinite/no limit. Default is 0.
	int stackLimit;
	/// If the inventory system supports stacking items, this will define how many exist of this specific item(type) in the inventory.
	int amount;
};

#endif
