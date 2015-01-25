/// Emil Hedemalm
/// 2014-07-26
/// Custom integrator for a flight-simulator game, focusing on aerical movement and velocities changing due to rotation.

#include "AircraftIntegrator.h"
#include "Message/MessageManager.h"

AircraftIntegrator::AircraftIntegrator()
{
}


void AircraftIntegrator::IntegrateDynamicEntities(List<Entity*> dynamicEntities, float timeInSeconds)
{
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		IntegrateVelocity(dynamicEntity, timeInSeconds);
	}
}



/** All entities sent here should be fully kinematic! 
	If not subclassed, the standard IntegrateEntities is called.
*/
void AircraftIntegrator::IntegrateKinematicEntities(List<Entity*> kinematicEntities, float timeInSeconds)
{
	for (int i = 0; i < kinematicEntities.Size(); ++i)
	{
		Entity * kinematicEntity = kinematicEntities[i];
		IntegrateVelocity(kinematicEntity, timeInSeconds);
	}
}
	

void AircraftIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{
	PhysicsProperty * pp = forEntity->physics;
	Vector3f & position = forEntity->position;
	Vector3f & velocity = pp->velocity;
	Vector3f distanceTraveled = forEntity->physics->velocity  * timeInSeconds;
	forEntity->position += distanceTraveled;


	// Fetch relative vectors.
	Vector3f up = forEntity->rotationMatrix.GetColumn(1);
	Vector3f forward = -forEntity->rotationMatrix.GetColumn(2);
	Vector3f right = forEntity->rotationMatrix.GetColumn(0);


	Vector3f upAcc = up * pp->relativeAcceleration[1];
	Vector3f forwardAcc = forward * pp->relativeAcceleration[2];
	Vector3f rightAcc = right * pp->relativeAcceleration[0];

	Vector3f accDueToRelAcc = upAcc + forwardAcc + rightAcc;

	/// Apply acceleration to the velocity.
	velocity += accDueToRelAcc * timeInSeconds;

	// Apply damping to the velocity for testing purposes?
	velocity *= pow(pp->linearDamping, timeInSeconds);

	forEntity->RecalculateMatrix();


	if (pp->relativeRotation.MaxPart())
	{

		/// Rotate it as needed.
		// Calculate RAdius of turn
		// http://www.flightlearnings.com/2009/08/26/radius-of-turn/
		// http://en.wikipedia.org/wiki/Standard_rate_turn
		// http://www.csgnetwork.com/aircraftturninfocalc.html
		Vector3f a;
		// Use random turning radius for now..
		// 200 meter radius circle for a complete 180-degree turn (or whole circle, w/e).
		float turningRadius = 1.f;

		// Calculate degrees turned per meter/unit.
		float circumference = turningRadius * 2 * PI;
		float metersPerDegree = circumference / (360.f);
		assert(metersPerDegree);
		float degreesPerMeter = 1 / metersPerDegree;

		// Get meters traveled for this iteration.
		float metersTraveled = distanceTraveled.Length();
		float degreesToTurn = metersTraveled * degreesPerMeter;
		float radiansToTurn = DEGREES_TO_RADIANS(degreesToTurn);

		// Turn it!
		// ..
		Vector3f rotation = pp->relativeRotation * degreesToTurn;
		
		rotation = rotation.ElementDivision(Vector3f(200.f, 200.f, 200.f));

		// Set a limit to how many degrees can be turned per time-frame?
		Vector3f maxRotationInDegreesPerSecond(2.f, 1.f, 2.f);
		Vector3f rotationPerSecond = rotation / timeInSeconds;

		rotationPerSecond.Clamp(-maxRotationInDegreesPerSecond, maxRotationInDegreesPerSecond);

		rotation = rotationPerSecond * timeInSeconds;

		// Multiply be the time? No? Time already taken into account when distance traveled was considered!
	//	rotation *= timeInSeconds;

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

		forEntity->RecalculateMatrix();

		// Fetch forward-vector again. 
		// Re-align the old velocity to the new forward vector!
		Vector3f newForward = -forEntity->rotationMatrix.GetColumn(2);

		// Check how much of the velocity was aligned with the previous forward.
		float oldForwardDotVel = pp->velocity.DotProduct(forward);
		pp->velocity = pp->velocity - forward * oldForwardDotVel + newForward * oldForwardDotVel;
	}
}
