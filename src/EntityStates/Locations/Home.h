/// Emil Hedemalm
/// 2013-02-08


#ifndef HOME_H
#define HOME_H

#include "Location.h"
;
/// Subclass of location,
class Residence : public Location {
public:
	/// Default constructor
	Residence() : Location(LocType::HOME){
		owner = NULL;
		price = 1;
	};
	/// Character owning it
	Entity * owner;
	/// Price to purchase it o-o;
	int price;
};

#endif

