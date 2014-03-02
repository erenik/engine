#include "PhysicsMessage.h"
#include "../PhysicsProperty.h"

PMSetVelocity::PMSetVelocity(Entity * i_entity, Vector3f i_newVelocity): PhysicsMessage(PM_SET_VELOCITY){
	newVelocity = i_newVelocity;
	entity = i_entity;
}

void PMSetVelocity::Process(){
	entity->physics->velocity = newVelocity;
}