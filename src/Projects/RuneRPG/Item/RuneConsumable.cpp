/// Emil Hedemalm
/// 2014-04-10
/// Item class for usable items the RuneRPG project.

#include "RuneConsumable.h"
#include "RuneItemTypes.h"

RuneConsumable::RuneConsumable(String name)
: RuneItem(RuneItemType::CONSUMABLE, name)
{
	healAmount = 10;
	healsHP = true;
	healsMP = false;
}
