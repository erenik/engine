#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

PMRegisterEntity::PMRegisterEntity(Entity * i_entity): PhysicsMessage(PM_REGISTER_ENTITY) {
	entity = i_entity;
	assert(i_entity->registeredForPhysics != true);
	if (Physics.physicalEntities.Exists(entity))
	{
        std::cout<<"Already registered? prewprewr";
	}
	assert(!Physics.dynamicEntities.Exists(entity));
}

void PMRegisterEntity::Process(){
	std::cout<<"\nRegistering selection for Physics.";
	int failed = Physics.RegisterEntity(entity);
	if (failed)
		std::cout<<"\nUnable to register entity "<<entity<<".";
}


PMRegisterEntities::PMRegisterEntities(List<Entity*> targetEntities): PhysicsMessage(PM_REGISTER_ENTITIES) {
	entities = targetEntities;
}

void PMRegisterEntities::Process(){
	std::cout<<"\nRegistering selection for Physics.";
	int failed = Physics.RegisterEntities(entities);
	if (!failed){
		std::cout<<"\n"<<entities.Size()<<" entities registered successfully.";
	}
	else
		std::cout<<"\nUnable to register "<<failed<<" of "<<entities.Size()<<" entities. Make sure they all have physicsProperties.";
}
