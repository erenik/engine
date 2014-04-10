/// Emil Hedemalm
/// 2014-04-10
/// A shop in the RuneRPG world.

#ifndef RUNE_SHOP_H
#define RUNE_SHOP_H

class RuneItem;
#include <fstream>
#include "String/AEString.h"
#include "List/List.h"

/** For shops, the item's 'quantity' specifier corresponds to how many
	of said item that the store has to sell.
*/
class RuneShop 
{
public:

	RuneItem * GetItem(String byName);

	/// Loads from binary file.
	bool ReadFrom(std::fstream & file);
	bool WriteTo(std::fstream & file); 
	List<RuneItem*> GetItems() { return items; };

	/// Test shop!
	static RuneShop testShop;
	/// Fills the test shop with some items to sell.
	static void SetupTestShop();
	
	/// Name of the shop!
	String name;
	/// Owner npc or.. stuff.
	String owner;
private:
	/// Current items for sale.
	List<RuneItem*> items;
};

#endif

