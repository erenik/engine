// Emil Hedemalm
// 2013-07-30

#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

PMSet::PMSet(int target, float fValue)
: PhysicsMessage(PM_SET), target(target), floatValue(fValue)
{

	switch(target){
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
		case INTEGRATOR:
			break;
		default:
			assert(false && "Invalid target in PMSet");
	}
}

void PMSet::Process(){
	switch(target)
	{
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
			Physics.pauseOnCollission = bValue;
			break;
		case INTEGRATOR:
			Physics.integrator = iValue;
			break;
	}
	return;
}