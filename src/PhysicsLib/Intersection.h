/// Emil Hedemalm
/// 2014-08-06
/// An intersection, as used by ray-casting functions.

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "Entity/Entity.h"

struct Intersection 
{
	/// Distance from the origin that the ray met this intersection.
	float distance;
	/// Entity the ray is intersecting with.
	EntitySharedPtr entity;
};

#endif
