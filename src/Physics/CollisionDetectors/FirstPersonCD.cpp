/// Emil Hedemalm
/// 2014-08-06
/// A collision-detector suitable for a first person game. 
/// Sub class to override behaviour.

#include "FirstPersonCD.h"

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int FirstPersonCD::DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions)
{
	for (int i = 0; i < pairs.Size(); ++i)
	{
		// stuff.
		EntityPair & pair = pairs[i];
		// do stuff?
	}

	return 0;
}


/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int FirstPersonCD::DetectCollisions(List<Entity*> & entities, List<Collision> & collisions)
{
	// Implement..
	return 0;
}
/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
int FirstPersonCD::DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions)
{
	// Implement..
	return 0;
}


