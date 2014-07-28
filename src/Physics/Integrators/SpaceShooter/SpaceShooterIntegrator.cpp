/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for pong.

#include "SpaceShooterIntegrator.h"
#include "Message/MessageManager.h"
#include "Entity/Properties/SpaceShooter/SpaceShooterPlayerProperty.h"

SpaceShooterIntegrator::SpaceShooterIntegrator(float zPlane)
{
	constantZ = zPlane;
}


void SpaceShooterIntegrator::IntegrateDynamicEntities(List<Entity*> dynamicEntities, float timeInSeconds)
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
void SpaceShooterIntegrator::IntegrateKinematicEntities(List<Entity*> kinematicEntities, float timeInSeconds)
{
	for (int i = 0; i < kinematicEntities.Size(); ++i)
	{
		Entity * kinematicEntity = kinematicEntities[i];
		IntegrateVelocity(kinematicEntity, timeInSeconds);
	}
}
	

void SpaceShooterIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{
	PhysicsProperty * dp = forEntity->physics;
	Vector3f & position = forEntity->position;
	Vector3f & velocity = dp->velocity;
	forEntity->position += forEntity->physics->velocity  * timeInSeconds;

	if (constantZ)
	{
		forEntity->position.z = constantZ;
		forEntity->physics->velocity.z = 0;
	}

	forEntity->RecalculateMatrix();

	/// Check if player
	// If so, limit to inside the radiusiusius
	SpaceShooterPlayerProperty * sspp = (SpaceShooterPlayerProperty *) forEntity->GetProperty("SpaceShooterPlayerProperty");
	if (sspp && sspp->isPlayer)
	{
		Vector3f position = forEntity->position;
		ClampFloat(position.x, frameMin.x, frameMax.x);
		ClampFloat(position.y, frameMin.y, frameMax.y);		
	}

}
