/// Emil Hedemalm
/// 2013-03-07
#include "../PhysicsProperty.h"
#include "../PhysicsManager.h"
#include "PhysicsLib/AABBSweeper.h"

#include "PhysicsLib/Shapes/OBB.h"
#include "Pathfinding/PathfindingProperty.h"

/** Registers an Entity to take part in physics calculations. This requires that the Entity has the physics attribute attached.
	Returns 0 upon success, 1 if it's lacking a physics attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
*/
int PhysicsManager::RegisterEntity(Entity * newEntity)
{
	int entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	int physicalEntitiesNum = physicalEntities.Size();
	int aabbSweeperNodes = aabbSweeper->Nodes();
	int collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
//	std::cout<<"\nPre register: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);

	/// Assertion not valid anymore as some entities are not interested in collissions..!
//	assert(entitiesInOctree == physicalEntitiesNum);
	if (newEntity->physics == NULL){
//		std::cout<<"\nINFO: Entity lacking physics property. Adding it for you!";
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

	PhysicsProperty * pp = newEntity->physics;
	if (!pp->aabb)
		pp->aabb = new AABB();
	if (!pp->obb)
		pp->obb = new OBB();

    /// Update size/AABB/etc.
    newEntity->physics->UpdateProperties(newEntity);

	/// Add only to octree/AABB-list if flagged to enable collissions!!
	if (newEntity->physics->collissionsEnabled){
		entityCollisionOctree->AddEntity(newEntity);
		assert(newEntity->physics->octreeNode);
//		std::cout<<"\nRegistering entity for AABBSweeper..";
		aabbSweeper->RegisterEntity(newEntity);
	}
	else {
	//	std::cout<<"\nWARNING: Entity: "<<newEntity->name<<" not flagged for physics! Is this the intent?";
	}


	/// Remove from relevant lists
	switch(newEntity->physics->type)
	{
		case PhysicsType::DYNAMIC:
		{
			assert(!dynamicEntities.Exists(newEntity) && "Entity already registered for dynamic calculations!");
			dynamicEntities.Add(newEntity);
			newEntity->physics->state = 0;
			break;
		}
		case PhysicsType::KINEMATIC:
		{
			assert(!kinematicEntities.Exists(newEntity) && "Entity already registered for dynamic calculations!");
			kinematicEntities.Add(newEntity);
			newEntity->physics->state |= PhysicsState::IN_REST;
			break;
		}
		case PhysicsType::STATIC:
		{
			newEntity->physics->state |= PhysicsState::IN_REST;
			// If static, set NULL inertia matrices.
			newEntity->physics->inertiaTensorInverted = Matrix3f(0,0,0,0,0,0,0,0,0);
			newEntity->physics->inverseMass = 0.0f;
			break;
		}
	}

	Physics.EnsurePhysicsMeshIfNeeded(newEntity);

	entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	physicalEntitiesNum = physicalEntities.Size();
	aabbSweeperNodes = aabbSweeper->Nodes();
	collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
//	std::cout<<"\nPost register: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);


	/// Recalculate AABB/OBB-data.
	if (newEntity->model)
	{
		newEntity->physics->aabb->Recalculate(newEntity);
		newEntity->physics->obb->Recalculate(newEntity);
	}
//	assert(entitiesInOctree == physicalEntitiesNum);

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
int PhysicsManager::UnregisterEntity(Entity * entityToRemove)
{
	int found = 0;
	int entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	int physicalEntitiesNum = physicalEntities.Size();
	int aabbSweeperNodes = aabbSweeper->Nodes();
	assert(entitiesInOctree <= physicalEntitiesNum);
	int collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
//	std::cout<<"\nPre unregister: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);

	// Remove from physical entities list
	bool removedResult = physicalEntities.Remove(entityToRemove) ;
	if (!removedResult)
		return 1;
	assert(removedResult && "Trying to unregister entity that has not been previously registered!");

 //   std::cout<<"\nCollision enabled for entity? "<<entityToRemove->physics->collissionsEnabled;

	/// Remove from octree/AABB-sweeper
	if (entityToRemove->physics->collissionsEnabled){
		removedResult = entityCollisionOctree->RemoveEntity(entityToRemove);
	//	std::cout<<"\nUnregistering from aabbsweeper...";
		aabbSweeper->UnregisterEntity(entityToRemove);
	}
	else {
		while(entityCollisionOctree->Exists(entityToRemove)){
	//		std::cout<<"\nERROR: Entity "<<entityToRemove->name<<" existed in Collision octree without having the collissionsEnabled flag! What are you doing?!";
			removedResult = entityCollisionOctree->RemoveEntity(entityToRemove);
		}
	}
	assert(removedResult && "Unable to remove entity from Octree!");

	/// Remove from relevant lists
	switch(entityToRemove->physics->type)
	{
		case PhysicsType::DYNAMIC:
		{
			assert(dynamicEntities.Remove(entityToRemove) && "Trying to unregister entity that has not been previously registered for dynamic calculations!");
			break;
		}
		case PhysicsType::KINEMATIC:
		{
			assert(kinematicEntities.Remove(entityToRemove) && "Trying to unregister entity that has not been previously registered for dynamic calculations!");
			break;
		}
	}

	/// Mark as not registerd for physics anymore.
	entityToRemove->registeredForPhysics = false;

	entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	physicalEntitiesNum = physicalEntities.Size();
	aabbSweeperNodes = aabbSweeper->Nodes();
	collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collissionsEnabled){
            ++collidingEntities;
        }
    }
//	std::cout<<"\nPost unregister: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	assert(aabbSweeperNodes == collidingEntities*2);

	// Check if marked for deletion. If so delete the PhysicsProperty too.
	if (entityToRemove->flaggedForDeletion)
	{
//		delete entityToRemove->physics;
//		entityToRemove->physics = NULL;
	}

	if (entityToRemove->pathfindingProperty)
	{
		delete entityToRemove->pathfindingProperty;
		entityToRemove->pathfindingProperty = NULL;
	}

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

/// Unregisters all entities from physics calculations, and clears the collission entityCollisionOctree as well.
int PhysicsManager::UnregisterAllEntities(){
	int failedUnregistrations = 0;
	int entities = this->physicalEntities.Size();
	for (int i = 0; i < entities; ++i){
		if (UnregisterEntity(this->physicalEntities[0]) != 0)
			++failedUnregistrations;
	}

	/// Clear the octree just in case
	int num = entityCollisionOctree->RegisteredEntities();
	if (num){
		std::cout<<"\nERROR: Collision octree had remaining entities despite having an UnregisterAllEntities called. Some flags are probably wrong..!";
		entityCollisionOctree->clearAll();
	}

	return failedUnregistrations;
}
