/// Emil Hedemalm
/// 2013-02-08


#ifndef SHOP_H
#define SHOP_H

#include "Location.h"
#include "../Item.h"
#include <cstring>

/// Subclass of location,
struct Shop : Location {
public:
	/// Default constructor
	Shop() : Location(LocType::SHOP){
		memset(sellableItems, NULL, sizeof(sellableItems));
	};
	~Shop(){
		for (int i = 0; i < MAX_SELLABLE_ITEMS; ++i)
			if (sellableItems[i])
				delete sellableItems[i];
	};
	static const int MAX_SELLABLE_ITEMS = 32;
	/// List of items available-ish~
	Item * sellableItems[MAX_SELLABLE_ITEMS];


};
#endif

