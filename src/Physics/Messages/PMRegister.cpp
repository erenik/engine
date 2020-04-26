/// Emil Hedemalm
/// 2015-02-19
/// Registering (much older originally)

#include "PhysicsMessage.h"
#include "../PhysicsManager.h"
#include "File/LogFile.h"

PMRegisterEntity::PMRegisterEntity(EntitySharedPtr i_entity): PhysicsMessage(PM_REGISTER_ENTITY) 
{
	entity = i_entity;
//	assert(i_entity->registeredForPhysics != true);
	if (Physics.physicalEntities.Exists(entity))
	{
      //  std::cout<<"Already registered? prewprewr";
	}
}

void PMRegisterEntity::Process()
{
	// If already registered, skip it.
	if (entity->registeredForPhysics)
		return;
//	std::cout<<"\nRegistering selection for Physics.";
	int failed = Physics.RegisterEntity(entity);
	if (failed)
		std::cout<<"\nUnable to register entity "<<entity<<".";
}


PMRegisterEntities::PMRegisterEntities(List< std::shared_ptr<Entity> > targetEntities): PhysicsMessage(PM_REGISTER_ENTITIES) 
{
	entities = targetEntities;
}

void PMRegisterEntities::Process()
{
//	std::cout<<"\nRegistering selection for Physics.";
	int failed = Physics.RegisterEntities(entities);
	if (!failed){
		LogPhysics(String(entities.Size())+" entities registered successfully.", EXTENSIVE_DEBUG);
	}
	else
		std::cout<<"\nUnable to register "<<failed<<" of "<<entities.Size()<<" entities. Make sure they all have physicsProperties.";
}
