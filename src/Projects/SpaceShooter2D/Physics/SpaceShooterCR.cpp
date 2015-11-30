/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for Breakout-type games.

#include "SpaceShooterCR.h"
#include "Physics/Collision/Collision.h"

/// Returns false if the colliding entities are no longer in contact after resolution.
bool SpaceShooterCR::ResolveCollision(Collision & c)
{
	// OK.... 
	// Check stuff.
	if (c.collisionNormal.MaxPart() == 0)
		return false;
    

	if  (c.dynamicEntities.Size() == 0)
	{
		c.ExtractData();
	}

	/// Wall? just reflect velocity so we're not going into the wall anymore.
	if (c.staticEntities.Size() || c.kinematicEntities.Size())
	{
		Entity * dynamic = c.dynamicEntities[0],
			* staticEntity;
		if (c.staticEntities.Size())
			staticEntity = c.staticEntities[0];
		else
			staticEntity = c.kinematicEntities[0];

		PhysicsProperty * dp = dynamic->physics;
		Vector3f & velocity = dp->velocity;
		// Should resolve!
		bool resolve = true;
				
		if (dynamic->physics->noCollisionResolutions ||
			staticEntity->physics->noCollisionResolutions)
			resolve = false;
		// p=p
		if (resolve)
		{
			// Flag it as resolved.
			c.resolved = true;
		}
		if (resolve)
		{
			// Notify both entities of the collision that just occured.
			c.one->OnCollision(c);
			c.two->OnCollision(c);
		}
		return true;
	}
	// Two dynamic entities.
	else 
	{
		// Notify both entities of the collision that just occured.
		c.one->OnCollision(c);
		c.two->OnCollision(c);
	}

	return false;
}



/// Resolves collisions.
int SpaceShooterCR::ResolveCollisions(List<Collision> collisions)
{
	/// sup.
	for (int i = 0; i < collisions.Size(); ++i)
	{
		ResolveCollision(collisions[i]);
	}
	return collisions.Size();
}
