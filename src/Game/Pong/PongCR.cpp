/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for pong.

#include "PongCR.h"
#include "Physics/Collision/Collision.h"

#include "PongBallProperty.h"

#include "Message/MessageManager.h"

/// Returns false if the colliding entities are no longer in contact after resolution.
bool PongCR::ResolveCollision(Collision & c)
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
		else 
		{		
			float previousVelocity = velocity.Length();

			float resitution = staticEntity->physics->restitution;
			/// Velocity along the normal
            Vector3f nVelocity = velDotNormal * collisionNormal;
            /// Velocity along tangent to the collission normal
            Vector3f tVelocity = velocity - nVelocity;
			// Reflect the velocity.
			dp->velocity = tVelocity * (1.f - staticEntity->physics->friction) - nVelocity * resitution;
			dp->velocity[2] = 0;

			float maxVelocityIncrease = 1.2f;
			
			PongBallProperty * pbp = (PongBallProperty*) dp->owner->GetProperty("PongBallProperty");
			if (dp->velocity.MaxPart())
			{
				// If any velocity, re-scale it.
				if (pbp)
				{	
					// Lock max y-vel.
					if (AbsoluteValue(dp->velocity[1]) > pbp->maxYVel)
						dp->velocity[1] = pbp->maxYVel * (dp->velocity[1] > 0 ? 1.f : -1.f);

					// Ensure X-velocity is at the exact value as specified in the ball property.
					float relVelX = pbp->minimumHorizontalVelocity / AbsoluteValue(dp->velocity[0]);
					if (relVelX > 0)
						dp->velocity[0] *= relVelX;
					pbp->minimumHorizontalVelocity += pbp->velocityIncreasePerBounce;
				}
				float newVelocity = velocity.Length();
				int ratio = newVelocity / (previousVelocity * maxVelocityIncrease);
				if ( ratio > 1.f )
				{
					velocity /= ratio;
				}
			}

			if (pbp)
				pbp->OnCollision(c);
			/*
			// Goal if ball and goal!
			if (pbp && staticEntity->name.Contains("Goal"))
			{
				
				AudioMan.QueueMessage(new AMPlaySFX("PongGoal.ogg"));
			//	AudioMan.PlaySFX("PongGoal.ogg", 1.f);
				// Notify of the goal.
				MesMan.QueueMessages(GOAL_MESSAGE+String(":")+String::ToString(dynamic->position[0]));
				pbp->sleeping = true;
			}
			else if (staticEntity->name.Contains("Wall"))
			{
				AudioMan.QueueMessage(new AMPlaySFX("PongWall.ogg"));
	//			AudioMan.PlaySFX("PongWall.ogg", 1.f);
			}
	*/
		}
		return true;
	}
	return false;
}


