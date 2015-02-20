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
	__m128 timeSSE = _mm_load1_ps(&timeInSeconds);
	float zero = 0.f;
	__m128 defaultSSE = _mm_load1_ps(&zero);
#endif
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * forEntity = entities[i];

		PhysicsProperty * pp = forEntity->physics;
		Vector3f & relativeVelocity = pp->relativeVelocity;

		// Add regular velocity (from physics and effects)
		Vector3f lookAt = forEntity->LookAt();

#ifdef USE_SSE
		assert(pp->velocity.x == pp->velocity.x);

		// Optimized
		// First acceleration.
		__m128 sse = _mm_load1_ps(&pp->gravityMultiplier);
		SSEVec totalAcceleration;
		totalAcceleration.data = defaultSSE;
		/// Apply gravity.
		if (pp->gravityMultiplier && !(pp->state & PhysicsState::AT_REST))
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
		/// Multiply by time.
		pp->velocity.data = _mm_add_ps(pp->velocity.data, _mm_mul_ps(totalAcceleration.data, timeSSE));
		// Apply linear damping
		sse = _mm_load1_ps(&pp->linearDampingPerPhysicsFrame);
		pp->velocity.data = _mm_mul_ps(pp->velocity.data, sse);
		/// Start updating current velocity.
		pp->currentVelocity.data = pp->velocity.data;
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
			pp->state &= ~PhysicsState::AT_REST;
#else
		/// Apply gravity
		if (pp->gravityMultiplier && !(pp->state & PhysicsState::AT_REST))
			pp->velocity += gravity * pp->gravityMultiplier * timeInSeconds;

		// Accelerate in the looking-direction
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
#endif
		assert(pp->velocity.x == pp->velocity.x);
	}
}

void FirstPersonIntegrator::IntegratePosition(List<Entity*> & entities, float timeInSeconds)
{
#ifdef USE_SSE
	__m128 timeSSE = _mm_load1_ps(&timeInSeconds);
	timeSSE.m128_f32[3] = 0;
#endif
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * forEntity = entities[i];
		PhysicsProperty * pp = forEntity->physics;
		Vector3f & position = forEntity->position;
#ifdef USE_SSE
		position.data = _mm_add_ps(position.data, _mm_mul_ps(pp->currentVelocity.data, timeSSE));
#else
		/// First position. Simple enough.
		Vector3f distanceTraveled = pp->currentVelocity * timeInSeconds;
		forEntity->position += distanceTraveled;
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
	}
}
