/// Emil Hedemalm
/// 2014-04-10
/// Item class for the RuneRPG project.

#include "RuneItem.h"

/// Type of item. This is stored in the 'type' variable of the base Item class.
RuneItem::RuneItem(int type, String name)
: Item(type, name)
{
}

RuneItem::~RuneItem()
{
}
