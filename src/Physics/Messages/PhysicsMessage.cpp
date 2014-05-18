/// Emil Hedemalm
/// 2013-10-27
/// General physics messages here, move them elsewhere if they are numerous can categorizable.

#include "PhysicsMessage.h"
#include "../PhysicsManager.h"
#include "Physics/Springs/Spring.h"

PhysicsMessage::PhysicsMessage(){
	type = PM_NULL;
}

PhysicsMessage::PhysicsMessage(int messageType){
	type = messageType;
}

PhysicsMessage::~PhysicsMessage(){
}

void PhysicsMessage::Process(){
	switch(type){
		case PM_RECALCULATE_PHYSICS_PROPERTIES: 
			Physics.RecalculatePhysicsProperties();
			break;
		/// For cleaing all, and also if the user sent an Unregister_Entities without it's own custom message .. ^^;;;
		case PM_CLEAR_ALL_ENTITIES: case PM_UNREGISTER_ENTITIES:
			Physics.UnregisterAllEntities();
			break;
		case PM_IGNORE_COLLISSIONS:
			Physics.ignoreCollissions = true;
			break;
		case PM_RESET_SETTINGS:
			Physics.gravitation = Vector3f(0, -DEFAULT_GRAVITY, 0);
			Physics.simulationSpeed = 1.0f;
			Physics.ignoreCollissions = false;
			break;
	}
}


/// Creates a default spring between the entities in the list (linearly attached).
PMCreateSpring::PMCreateSpring(List<Entity*> targetEntities, float springConstant = 1.0f)
: PhysicsMessage(PM_CREATE_SPRING), entities(targetEntities), springConstant(springConstant), springLength(-1.0f){

}
/// Creates a default spring between the entities in the list (linearly attached).
PMCreateSpring::PMCreateSpring(List<Entity*> targetEntities, float springConstant, float springLength)
: PhysicsMessage(PM_CREATE_SPRING), entities(targetEntities), springConstant(springConstant), springLength(springLength)
{
};

#include "Physics/PhysicsProperty.h"
void PMCreateSpring::Process() {
	for (int i = 0; i < entities.Size()-1; ++i){
		Entity * one = entities[i], * two = entities[i+1];
		Spring * spring = new Spring(one, two);
		spring->springConstant = springConstant;
		spring->equilibriumLength = springLength;
		if (spring->equilibriumLength < 0)
			spring->equilibriumLength = (one->position - two->position).Length();
		one->physics->springs.Add(spring);
		two->physics->springs.Add(spring);
		Physics.springs.Add(spring);
	}		
};

