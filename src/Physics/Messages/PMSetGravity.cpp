#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

PMSetGravity::PMSetGravity(Vector3f i_newGravity): PhysicsMessage(PM_SET_GRAVITY){
	newGravity = i_newGravity;
}

void PMSetGravity::Process(){
	std::cout<<"\nSetting new gravity: "<<newGravity;
	Physics.gravitation = newGravity;
}
