/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for Breakout-type games.

#include "SpaceShooterCR.h"
#include "Physics/Collision/Collision.h"

#include "Entity/Properties/SpaceShooter/SpaceShooterPlayerProperty.h"
#include "Entity/Properties/SpaceShooter/SpaceShooterProjectileProperty.h"
#include "Entity/Properties/SpaceShooter/SpaceShooterPowerupProperty.h"

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
		/// Velocity already aligned with collision normal, skip it.
		float velDotNormal = dp->velocity.DotProduct(c.collisionNormal);
		Vector3f & collisionNormal = c.collisionNormal;

		if (velDotNormal >= 0.0f)
		{
			return false;
		}
		// Should resolve!
		bool resolve = true;
				
		if (dynamic->physics->noCollisionResolutions ||
			staticEntity->physics->noCollisionResolutions)
			resolve = false;
		// p=p
		if (resolve)
		{
			// Skip collision resolution for now.
			/*
			float previousVelocity = velocity.Length();

			float restitution = staticEntity->physics->restitution;
			/// Velocity along the normal
			Vector3f nVelocity = velDotNormal * collisionNormal;
			/// Velocity along tangent to the collission normal
			Vector3f tVelocity = velocity - nVelocity;
			// If the normal velocity is a much larger degree smaller than the tangent velocity, omit the collision.
			if (nVelocity.Length() < tVelocity.Length() * 0.01f)
				return false;
			// Reflect the velocity.
			dp->velocity = tVelocity * (1.f - staticEntity->physics->friction) - nVelocity * restitution;
			dp->velocity.z = 0;

			float maxVelocityIncrease = 1.2f;
			*/

			// Flag it as resolved.
			c.resolved = true;
		}
		// Notify both entities of the collision that just occured.
		staticEntity->OnCollision(c);
		dynamic->OnCollision(c);
		return true;
	}
	return false;
}


