/// Emil Hedemalm
/// 2014-08-01
/// Integrator dedicated to the MORPG-project.

#include "MORPGIntegrator.h"


/** All entities sent here should be fully dynamic! 
	Kinematic ones may or may not work (consider adding own integration function).
*/
void MORPGIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
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
void MORPGIntegrator::IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds)
{

}


void MORPGIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{

	PhysicsProperty * pp = forEntity->physics;
	Vector3f & position = forEntity->position;

	// Add regular velocity (from physics and effects)
	Vector3f velocity = pp->velocity;
	// Add the relative velocity (from player desired movement)
	velocity += forEntity->rotationMatrix.Product(pp->relativeVelocity);

	Vector3f lookAt = forEntity->rotationMatrix.Product(Vector3f(0,0,2.f));

	Vector3f distanceTraveled;
	if (velocity.MaxPart())
	{
		//... and travel!
		distanceTraveled = velocity  * timeInSeconds;
		forEntity->position += distanceTraveled;
	}	
	// Accelerate in the looking-direction
	Vector3f localAcceleration = forEntity->rotationMatrix.Product(pp->acceleration);
	pp->velocity += localAcceleration * timeInSeconds;

	// Apply linear damping
	pp->velocity *= pow(pp->linearDamping, timeInSeconds);

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
	}
	// Recalculate matrix after integration is done.
	forEntity->RecalculateMatrix();
}