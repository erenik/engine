/// Emil Hedemalm
/// 2014-07-25
/// Custom integrator for Space shooter type games.

#include "BreakoutCR.h"
#include "Physics/Collision/Collision.h"

#include "BreakoutPaddleProperty.h"
#include "BreakoutBallProperty.h"

/// Returns false if the colliding entities are no longer in contact after resolution.
bool BreakoutCR::ResolveCollision(Collision & c)
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
			dp->velocity[2] = 0;

			/// Move out the dynamic entity
			dynamic->position += collisionNormal * (-c.distanceIntoEachOther);
			
			float maxVelocityIncrease = 1.2f;
			
			BreakoutBallProperty * pp = (BreakoutBallProperty*) dp->owner->GetProperty("BreakoutBallProperty");
			if (dp->velocity.MaxPart())
			{
				// If any velocity, re-scale it.
				if (pp)
				{	
					// Lock max y-vel.
					if (AbsoluteValue(dp->velocity[1]) > pp->maxYVel)
						dp->velocity[1] = pp->maxYVel * (dp->velocity[1] > 0 ? 1.f : -1.f);

					// Ensure X-velocity is at the exact value as specified in the ball property.
					float relVelY = pp->minimumVerticalVelocity / AbsoluteValue(dp->velocity[1]);
					if (relVelY > 0)
						dp->velocity[1] *= relVelY;
					pp->minimumVerticalVelocity += pp->velocityIncreasePerBounce;
				}
				float newVelocity = velocity.Length();
				int ratio = newVelocity / (previousVelocity * maxVelocityIncrease);
				if ( ratio > 1.f )
				{
					velocity /= ratio;
				}
			}
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


