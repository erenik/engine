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
	if (forceBasedEntities.Size())
	{
		/// Provides default "scientific" rigid-body based simulation handling of forces, torques, etc.
		CalculateForces(forceBasedEntities);
		UpdateMomentum(forceBasedEntities, timeInSeconds);
		DeriveVelocity(forceBasedEntities);
	}

	IntegrateVelocity(dynamicEntities, timeInSeconds);
	IntegratePosition(dynamicEntities, timeInSeconds);
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
	
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * forEntity = entities[i];

		PhysicsProperty * pp = forEntity->physics;
		Vector3f & relativeVelocity = pp->relativeVelocity;

		// Add regular velocity (from physics and effects)
		Vector3f velocity = pp->velocity;
		Vector3f lookAt = forEntity->LookAt();

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
		pp->currentVelocity = velocity + relVelWorldSpaced;
		
		assert(pp->velocity[0] == pp->velocity[0]);
		if (pp->velocity[0] != pp->velocity[0])
			pp->velocity = Vector3f();
	}
}

void FirstPersonIntegrator::IntegratePosition(List<Entity*> & entities, float timeInSeconds)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * forEntity = entities[i];
		PhysicsProperty * pp = forEntity->physics;
		Vector3f & position = forEntity->position;
		/// First position. Simple enough.
		Vector3f distanceTraveled = pp->currentVelocity * timeInSeconds;
		forEntity->position += distanceTraveled;
		
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
