/// Emil Hedemalm
/// 2014-04-10
/// Item class for usable items the RuneRPG project.

#ifndef RUNE_CONSUMABLE_H
#define RUNE_CONSUMABLE_H

#include "RuneItem.h"

class RuneConsumable : public RuneItem 
{
public:
	RuneConsumable(String name);

	/// Most stuff just heal.
	int healAmount;
	/// But what does it heal?
	bool healsHP;
	bool healsMP;
private:

};

#endif
