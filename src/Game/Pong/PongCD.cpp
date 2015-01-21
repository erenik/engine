/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a simple Pong game

#include "PongCD.h"

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int PongCD::DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions)	
{ 
	return 0;
}
/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int PongCD::DetectCollisions(List<Entity*> & entities, List<Collision> & collisions)
{
	int numCol = 0;
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (!entity->physics->collissionsEnabled)
			continue;
		// Must be at least one dynamic entity to get a collision.
		if (entity->physics->type != PhysicsType::DYNAMIC)
			continue;
		// Skip sleeping ones.
		if (entity->physics->velocity.MaxPart() == 0)
			continue;
		for (int j = 0; j < entities.Size(); ++j)
		{
			Entity * entity2 = entities[j];
			/// Check filter.
			int valid = entity->physics->collisionFilter & entity2->physics->collisionCategory;
			if (!valid)
				continue;
			valid = entity2->physics->collisionFilter & entity->physics->collisionCategory;
			if (!valid)
				continue;
			if (entity == entity2)
				continue;
			if (!entity2->physics->collissionsEnabled)
				continue;
			numCol += CollisionDetector::DetectCollisions(entity, entity2, collisions);
		}
	}
	return numCol;
}


