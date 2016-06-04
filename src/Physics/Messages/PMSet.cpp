// Emil Hedemalm
// 2013-07-30

#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

#include "Physics/Integrator.h"
#include "Physics/CollisionResolver.h"
#include "Physics/CollisionDetector.h"

#include "File/LogFile.h"

PMSet::PMSet(int target, const Vector3f & value)
: PhysicsMessage(PM_SET), target(target), vec3fValue(value)
{
	dataType = VEC3F;
	switch(target)
	{
		case PT_GRAVITY:
			break;
		default:
			assert(false);
	}
}


PMSet::PMSet(int target, float fValue)
: PhysicsMessage(PM_SET), target(target), floatValue(fValue)
{

	switch(target)
	{
		case PT_SIMULATION_SPEED:
		case PT_AIR_DENSITY:
		case PT_GRAVITY:
		case PT_DEFAULT_DENSITY:
		case PT_LINEAR_DAMPING:
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}

PMSet::PMSet(int target, bool bValue)
: PhysicsMessage(PM_SET), target(target), bValue(bValue)
{
	switch(target){
		case PT_PAUSE_ON_COLLISSION:
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}


PMSet::PMSet(Integrator * newPhysicsIntegrator)
	: PhysicsMessage(PM_SET), target(PT_PHYSICS_INTEGRATOR), i(newPhysicsIntegrator)
{
}

PMSet::PMSet(CollisionResolver * collisionResolver)
	: PhysicsMessage(PM_SET), target(PT_COLLISION_RESOLVER), cr(collisionResolver)
{
}

PMSet::PMSet(CollisionDetector * cd)
	: PhysicsMessage(PM_SET), target(PT_COLLISION_DETECTOR), cd(cd)
{}


void PMSet::Process()
{
	switch(target)
	{
		case PT_SIMULATION_SPEED:
			Physics.simulationSpeed = floatValue;
			break;
		case PT_AIR_DENSITY:
			Physics.airDensity = floatValue;
			break;
		case PT_DEFAULT_DENSITY:
			Physics.defaultDensity = floatValue;
			break;
		case PT_GRAVITY:
			switch(dataType)
			{
			case VEC3F:
				Physics.gravitation = vec3fValue;
				PhysicsMan.physicsIntegrator->gravity = vec3fValue;
				break;
			case FLOAT:
				Physics.gravitation[1] = floatValue;
				break;
			}
			break;
		case PT_PAUSE_ON_COLLISSION:
			Physics.pauseOnCollision = bValue;
			break;
		case PT_PHYSICS_INTEGRATOR: 
		{
			PhysicsManager & physics = PhysicsMan;
			// Same. Skip.
			if (physics.physicsIntegrator == i)
				break;
			// Delete theo ld one?
			if (physics.physicsIntegrator)
			{
				delete physics.physicsIntegrator;
				physics.physicsIntegrator = NULL;
			}
			LogPhysics("Setting new integrator", INFO);
			physics.physicsIntegrator = i;
			break;
		}
		case PT_COLLISION_RESOLVER:	
		{
			PhysicsManager & physics = PhysicsMan;
			if (physics.collisionResolver == cr)
				break;
			// Delete theo ld one?
			if (physics.collisionResolver)
				delete physics.collisionResolver;
			LogPhysics("Setting new collision resolver", INFO);
			physics.collisionResolver = cr;
			break;
		}
		case PT_COLLISION_DETECTOR:	
		{
			PhysicsManager & physics = PhysicsMan;
			if (physics.collisionDetector == cd)
				break;
			if (physics.collisionDetector)
				delete physics.collisionDetector;
			LogPhysics("Setting new collision detector", INFO);
			physics.collisionDetector = cd;
			break;
		}
	}
	return;
}

PMSeti::PMSeti(int target, int iValue)
: PhysicsMessage(PM_SET), target(target), iValue(iValue)
{
	switch(target)
	{
		case PT_AABB_SWEEPER_DIVISIONS:
			break;
		case PT_INTEGRATOR_TYPE:
			break;
		case PT_PHYSICS_INTEGRATOR:
		case PT_COLLISION_DETECTOR:
		case PT_COLLISION_RESOLVER:
			assert(iValue == 0);
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}

void PMSeti::Process()
{
	PhysicsManager & physics = PhysicsMan;
	switch(target)
	{
//		case PT_AABB_SWEEPER_AXES_SCALE: 
	//		minZ
		//	break;
		case PT_AABB_SWEEPER_DIVISIONS:
			physics.aabbSweeper->divisions = iValue;
			physics.aabbSweeper->CreateAxes();
			break;
		case PT_INTEGRATOR_TYPE:
			Physics.integratorType = iValue;
			break;
		case PT_PHYSICS_INTEGRATOR: 
		{
			// Same. Skip.
			if (physics.physicsIntegrator == NULL)
				break;
			// Delete theo ld one?
			if (physics.physicsIntegrator)
			{
				delete physics.physicsIntegrator;
				physics.physicsIntegrator = NULL;
			}
			LogPhysics("Setting new integrator", INFO);
			physics.physicsIntegrator = NULL;
			break;
		}
		case PT_COLLISION_RESOLVER:	
		{
			if (physics.collisionResolver == NULL)
				break;
			// Delete theo ld one?
			if (physics.collisionResolver)
				delete physics.collisionResolver;
			LogPhysics("Setting new collision resolver", INFO);
			physics.collisionResolver = NULL;
			break;
		}
		case PT_COLLISION_DETECTOR:	
		{
			if (physics.collisionDetector == 0)
				break;
			if (physics.collisionDetector)
				delete physics.collisionDetector;
			LogPhysics("Setting new collision detector", INFO);
			physics.collisionDetector = NULL;
			break;
		}
	}	
}

