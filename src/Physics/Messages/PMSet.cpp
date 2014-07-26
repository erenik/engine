// Emil Hedemalm
// 2013-07-30

#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

#include "Physics/Integrator.h"
#include "Physics/CollisionResolver.h"
#include "Physics/CollisionDetector.h"

PMSet::PMSet(int target, float fValue)
: PhysicsMessage(PM_SET), target(target), floatValue(fValue)
{

	switch(target)
	{
		case SIMULATION_SPEED:
		case AIR_DENSITY:
		case GRAVITY:
		case DEFAULT_DENSITY:
		case LINEAR_DAMPING:
		case ANGULAR_DAMPING:
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}

PMSet::PMSet(int target, bool bValue)
: PhysicsMessage(PM_SET), target(target), bValue(bValue)
{
	switch(target){
		case PAUSE_ON_COLLISSION:
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}

PMSet::PMSet(int target, int iValue)
: PhysicsMessage(PM_SET), target(target), iValue(iValue)
{
	switch(target){
		case INTEGRATOR_TYPE:
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}

PMSet::PMSet(Integrator * physicsIntegrator)
	: PhysicsMessage(PM_SET), target(PHYSICS_INTEGRATOR), physicsIntegrator(physicsIntegrator)
{
}

PMSet::PMSet(CollisionResolver * collisionResolver)
	: PhysicsMessage(PM_SET), target(COLLISION_RESOLVER), cr(collisionResolver)
{
}

PMSet::PMSet(CollisionDetector * cd)
	: PhysicsMessage(PM_SET), target(COLLISION_DETECTOR), cd(cd)
{}


void PMSet::Process()
{
	switch(target)
	{
		case SIMULATION_SPEED:
			Physics.simulationSpeed = floatValue;
			break;
		case LINEAR_DAMPING:
			Physics.linearDamping = floatValue;
			break;
		case ANGULAR_DAMPING:
			Physics.angularDamping = floatValue;
			break;
		case AIR_DENSITY:
			Physics.airDensity = floatValue;
			break;
		case DEFAULT_DENSITY:
			Physics.defaultDensity = floatValue;
			break;
		case GRAVITY:
			Physics.gravitation.y = floatValue;
			break;
		case PAUSE_ON_COLLISSION:
			Physics.pauseOnCollision = bValue;
			break;
		case INTEGRATOR_TYPE:
			Physics.integratorType = iValue;
			break;
		case PHYSICS_INTEGRATOR: 
		{
			// Delete theo ld one?
			if (Physics.physicsIntegrator)
				delete Physics.physicsIntegrator;
			Physics.physicsIntegrator = physicsIntegrator;
			break;
		}
		case COLLISION_RESOLVER:	
		{
			// Delete theo ld one?
			if (Physics.collisionResolver)
				delete Physics.collisionResolver;
			Physics.collisionResolver = cr;
			break;
		}
		case COLLISION_DETECTOR:	
		{
			if (Physics.collisionDetector)
				delete Physics.collisionDetector;
			Physics.collisionDetector = cd;
			break;
		}
	}
	return;
}