/// Emil Hedemalm
/// 2015-01-21
/// Integrator

#include "SSIntegrator.h"
#include "SpaceShooter2D.h"

SSIntegrator::SSIntegrator(float zPlane)
{
	constantZ = zPlane;
}


void SSIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
{
	if (levelCamera)
	{
		frameMin = levelCamera->position - Vector2f(15.f, 10.f);
		frameMax = levelCamera->position + Vector2f(15.f, 10.f);
	}
	Timer timer;
	timer.Start();
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		IntegrateVelocity(dynamicEntity, timeInSeconds);
	}
	timer.Stop();
	int micros = timer.GetMs();
	timer.Start();
	RecalculateMatrices(dynamicEntities);
	timer.Stop();
	int micros2 = timer.GetMs();
}



/** All entities sent here should be fully kinematic! 
	If not subclassed, the standard IntegrateEntities is called.
*/
void SSIntegrator::IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds)
{
	for (int i = 0; i < kinematicEntities.Size(); ++i)
	{
		Entity * kinematicEntity = kinematicEntities[i];
		IntegrateVelocity(kinematicEntity, timeInSeconds);
	}
}
	

void SSIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{
	PhysicsProperty * pp = forEntity->physics;
	if (pp->paused)
		return;
	Vector3f & position = forEntity->position;
	Vector3f & velocity = pp->velocity;
	forEntity->position += forEntity->physics->velocity  * timeInSeconds;

	if (constantZ)
	{
		forEntity->position.z = constantZ;
		forEntity->physics->velocity.z = 0;
	}

	/// Check if player
	// If so, limit to inside the radiusiusius
	ShipProperty * sp = forEntity->GetProperty<ShipProperty>();
	if (sp && sp->ship->allied)
	{
		Vector3f & position = forEntity->position;
		ClampFloat(position.x, frameMin.x, frameMax.x);
		ClampFloat(position.y, frameMin.y, frameMax.y);		
	}
}
