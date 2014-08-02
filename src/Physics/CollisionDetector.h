/// Emil Hedemalm
/// 2014-07-16
/// Physics collision-detection class. Subclass for custom behaviour.

#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include "Physics/PhysicsProperty.h"
#include "Entity/Entity.h"

class CollisionDetector 
{
public:
	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> entities, List<Collision> & collisions) = 0;

	/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
	virtual int DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions);
private:
};

#endif


