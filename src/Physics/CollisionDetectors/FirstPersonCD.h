/// Emil Hedemalm
/// 2014-08-06
/// A collision-detector suitable for a first person game (i.e. a game where the camera follows a single entity). 
/// Sub class to override behaviour.

#include "Physics/CollisionDetector.h"

class FirstPersonCD : public CollisionDetector 
{
public:
	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> entities, List<Collision> & collisions);
	/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
	virtual int DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions);

};

