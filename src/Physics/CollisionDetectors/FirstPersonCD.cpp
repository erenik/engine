/// Emil Hedemalm
/// 2014-08-06
/// A collision-detector suitable for a first person game. 
/// Sub class to override behaviour.

#include "FirstPersonCD.h"
#include "Physics/Collision/Collision.h"
#include "Physics/Collision/Collisions.h"

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int FirstPersonCD::DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions)
{
	for (int i = 0; i < pairs.Size(); ++i)
	{
		// stuff.
		EntityPair & pair = pairs[i];
		// do detailed collision detection?
		Collision data;
		List<Collision> collisionsFound;
		// o.o
		Entity * dynamic = NULL, * dynamic2 = NULL;
		if (pair.one->physics->type == PhysicsType::DYNAMIC)
			dynamic = pair.one;
		if (pair.two->physics->type == PhysicsType::DYNAMIC)
			dynamic2 = pair.two;
		if (dynamic == 0 && dynamic2 == 0)
			continue;

		bool colliding = TestCollision(pair.one, pair.two, collisionsFound);
		if (!colliding)
			continue;
	
//		std::cout<<"\nCollision, woooo: "<<pair.one->name<<" & "<<pair.two->name;

		collisions.Add(collisionsFound);
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


