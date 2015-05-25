/// Emil Hedemalm
/// 2014-08-06
/// An integrator suitable for a first-person game. Works well together with the FirstPersonCD, FirstPersonCR and FirstPersonPlayerProperty.

#include "FirstPersonIntegrator.h"

#include "Physics/PhysicsManager.h"

FirstPersonIntegrator::FirstPersonIntegrator()
{
	applyGravity = true;
}

/** All entities sent here should be fully dynamic!
	Kinematic ones may or may not work (consider adding own integration function).
*/
void FirstPersonIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
{

	Timer timer;
	timer.Start();
	List<Entity*> & forceBasedEntities = PhysicsMan.forceBasedEntities;
	Timer timer2;
	timer2.Start();
	if (forceBasedEntities.Size())
	{
		/// Provides default "scientific" rigid-body based simulation handling of forces, torques, etc.
		CalculateForces(forceBasedEntities);
		UpdateMomentum(forceBasedEntities, timeInSeconds);
		DeriveVelocity(forceBasedEntities);
	}
	timer2.Stop();
	int forcesMomenumVelocity = timer2.GetMs();

	timer2.Start();
	IntegrateVelocity(dynamicEntities, timeInSeconds);
	timer2.Stop();
	int velocityIntegration = timer2.GetMs();
	timer2.Start();
	IntegratePosition(dynamicEntities, timeInSeconds);
	timer2.Stop();
	int positionIntegration = timer2.GetMs();
	timer.Stop();
	this->integrationTimeMs = timer.GetMs();
};
/** All entities sent here should be fully kinematic!
	If not subclassed, the standard IntegrateEntities is called.
*/
void FirstPersonIntegrator::IntegrateKinematicEntities(List<Entity*> & entities, float timeInSeconds)
{
	Timer timer;
	timer.Start();
	IntegratePosition(entities, timeInSeconds);
	integrationTimeMs += timer.GetMs();
}


void FirstPersonIntegrator::IntegrateVelocity(List<Entity*> & entities, float timeInSeconds)
{
#ifdef USE_SSE
	SSEVec timeSSE;
	timeSSE.data = _mm_load1_ps(&timeInSeconds);
	float zero = 0.f;
	__m128 defaultSSE = _mm_load1_ps(&zero);
#endif

    /// Apply homogenized damping.
    float damp = pow(0.9f, timeInSeconds);

	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * forEntity = entities[i];

		PhysicsProperty * pp = forEntity->physics;
		Vector3f & relativeVelocity = pp->relativeVelocity;

		// Add regular velocity (from physics and effects)
		Vector3f lookAt = forEntity->LookAt();

        /// First acceleration
		assert(pp->velocity.x == pp->velocity.x);
#ifdef USE_SSE
		__m128 sse = _mm_load1_ps(&pp->gravityMultiplier);
		SSEVec totalAcceleration;
		totalAcceleration.data = defaultSSE;
		/// Apply gravity.
		if (pp->gravityMultiplier && !(pp->state & CollisionState::IN_REST))
			totalAcceleration.data = _mm_add_ps(totalAcceleration.data, _mm_mul_ps(gravity.data, sse));
		/// Accelerate only if requirements met.
		if (!pp->requireGroundForLocalAcceleration || (physicsNowMs - pp->lastGroundCollisionMs) < pp->isOnGroundThresholdMs)
		{
			// Accelerate in the looking-direction
			// Require acceleration, but also that the entity be considered on the ground, if needed.
			if (pp->relativeAcceleration.x || pp->relativeAcceleration.y || pp->relativeAcceleration.z)
			{
				Vector3f relAcc = pp->relativeAcceleration;
				relAcc.z *= -1;
				Vector3f worldAcceleration = forEntity->rotationMatrix.Product(relAcc);
				totalAcceleration.data = _mm_add_ps(totalAcceleration.data, worldAcceleration.data);
				assert(totalAcceleration.x == totalAcceleration.x);
			}
			// Regular acceleration.
			totalAcceleration.data = _mm_add_ps(totalAcceleration.data, pp->acceleration.data);
			assert(totalAcceleration.x == totalAcceleration.x);
		}
#else   /// Acceleration, non-SSE
		Vector3f totalAcceleration;
		if (pp->gravityMultiplier && !(pp->state & CollisionState::IN_REST))
			totalAcceleration += gravity * pp->gravityMultiplier;
		if (!pp->requireGroundForLocalAcceleration || (physicsNowMs - pp->lastGroundCollisionMs) < pp->isOnGroundThresholdMs)
		{
			if (pp->relativeAcceleration.x || pp->relativeAcceleration.y || pp->relativeAcceleration.z)
			{
				Vector3f relAcc = pp->relativeAcceleration;
				relAcc.z *= -1;
				Vector3f worldAcceleration = forEntity->rotationMatrix.Product(relAcc);
				totalAcceleration += worldAcceleration;
				assert(totalAcceleration.x == totalAcceleration.x);
			}
			// Regular acceleration.
            totalAcceleration += pp->acceleration;
        }
#endif

        /// Velocity
#ifdef USE_SSE
		pp->velocity.data = _mm_add_ps(pp->velocity.data, _mm_mul_ps(totalAcceleration.data, timeSSE.data));
		// Apply linear damping
		sse = _mm_load1_ps(&damp);
		pp->currentVelocity.data = pp->velocity.data = _mm_mul_ps(pp->velocity.data, sse);
		/// Player induced / controlled constant velocity in relative direction?
		Vector3f relVelWorldSpaced;
		if (relativeVelocity.x || relativeVelocity.y || relativeVelocity.z)
		{
			// Add it.
			Vector3f relVel;
			relVel.data = relativeVelocity.data;
			relVel.z *= -1;
			relVelWorldSpaced = forEntity->rotationMatrix * relVel;
		}
		/// Add it up.
		pp->currentVelocity.data = _mm_add_ps(pp->currentVelocity.data, relVelWorldSpaced.data);
		/// De-flag at rest if we got any acceleration or velocity?
		if (pp->currentVelocity.MaxPart())
			pp->state &= ~CollisionState::IN_REST;
#else   /// Velocity, non-SSE
        pp->velocity += totalAcceleration * timeInSeconds;
        pp->velocity *= damp;
		pp->currentVelocity = pp->velocity;
		Vector3f relVelWorldSpaced;
		if (relativeVelocity.x || relativeVelocity.y || relativeVelocity.z)
		{
			Vector3f relVel;
			relVel = relativeVelocity;
			relVel.z *= -1;
			relVelWorldSpaced = forEntity->rotationMatrix * relVel;
		}
		/// Add it up.
		pp->currentVelocity += pp->currentVelocity + relVelWorldSpaced;
		/// De-flag at rest if we got any acceleration or velocity?
		if (pp->currentVelocity.MaxPart())
			pp->state &= ~CollisionState::IN_REST;
#endif

		assert(pp->velocity.x == pp->velocity.x);
	}
}

		/* // Old non-SSE code, obsolete ?
		/// Accelerate in the looking-direction
		Vector3f localAcceleration = forEntity->rotationMatrix.Product(pp->acceleration);
		pp->velocity += localAcceleration * timeInSeconds;
		// Apply linear damping
		pp->velocity *= pp->linearDampingPerPhysicsFrame; //  pow(pp->linearDamping, timeInSeconds);
		/// Player induced / controlled constant velocity in relative direction?
		Vector3f relVelWorldSpaced;
		if (relativeVelocity.MaxPart())
		{
			// Add it.
			Vector3f relVel = relativeVelocity;
			relVel.z *= -1;
			relVelWorldSpaced = forEntity->rotationMatrix * relVel;
		}
		/// Add it up.
		pp->currentVelocity = pp->velocity + relVelWorldSpaced;
		*/


	/*
		// Was in SSE section above earlier.
		if (pp->angularAcceleration.MaxPart())
		{
			std::cout<<"\nDo the drone dance.";
			// Add rotational velocity around local forward, up and right vectors.
			Vector3f rightVec = forEntity->RightVec();
			Vector3f upVec = forEntity->UpVec();
			Vector3f lookAt = forEntity->LookAt();
			Quaternion pitch(forEntity->RightVec(), pp->angularAcceleration.x);
			Quaternion yaw(forEntity->UpVec(), pp->angularAcceleration.y);
			Quaternion roll(forEntity->LookAt(), pp->angularAcceleration.z);

			if (pitch.angle)
			{
				pp->angularVelocityQuaternion = pp->angularVelocityQuaternion * pitch;
				pp->angularVelocityQuaternion.Normalize();
			}
			if (yaw.angle)
			{
				pp->angularVelocityQuaternion = pp->angularVelocityQuaternion * yaw;
				pp->angularVelocityQuaternion.Normalize();
			}
			if (roll.angle)
			{
				pp->angularVelocityQuaternion = pp->angularVelocityQuaternion * roll;
				pp->angularVelocityQuaternion.Normalize();
			}
			forEntity->RecalculateMatrix();

			// Fetch forward-vector again.
			// Re-align the old velocity to the new forward vector!
	//		Vector3f newForward = -forEntity->rotationMatrix.GetColumn(2);
			std::cout<<"\nAngular velocity quaternion: "<<pp->angularVelocityQuaternion;
		*/

void FirstPersonIntegrator::IntegratePosition(List<Entity*> & entities, float timeInSeconds)
{
#ifdef USE_SSE
	SSEVec timeSSE;
	timeSSE.data = _mm_load1_ps(&timeInSeconds);
	timeSSE.v[3] = 0;
#endif
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * forEntity = entities[i];
		PhysicsProperty * pp = forEntity->physics;
		Vector3f & position = forEntity->position;
#ifdef USE_SSE
		position.data = _mm_add_ps(position.data, _mm_mul_ps(pp->currentVelocity.data, timeSSE.data));
#else
		/// First position. Simple enough.
		forEntity->position += pp->currentVelocity * timeInSeconds;
#endif

		/// Rotation below
		bool rotated = false;
		// Force rot to follow vel.
		if (pp->faceVelocityDirection && (pp->currentVelocity.MaxPart()))
		{
			// From default of 0,0,-1
			Vector3f defaultDir(0,0,-1);
			Vector3f vec1 = pp->currentVelocity;
			vec1.Normalize();
			Angle a(defaultDir.x, defaultDir.z), b(vec1.x, vec1.z);
			Angle to = b - a;
			if (to.IsGood())
			{
				float angle = to.Radians();
				Quaternion q(Vector3f(0,1,0), angle);
				q.Normalize();
				assert(q.x == q.x);
				forEntity->physics->orientation = q;
				forEntity->hasRotated = true;
		//		forEntity->rotationMatrix = q.Matrix();
			}
		}
		else if (pp->angularVelocityQuaternion.MaxPart())
		{
			// Rotate.
			Quaternion rotation(pp->angularVelocityQuaternion);
			// Multiple the amount to rotate with time.
			rotation.angle *= timeInSeconds;
			// Recalculate it so that it becomes a unit quaternion again.
			rotation.RecalculateXYZW();
			assert(rotation.x == rotation.x);
			pp->orientation = pp->orientation * rotation;
			assert(pp->orientation.x == pp->orientation.x);
			//.. and don't forget to normalize it or it will die.
			pp->orientation.Normalize();
			assert(pp->orientation.x == pp->orientation.x);
			forEntity->hasRotated = true;
		}
		else if (pp->relativeRotationalVelocity.MaxPart())
		{
			// o.o
			Vector3f up = forEntity->rotationMatrix.GetColumn(1);
			Vector3f forward = -forEntity->rotationMatrix.GetColumn(2);
			Vector3f right = forEntity->rotationMatrix.GetColumn(0);

			Vector3f rotation = pp->relativeRotationalVelocity * timeInSeconds;
			rotation.x *= -1;
			rotation.z *= -1;

			// Alright. So now we have rotations along the 3 main local axises...
			// Now we need to find the 3 quaternions which would apply these rotations (looking from a global perspective)
			Quaternion pitch(right, rotation[0]);
			Quaternion yaw(up, rotation[1]);
			Quaternion roll(forward, rotation[2]);

			// Now multiply these three with the current orientation quaternion one at a time... should be simple enouhg.. maybe :P
			Quaternion & orientation = pp->orientation;
			//
			orientation = orientation * pitch;
			orientation = orientation * yaw;
			orientation = orientation * roll;
			forEntity->hasRotated = true;
			forEntity->RecalculateMatrix();
			// Recalculate velocity o.o'
			Vector3f newForward = -forEntity->rotationMatrix.GetColumn(2);
			float velDotForward = pp->velocity.DotProduct(forward);
			Vector3f localZPart = velDotForward * forward;
			float localZPartLen = localZPart.Length() * (velDotForward > 0? 1 : -1);
			Vector3f otherPart = pp->velocity - localZPart;
			Vector3f oldVel = pp->velocity;
			float factor = pow(pp->velocityRetainedWhileRotating, timeInSeconds);
			pp->velocity =  localZPartLen * factor * newForward +
				(1 - factor) * 0.8f * localZPartLen * forward +
				otherPart;
			float dot = oldVel.NormalizedCopy().DotProduct(pp->velocity.NormalizedCopy());
			if (debug == -27)
				std::cout<<"\nVelcity "<<oldVel<<" -> "<<pp->velocity<<" dot: "<<dot;
		}
	}
}
