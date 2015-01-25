/// Emil Hedemalm
/// 2014-08-01
/// Collision-detector dedicated to the MORPG-project.

// If it will use collisions at all.. if not it will spare collision-detections used within the usual detection.

#include "Physics/CollisionDetector.h"

class MORPGCD : public CollisionDetector
{
public:
	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions);

	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> & entities, List<Collision> & collisions);

	/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
	virtual int DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions);

};


