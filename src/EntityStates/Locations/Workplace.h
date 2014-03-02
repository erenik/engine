/// Emil Hedemalm
/// 2013-02-08


#ifndef WORKPLACE_H
#define WORKPLACE_H

#include "Location.h"

/// Subclass of location,
struct Workplace : Location {
public:
	/// Default constructor
	Workplace() : Location(LocType::WORKPLACE){
		coinsPerHour = 1;
	};
	/// Wage
	float coinsPerHour;


};
#endif
