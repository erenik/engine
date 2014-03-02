
#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

void PMSetSpeed::Process(){ 
	Physics.simulationSpeed = speedMultiplier; 
};