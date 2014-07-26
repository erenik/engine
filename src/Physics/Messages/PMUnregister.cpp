#include "PhysicsMessage.h"
#include "../PhysicsManager.h"
#include <cassert>

PMUnregisterEntities::PMUnregisterEntities(List<Entity*> targetEntities): PhysicsMessage(PM_UNREGISTER_ENTITIES){
	entities = targetEntities;
	for (int i = 0; i < entities.Size(); ++i){
		assert(entities[i]->registeredForPhysics);
	}
}

void PMUnregisterEntities::Process(){
	std::cout<<"\nUnregistering selection for Physics.";
	int failed = Physics.UnregisterEntities(entities);
	if (failed == 0){
		std::cout<<"\n"<<entities.Size()<<" entities unregistered successfully from Physics.";
		return;
	}
	std::cout<<"\nUnable to register "<<failed<<" of "<<entities.Size()<<" entities. Make sure they all have physicsProperties.";				
}


PMUnregisterEntity::PMUnregisterEntity(Entity * i_entity)
: PhysicsMessage(PM_UNREGISTER_ENTITY)
{
	entity = i_entity;
}

void PMUnregisterEntity::Process()
{
//	std::cout<<"\nUnregistering selection for Physics.";
	if (!entity->registeredForPhysics)
		return;
	int result = Physics.UnregisterEntity(entity);
	if (result != 0){
		std::cout<<"\nERROR: Failed to unregister entity!";
	}
}
