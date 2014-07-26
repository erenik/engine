/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for pong.

#include "PongIntegrator.h"
#include "Message/MessageManager.h"
#include "Time/Time.h"

PongIntegrator::PongIntegrator(float zPlane)
{
	constantZ = zPlane;
}


void PongIntegrator::IntegrateDynamicEntities(List<Entity*> dynamicEntities, float timeInSeconds)
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
void PongIntegrator::IntegrateKinematicEntities(List<Entity*> kinematicEntities, float timeInSeconds)
{
	for (int i = 0; i < kinematicEntities.Size(); ++i)
	{
		Entity * kinematicEntity = kinematicEntities[i];
		IntegrateVelocity(kinematicEntity, timeInSeconds);
	}
}
	

void PongIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{
	PhysicsProperty * dp = forEntity->physics;
	Vector3f & position = forEntity->position;
	Vector3f & velocity = dp->velocity;
	forEntity->position += forEntity->physics->velocity  * timeInSeconds;

	float speed = velocity.Length();
	static Time lastTime = Time::Now();
	Time now = Time::Now();
	int seconds = (now - lastTime).Seconds();
	if (seconds >= 1 && speed > 0)
	{
		if (speed > 0)
		{
			std::cout<<"\nSpeed: "<<speed<<" Velocity: "<<velocity.x<<" timeInSeconds: "<<timeInSeconds;
		}
		lastTime = now;
	}
	
	if (constantZ)
	{
		forEntity->position.z = constantZ;
		forEntity->physics->velocity.z = 0;
	}

	forEntity->RecalculateMatrix();

	// Goal! No! Solve with collisions!
	/*
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
	*/
		
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
