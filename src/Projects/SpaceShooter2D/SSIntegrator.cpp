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
	if (spaceShooter->levelEntity)
	{
		frameMin = spaceShooter->levelEntity->position - spaceShooter->playingFieldHalfSize;
		frameMax = spaceShooter->levelEntity->position + spaceShooter->playingFieldHalfSize;
	}
	static int shipID = ShipProperty::ID();
	Timer timer;
	timer.Start();
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		IntegrateVelocity(dynamicEntity, timeInSeconds);
		/// Check if player
		ShipProperty * sp = (ShipProperty*) dynamicEntity->GetProperty(shipID);
		// If so, limit to inside the radiusiusius
		if (sp && sp->ship->allied)
		{
	//		std::cout<<"\nShip property: "<<sp<<" ID "<<sp->GetID()<<" allied: "<<sp->ship->allied;
			Vector3f & position = dynamicEntity->position;
			ClampFloat(position.x, frameMin.x, frameMax.x);
			ClampFloat(position.y, frameMin.y, frameMax.y);		
		}
	}
	timer.Stop();
	integrationTimeMs = timer.GetMs();
	
	timer.Start();
	RecalculateMatrices(dynamicEntities);
	timer.Stop();
	entityMatrixRecalcMs = timer.GetMs();
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
	RecalculateMatrices(kinematicEntities);
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
}
