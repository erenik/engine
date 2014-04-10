/// Emil Hedemalm
/// 2014-04-10
/// Item class for the RuneRPG project.

#ifndef RUNE_ITEM_H
#define RUNE_ITEM_H

#include "Inventory/Item.h"

/// Base item class. Sub-class to add extra features for any item-type!
class RuneItem : public Item
{
public:
	/// Type of item. This is stored in the 'type' variable of the base Item class.
	RuneItem(int type, String name);
	virtual ~RuneItem();
private:
};

#endif

