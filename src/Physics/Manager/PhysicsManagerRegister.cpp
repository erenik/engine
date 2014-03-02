/// Emil Hedemalm
/// 2013-03-07
#include "../PhysicsProperty.h"
#include "../PhysicsManager.h"
#include "PhysicsLib/AABBSweeper.h"

/** Registers an Entity to take part in physics calculations. This requires that the Entity has the physics attribute attached.
	Returns 0 upon success, 1 if it's lacking a physics attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
*/
int PhysicsManager::RegisterEntity(Entity * newEntity){
	int entitiesInOctree = entityCollissionOctree->RegisteredEntities();
	int physicalEntitiesNum = physicalEntities.Size();
	int aabbSweeperNodes = aabbSweeper->Nodes();
	int collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
	std::cout<<"\nPre register: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);

	/// Assertion not valid anymore as some entities are not interested in collissions..!
//	assert(entitiesInOctree == physicalEntitiesNum);
	if (newEntity->physics == NULL){
		std::cout<<"\nINFO: Entity lacking physics property. Adding it for you!";
		AttachPhysicsTo(newEntity);
	}
	if (newEntity->registeredForPhysics){
		std::cout<<"\nReturn here. We're already registered.";
	//	assert(false && "WARNING: Entity already registered for physics in PhysicsManager::RegisterEntity");
		return 0;
	}
	assert(!physicalEntities.Exists(newEntity) && "Entity already registered for Physics!");
	if (physicalEntities.Exists(newEntity))
		return 4;
	physicalEntities.Add(newEntity);

    /// Update size/AABB/etc.
    newEntity->physics->UpdateProperties(newEntity);

	/// Add only to octree/AABB-list if flagged to enable collissions!!
	if (newEntity->physics->collissionsEnabled){
		entityCollissionOctree->AddEntity(newEntity);
		assert(newEntity->physics->octreeNode);
		std::cout<<"\nRegistering entity for AABBSweeper..";
		aabbSweeper->RegisterEntity(newEntity);
	}
	else {
		std::cout<<"\nWARNING: Entity: "<<newEntity->name<<" not flagged for physics! Is this the intent?";
	}


	/// Check if dynamic or otherwise and add to their arrays too
	if (newEntity->physics->type == PhysicsType::DYNAMIC){
		assert(!dynamicEntities.Exists(newEntity) && "Entity already registered for dynamic calculations!");
		dynamicEntities.Add(newEntity);
		newEntity->physics->state = 0;
	}
	else {
		newEntity->physics->state |= PhysicsState::IN_REST;
	}

	Physics.EnsurePhysicsMeshIfNeeded(newEntity);

	entitiesInOctree = entityCollissionOctree->RegisteredEntities();
	physicalEntitiesNum = physicalEntities.Size();
	aabbSweeperNodes = aabbSweeper->Nodes();
	collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
	std::cout<<"\nPost register: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);


	/// Recalculate AABB/OBB-data.
    newEntity->physics->aabb.Recalculate(newEntity);
    newEntity->physics->obb.Recalculate(newEntity);

//	assert(entitiesInOctree == physicalEntitiesNum);

	// If static, set NULL inertia matrices.
	if (newEntity->physics->type == PhysicsType::STATIC){
		newEntity->physics->inertiaTensorInverted = Matrix3f(0,0,0,0,0,0,0,0,0);
		newEntity->physics->inverseMass = 0.0f;
	}

	// Finally mark the entity as registered for physics..!
	newEntity->registeredForPhysics = true;
	/// Bind owner of the property.
	newEntity->physics->owner = newEntity;
	return 0;
}
/** Registers a selection of entities to take part in physics calculations. This requires that the entities have physics attributes attached.
	Returns 0 upon success or a positive number equal to the amount of entities that it failed to register.
*/
int PhysicsManager::RegisterEntities(List<Entity*> & targetEntities){
	int failedRegistrations = 0;
	for (int i = 0; i < targetEntities.Size(); ++i){
		if (RegisterEntity(targetEntities[i]) != 0)
			++failedRegistrations;
	}
	return failedRegistrations;
}


/// Unregisters an Entity from the physics calculations. Returns 0 if it found the Entity and successfully removed it, 1 if not.
int PhysicsManager::UnregisterEntity(Entity * entityToRemove){
	int found = 0;
	int entitiesInOctree = entityCollissionOctree->RegisteredEntities();
	int physicalEntitiesNum = physicalEntities.Size();
	int aabbSweeperNodes = aabbSweeper->Nodes();
	assert(entitiesInOctree <= physicalEntitiesNum);
	int collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
	std::cout<<"\nPre unregister: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);

	// Remove from physical entities list
	bool removedResult = physicalEntities.Remove(entityToRemove) ;
	if (!removedResult)
		return 1;
	assert(removedResult && "Trying to unregister entity that has not been previously registered!");

    std::cout<<"\nCollission enabled for entity? "<<entityToRemove->physics->collissionsEnabled;

	/// Remove from octree/AABB-sweeper
	if (entityToRemove->physics->collissionsEnabled){
		removedResult = entityCollissionOctree->RemoveEntity(entityToRemove);
		std::cout<<"\nUnregistering from aabbsweeper...";
		aabbSweeper->UnregisterEntity(entityToRemove);
	}
	else {
		while(entityCollissionOctree->Exists(entityToRemove)){
			std::cout<<"\nERROR: Entity "<<entityToRemove->name<<" existed in Collission octree without having the collissionsEnabled flag! What are you doing?!";
			removedResult = entityCollissionOctree->RemoveEntity(entityToRemove);
		}
	}
	assert(removedResult && "Unable to remove entity from Octree!");

	/// Remove from dynamic list if applicable
	if (entityToRemove->physics->type == PhysicsType::DYNAMIC)
		assert(dynamicEntities.Remove(entityToRemove) && "Trying to unregister entity that has not been previously registered for dynamic calculations!");

	/// Mark as not registerd for physics anymore.
	entityToRemove->registeredForPhysics = false;

	entitiesInOctree = entityCollissionOctree->RegisteredEntities();
	physicalEntitiesNum = physicalEntities.Size();
	aabbSweeperNodes = aabbSweeper->Nodes();
	collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
	std::cout<<"\nPost unregister: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);

//	assert(entitiesInOctree == physicalEntitiesNum);
	return 0;
}

/** Unregisters a selection of entities from physics calculations.
	Returns 0 upon success or a positive number equal to the amount of entities that it failed to unregister.
*/
int PhysicsManager::UnregisterEntities(List<Entity*> & targetEntities){
	int failedUnregistrations = 0;
	for (int i = 0; i < targetEntities.Size(); ++i){
		if (UnregisterEntity(targetEntities[i]) != 0)
			++failedUnregistrations;
	}
	return failedUnregistrations;
}

/// Unregisters all entities from physics calculations, and clears the collission entityCollissionOctree as well.
int PhysicsManager::UnregisterAllEntities(){
	int failedUnregistrations = 0;
	int entities = this->physicalEntities.Size();
	for (int i = 0; i < entities; ++i){
		if (UnregisterEntity(this->physicalEntities[0]) != 0)
			++failedUnregistrations;
	}

	/// Clear the octree just in case
	int num = entityCollissionOctree->RegisteredEntities();
	if (num){
		std::cout<<"\nERROR: Collission octree had remaining entities despite having an UnregisterAllEntities called. Some flags are probably wrong..!";
		entityCollissionOctree->clearAll();
	}

	return failedUnregistrations;
}
