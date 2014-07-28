/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a Breakout-type game

#include "SpaceShooterCD.h"
#include "Physics/Collision/Collision.h"

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int SpaceShooterCD::DetectCollisions(List<Entity*> entities, List<Collision> & collisions)
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
		// Skip sleeping ones in the outer loop.
		if (entity->physics->velocity.MaxPart() == 0)
			continue;

		// Inner loop with entities to compare this entity to.
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
			numCol += DetectCollisions(entity, entity2, collisions);
		}
	}
	return numCol;
}




/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
int SpaceShooterCD::DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions)
{
	if (SpheresColliding(one->position, two->position, one->physics->physicalRadius + two->physics->physicalRadius))
	{
		Collision c;
		c.one = one;
		c.two = two;
		c.collisionNormal = (two->position - one->position).NormalizedCopy();
		collisions.Add(c);
		return true;
	}
	return false;
}


