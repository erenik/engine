/// Emil Hedemalm
/// 2014-08-01
/// Collision-detector dedicated to the MORPG-project.

#include "MORPGCD.h"

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int MORPGCD::DetectCollisions(List<Entity*> entities, List<Collision> & collisions)
{
	return 0;
}


/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
int MORPGCD::DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions)
{
	/// ...
	return 0;
}


