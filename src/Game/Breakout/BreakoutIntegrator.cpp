/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for pong.

#include "BreakoutIntegrator.h"
#include "Message/MessageManager.h"
#include "BreakoutBallProperty.h"

BreakoutIntegrator::BreakoutIntegrator(float zPlane)
{
	constantZ = zPlane;
}


void BreakoutIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
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
void BreakoutIntegrator::IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds)
{
	for (int i = 0; i < kinematicEntities.Size(); ++i)
	{
		Entity * kinematicEntity = kinematicEntities[i];
		IntegrateVelocity(kinematicEntity, timeInSeconds);
	}
}
	

void BreakoutIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{
	PhysicsProperty * dp = forEntity->physics;
	Vector3f & position = forEntity->position;
	Vector3f & velocity = dp->velocity;
	float speed = velocity.Length();
	forEntity->position += forEntity->physics->velocity  * timeInSeconds;
	
	/// Check if it's da ball.
	BreakoutBallProperty * pbp = forEntity->GetProperty<BreakoutBallProperty>();
	if (pbp && speed)
	{
		/// Set velocity appropriately.
		float minVel = pbp->minimumVerticalVelocity;
		if (speed < minVel)
			velocity *= minVel / speed * 1.01f;
	}


	forEntity->position.z = constantZ;
	forEntity->physics->velocity.z = 0;

	forEntity->RecalculateMatrix();

	// Goal!
	if (position.x > frameMax.x)
	{
		velocity = Vector3f();
		MesMan.QueueMessages("RightGoal");
	}
	else if (position.x < frameMin.x)
	{
		velocity = Vector3f();
		MesMan.QueueMessages("LeftGoal");
	}
		
	if (position.y >= frameMax.y)
	{
		velocity.y = -AbsoluteValue(velocity.y);
		position.y = frameMax.y;
	}
	else if (position.y <= frameMin.y)
	{
		velocity.y = +AbsoluteValue(velocity.y);
		position.y = frameMin.y;
	}
}
