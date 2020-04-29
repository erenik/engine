/// Emil Hedemalm
/// 2013-03-07
#include "../PhysicsProperty.h"
#include "../PhysicsManager.h"
#include "PhysicsLib/AABBSweeper.h"

#include "PhysicsLib/Shapes/OBB.h"
#include "Pathfinding/PathfindingProperty.h"
#include "File/LogFile.h"
#include "Entity/EntityManager.h"

/** Registers an Entity to take part in physics calculations. This requires that the Entity has the physics attribute attached.
	Returns 0 upon success, 1 if it's lacking a physics attribute, 2 if the Entity array has been filled and 3 if the dynamic entity array has been filled.
*/
int PhysicsManager::RegisterEntity(EntitySharedPtr newEntity)
{
//	std::cout<<"\nRegistering entity for physics "<<newEntity->name;
	/// So it can be avoided when creating them and positioning manually in main code...
	newEntity->RecalculateMatrix();

	if (!EntityMan.IsGood(newEntity))
		return -1;
	int entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	int physicalEntitiesNum = physicalEntities.Size();
//	int aabbSweeperNodes = aabbSweeper->Nodes();
	int collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collisionsEnabled){
            ++collidingEntities;
        }
    }

	/// Create AABB.. unless model is missing.
	if (newEntity->model)
	{
		if (newEntity->aabb == 0)
			newEntity->aabb = new AABB();
		newEntity->aabb->Recalculate(newEntity);
	}
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
	if (!pp->obb)
		pp->obb = new OBB();

    /// Update size/AABB/etc.
    newEntity->physics->UpdateProperties(newEntity);

	/// Add only to octree/AABB-list if flagged to enable collissions!!
	if (newEntity->physics->collisionsEnabled)
	{
		if (checkType == OCTREE)
		{
			entityCollisionOctree->AddEntity(newEntity);
			assert(newEntity->physics->octreeNode);
		}
		else if (checkType == AABB_SWEEP)
		{
			aabbSweeper->RegisterEntity(newEntity);	
			//	std::cout<<"\nPost register: AABBSweeper nodes: "<<aabbSweeperNodes<<" colliding entities: "<<collidingEntities;
	//		aabbSweeperNodes = aabbSweeper->Nodes();
	//		assert(aabbSweeperNodes == collidingEntities* aabbSweeper->AxesToWorkWith() * 2);
		}
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
			newEntity->physics->state |= CollisionState::IN_REST;
			break;
		}
		case PhysicsType::STATIC:
		{
			newEntity->physics->state |= CollisionState::IN_REST;
			// If static, set NULL inertia matrices.
			newEntity->physics->inertiaTensorInverted = Matrix3f(0,0,0,0,0,0,0,0,0);
			newEntity->physics->inverseMass = 0.0f;
			break;
		}
	}
	switch(pp->type)
	{
		case PhysicsType::DYNAMIC:
		case PhysicsType::KINEMATIC:
			if (pp->fullyDynamic) 
				fullyDynamicEntities.Add(newEntity);
			else 
				semiDynamicEntities.Add(newEntity); 
	}

	if (pp->useForces)
		forceBasedEntities.Add(newEntity);
	
	Physics.EnsurePhysicsMeshIfNeeded(newEntity);

	entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	physicalEntitiesNum = physicalEntities.Size();
//	aabbSweeperNodes = aabbSweeper->Nodes();
	collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collisionsEnabled){
            ++collidingEntities;
        }
    }


	/// Recalculate AABB/OBB-data.
	if (newEntity->model)
	{
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
int PhysicsManager::RegisterEntities(List< std::shared_ptr<Entity> > & targetEntities)
{
	int failedRegistrations = 0;
	for (int i = 0; i < targetEntities.Size(); ++i){
		if (RegisterEntity(targetEntities[i]) != 0)
			++failedRegistrations;
	}
	return failedRegistrations;
}


/// Unregisters an Entity from the physics calculations. Returns 0 if it found the Entity and successfully removed it, 1 if not.
int PhysicsManager::UnregisterEntity(EntitySharedPtr entityToRemove)
{
	LogPhysics("Unregistering entity " + entityToRemove->name + " from physics", INFO);

	/// If have kids/parent, unbind. -> Why? What if it is just a unregistration for re-registering?
	if (entityToRemove->flaggedForDeletion)
		entityToRemove->RemoveLinks();
//	LogPhysics("Unregistering entity for physics "+entityToRemove->name, DEBUG);
	int found = 0;
	int entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	int physicalEntitiesNum = physicalEntities.Size();
//	int aabbSweeperNodes = aabbSweeper->Nodes();
	assert(entitiesInOctree <= physicalEntitiesNum);
	int collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i){
        if (physicalEntities[i]->physics->collisionsEnabled){
            ++collidingEntities;
        }
    }

	// Remove from physical entities list
	bool removedResult = physicalEntities.Remove(entityToRemove) ;
	if (!removedResult)
		return 1;
	assert(removedResult && "Trying to unregister entity that has not been previously registered!");

 //   std::cout<<"\nCollision enabled for entity? "<<entityToRemove->physics->collisionsEnabled;

	// pp o.o
	PhysicsProperty * pp = entityToRemove->physics;

	if (pp->useForces)
		forceBasedEntities.Remove(entityToRemove);


	/// Remove from octree/AABB-sweeper
	if (entityToRemove->physics->collisionsEnabled)
	{
		if (checkType == OCTREE)
			removedResult = entityCollisionOctree->RemoveEntity(entityToRemove);
		else if (checkType == AABB_SWEEP)
			aabbSweeper->UnregisterEntity(entityToRemove);
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
	switch(pp->type)
	{
		case PhysicsType::DYNAMIC:
		case PhysicsType::KINEMATIC:
			if (pp->fullyDynamic) 
				fullyDynamicEntities.Remove(entityToRemove);
			else 
				semiDynamicEntities.Remove(entityToRemove); 
	}

	/// Mark as not registerd for physics anymore.
	entityToRemove->registeredForPhysics = false;

	entitiesInOctree = entityCollisionOctree->RegisteredEntities();
	physicalEntitiesNum = physicalEntities.Size();
	collidingEntities = 0;
	for (int i = 0; i < physicalEntities.Size(); ++i)
	{
        if (physicalEntities[i]->physics->collisionsEnabled){
            ++collidingEntities;
        }
    }
	if (entityToRemove->pathfindingProperty)
	{
		delete entityToRemove->pathfindingProperty;
		entityToRemove->pathfindingProperty = NULL;
	}
	entityToRemove->registeredForPhysics = false;
	return 0;
}

/** Unregisters a selection of entities from physics calculations.
	Returns 0 upon success or a positive number equal to the amount of entities that it failed to unregister.
*/
int PhysicsManager::UnregisterEntities(List< std::shared_ptr<Entity> > & targetEntities){
	int failedUnregistrations = 0;
	for (int i = 0; i < targetEntities.Size(); ++i){
		if (UnregisterEntity(targetEntities[i]) != 0)
			++failedUnregistrations;
	}
	return failedUnregistrations;
}

/// Unregisters all entities from physics calculations, and clears the collission entityCollisionOctree as well.
int PhysicsManager::UnregisterAllEntities()
{
	for (int i = 0; i < physicalEntities.Size(); ++i)
	{
		physicalEntities[i]->registeredForPhysics = false;
	}
	physicalEntities.Clear();
	dynamicEntities.Clear();
	kinematicEntities.Clear();
	fullyDynamicEntities.Clear();
	semiDynamicEntities.Clear();
	forceBasedEntities.Clear();
	if (aabbSweeper)
		aabbSweeper->Clear();
	if (entityCollisionOctree)
		entityCollisionOctree->clearAll();

	return 0;
}
