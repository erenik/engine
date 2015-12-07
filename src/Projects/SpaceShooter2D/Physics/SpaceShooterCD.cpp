/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a Breakout-type game

#include "SpaceShooterCD.h"
#include "Physics/Collision/Collision.h"
#include "Entity/Entity.h"
#include "PhysicsLib/Physics.h"

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int SpaceShooterCD::DetectCollisions(List<Entity*> & entities, List<Collision> & collisions)
{
	int numCol = 0;
	List<EntityPair> pairs;
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (!entity->physics->collisionsEnabled)
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
			EntityPair pair;
			pair.one = entity;
			pair.two = entity2;
			pairs.Add(pair);
		}
	}
	DetectCollisions(pairs, collisions);
			
	return numCol;
}

/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
int SpaceShooterCD::DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions)
{
	int numCol = 0;
	for (int i = 0; i < pairs.Size(); ++i)
	{
		EntityPair & pair = pairs[i];
		Entity * entity = pair.one, * entity2 = pair.two;
		/// Check filter.
		int valid = entity->physics->collisionFilter & entity2->physics->collisionCategory;
		if (!valid)
			continue;
		valid = entity2->physics->collisionFilter & entity->physics->collisionCategory;
		if (!valid)
			continue;
		// Quick opt-out.
		if (entity == entity2)
			continue;
		if (!entity2->physics->collisionsEnabled)
			continue;

		float radiiSum = entity->physics->physicalRadius + entity2->physics->physicalRadius;
		Vector3f distance = entity->worldPosition - entity2->worldPosition;
		if (abs(distance[0]) > radiiSum)
			continue;
		if (abs(distance[1]) > radiiSum)
			continue;
		if (abs(distance[2]) > radiiSum)
			continue;
		float distanceLengthNotRooted = distance[0] * distance[0] + distance[1] * distance[1] + distance[2] * distance[2];
		float radiusPowerOf2 = radiiSum * radiiSum;
		if (distanceLengthNotRooted > radiusPowerOf2 * 1.05f)
			continue;
		
		Collision c;
		c.one = entity;
		c.two = entity2;
		c.collisionNormal = (entity2->worldPosition - entity->worldPosition).NormalizedCopy();
		collisions.Add(c);
		numCol += 1;
	}	
	return numCol;
}

/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
int SpaceShooterCD::DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions)
{
	if (SpheresColliding(one->worldPosition, two->worldPosition, one->physics->physicalRadius + two->physics->physicalRadius))
	{
		Collision c;
		c.one = one;
		c.two = two;
		c.collisionNormal = (two->worldPosition - one->worldPosition).NormalizedCopy();
		collisions.Add(c);
		return true;
	}
	return false;
}


