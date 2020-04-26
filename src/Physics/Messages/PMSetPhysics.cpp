
#include "PhysicsMessage.h"
#include "../PhysicsManager.h"

PMSetPhysicsType::PMSetPhysicsType(List< std::shared_ptr<Entity> > targetEntities, int i_physicsType): PhysicsMessage(PM_SET_PHYSICS_TYPE){
	entities = targetEntities;
	physicsType = i_physicsType;
}

void PMSetPhysicsType::Process(){
	Physics.SetPhysicsType(entities, physicsType);
}

PMSetPhysicsShape::PMSetPhysicsShape(List< std::shared_ptr<Entity> > targetEntities, int i_physicsShape): PhysicsMessage(PM_SET_PHYSICS_SHAPE){
	entities = targetEntities;
	physicsShape = i_physicsShape;
}

void PMSetPhysicsShape::Process(){
	Physics.SetPhysicsShape(entities, physicsShape);
}
