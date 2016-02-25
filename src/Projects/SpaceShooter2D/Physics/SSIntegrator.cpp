/// Emil Hedemalm
/// 2015-01-21
/// Integrator

#include "SSIntegrator.h"
#include "SpaceShooter2D/SpaceShooter2D.h"

extern Entity * playerEntity;

SSIntegrator::SSIntegrator(float zPlane)
{
	constantZ = zPlane;
}


void SSIntegrator::IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds)
{
	if (levelEntity)
	{
		frameMin = levelEntity->worldPosition - playingFieldHalfSize;
		frameMax = levelEntity->worldPosition + playingFieldHalfSize;
	}
	static int shipID = ShipProperty::ID();
	Timer timer;
	timer.Start();
	for (int i = 0; i < dynamicEntities.Size(); ++i)
	{
		Entity * dynamicEntity = dynamicEntities[i];
		IntegrateVelocity(dynamicEntity, timeInSeconds);
		/// Apply bounding for player in other manner...
		/// Check if player
//		ShipProperty * sp = (ShipProperty*) dynamicEntity->GetProperty(shipID);
		// If so, limit to inside the radiusiusius
		if (dynamicEntity == playerShip->entity)
		{
	//		std::cout<<"\nShip property: "<<sp<<" ID "<<sp->GetID()<<" allied: "<<sp->ship->allied;
			/// Adjusting local position may not help ensuring entity is within bounds for child entities.
//			assert(dynamicEntity->parent == 0);
			if (dynamicEntity->parent != 0)
			{
				std::cout<<"\nDE: "<<dynamicEntity->name;
				return;
			}
			Vector3f & position = dynamicEntity->localPosition;
			ClampFloat(position[0], frameMin[0], frameMax[0]);
			ClampFloat(position[1], frameMin[1], frameMax[1]);		
		}
	}
	timer.Stop();
	integrationTimeMs = (int) timer.GetMs();
	
	timer.Start();
	RecalculateMatrices(dynamicEntities);
	timer.Stop();
	entityMatrixRecalcMs = (int)timer.GetMs();
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
	
float velocitySmoothingLast = 0.f;
float smoothingFactor = 0.f;

void SSIntegrator::IntegrateVelocity(Entity * forEntity, float timeInSeconds)
{
	PhysicsProperty * pp = forEntity->physics;
	if (pp->paused)
		return;
	Vector3f & localPosition = forEntity->localPosition;
	Vector3f & velocity = pp->velocity;
	/// For linear damping.
	float linearDamp = pow(pp->linearDamping, timeInSeconds);
	velocity *= linearDamp;

	pp->currentVelocity = velocity;
	if (velocitySmoothingLast != pp->velocitySmoothing)
	{
		smoothingFactor = pow(pp->velocitySmoothing, timeInSeconds);
		velocitySmoothingLast = pp->velocitySmoothing;
	}
	pp->smoothedVelocity = pp->smoothedVelocity * smoothingFactor + pp->currentVelocity * (1 - smoothingFactor);
//	forEntity->position += forEntity->physics->velocity * timeInSeconds;
	localPosition += pp->smoothedVelocity * timeInSeconds;
	if (pp->relativeVelocity.MaxPart())
	{
		Vector3f velocity = forEntity->rotationMatrix * pp->relativeVelocity;
		localPosition += velocity * timeInSeconds;
	}
	if (pp->angularVelocity.MaxPart())
	{
		forEntity->rotation += pp->angularVelocity * timeInSeconds;
		forEntity->hasRotated = true;
	}

	if (constantZ)
	{
		localPosition[2] = constantZ;
		forEntity->physics->velocity[2] = 0;
	}

	// Force rot to follow vel.
	if (pp->faceVelocityDirection)
	{
		if ((pp->currentVelocity.MaxPart() == 0))
			return;
		// 
		Vector3f & cVel = pp->currentVelocity;
		// Check Z is 0.
		if (cVel.z != 0)
		{	
			/// Unregister
			std::cout<<"\nBAD VEL";
			PhysicsMan.QueueMessage(new PMUnregisterEntity(forEntity));
//		assert(cVel.z == 0);
			return;
		}
		Matrix4f & rot = forEntity->rotationMatrix;
		Vector2f up(0,1);
		Angle ang(up);
		Vector2f normVel = cVel.NormalizedCopy();
		Angle look(normVel);
		Angle toLook = look - ang;
		forEntity->rotation.x = PI / 2;
		forEntity->rotation.y = toLook.Radians();
		forEntity->hasRotated = true;
	}
}
