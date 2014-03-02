/// Emil Hedemalm
/// 2013-02-08


#ifndef LOCATION_H
#define LOCATION_H

#include "LocationTypes.h"
#include "../Globals.h"
#include "../Pathfinding/Waypoint.h"

/// Encapsulating class for a more or less multi-purpose in-game location
class Location {
public:
	/// Default constructor
	Location(char i_type){
		radius = 1.0f;
		type = i_type;
		locationEntity = NULL;
		wp = NULL;
	};
	/** Entity (if any) for the location.
		If this is NULL, that means no good visual representation is available,
		or that the visual representation does not equal the interactable radius.
		E.g: A shop and it's surrounding perimeter from which customers can order stuff.
	*/
	Entity * locationEntity;
	/// Bounds relative to the location
//	AABB bounds;
	float radius;
	/// XYZ-location
	Vector3f location;

	/// Name of the place
	char name[NAME_LIMIT];

	/// Type of location. Location types are defined in LocationTypes.h
	char type;

	/// Waypoint that the location resides in.
	Waypoint * wp;
};

#endif
