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
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * entity = dynamicEntities[i];
		IntegrateVelocity(entity, timeInSeconds);
	}
};
/** All entities sent here should be fully kinematic! 
	If not subclassed, the standard IntegrateEntities is called.
*/
void FirstPersonIntegrator::IntegrateKinematicEntities(List<Entity*> & entities, float timeInSeconds)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		IntegrateVelocity(entity, timeInSeconds);
	}
}


void FirstPersonIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{

	PhysicsProperty * pp = forEntity->physics;
	Vector3f & position = forEntity->position;
	Vector3f & relativeVelocity = pp->relativeVelocity;

	// Add regular velocity (from physics and effects)
	Vector3f velocity = pp->velocity;
	Vector3f lookAt = forEntity->LookAt();

	Vector3f distanceTraveled;
	/// Regular velocity, due to external factors, acceleration, etc.
	if (velocity.MaxPart())
	{
		//... and travel!
		distanceTraveled = velocity  * timeInSeconds;
		forEntity->position += distanceTraveled;
	}	
	/// Player induced / controlled constant velocity in relative direction?
	Vector3f relVelWorldSpaced;
	if (relativeVelocity.MaxPart())
	{
		// Add it.
		Vector3f relVel = relativeVelocity;
		relVel.z *= -1;
		relVelWorldSpaced = forEntity->rotationMatrix * relVel;
		forEntity->position += relVelWorldSpaced * timeInSeconds;
	}

	/// Apply gravity
	if (applyGravity)
		pp->velocity += Physics.GetGravitation() * pp->gravityMultiplier * timeInSeconds;

	// Accelerate in the looking-direction
	Vector3f localAcceleration = forEntity->rotationMatrix.Product(pp->acceleration);
	pp->velocity += localAcceleration * timeInSeconds;

	// Apply linear damping
	pp->velocity *= pow(pp->linearDamping, timeInSeconds);

	bool rotated = false;
	// Force rot to follow vel.
	if (pp->faceVelocityDirection && (relVelWorldSpaced.MaxPart() || velocity.MaxPart()))
	{
		// From default of 0,0,-1
		Vector3f defaultDir(0,0,-1);
		Vector3f vec1 = (relVelWorldSpaced.MaxPart() > 0)? relVelWorldSpaced : velocity;
		vec1.Normalize();
		Angle a(defaultDir.x, defaultDir.z), b(vec1.x, vec1.z);
		Angle to = b - a;
		if (to.IsGood())
		{
			float angle = to.Radians();
			Quaternion q(Vector3f(0,1,0), angle);
			q.Normalize();
			forEntity->physics->orientation = q;
			rotated = true;
	//		forEntity->rotationMatrix = q.Matrix();
		}
	}
	else 
	{
		// Rotate.
		Quaternion rotation(pp->angularVelocityQuaternion);
		if (rotation.w && rotation.MaxPart())
		{
			// Multiple the amount to rotate with time.
			rotation.angle *= timeInSeconds;
			// Recalculate it so that it becomes a unit quaternion again.
			rotation.RecalculateXYZW();
			pp->orientation = pp->orientation * rotation;
			//.. and don't forget to normalize it or it will die.
			pp->orientation.Normalize();
			rotated = true;
		}
	}
	// Recalculate matrix after integration is done.
	forEntity->RecalculateMatrix(rotated);

	assert(pp->velocity[0] == pp->velocity[0]);
	if (pp->velocity[0] != pp->velocity[0])
		pp->velocity = Vector3f();

}