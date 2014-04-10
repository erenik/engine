/// Emil Hedemalm
/// 2014-04-10
/// A shop in the RuneRPG world.

#include "RuneShop.h"
#include "RuneConsumable.h"

RuneShop RuneShop::testShop;


RuneItem * RuneShop::GetItem(String byName)
{
	for (int i = 0; i < items.Size(); ++i)
	{
		RuneItem * item = items[i];
		if (item->name == byName)
			return item;
	}
	return NULL;
}

void RuneShop::SetupTestShop()
{
	// Already has items. Return.
	if (testShop.items.Size())
		return;
	RuneConsumable * potion = new RuneConsumable("Potion");
	potion->price = 50;
	testShop.items.Add(potion);

	RuneConsumable * hiPotion = new RuneConsumable("Hi-Potion");
	hiPotion->price = 150;
	testShop.items.Add(hiPotion);
}