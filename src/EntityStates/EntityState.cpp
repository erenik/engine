/// Emil Hedemalm
/// 2013-02-08


#include "EntityState.h"

EntityState::EntityState(int stateType, Entity * owner){ 
	this->stateID = stateType; 
	entity = owner;
	ai = NULL;
}

/// Callback function for when paths are invalidated.
void EntityState::PathsInvalidated(){
	std::cout<<"\nEntityState::PathsInvalidated called.";
}

/// Assigns this entity an own AI which will be deleted when this state is destructed.
void EntityState::AssignAI(AI * newAI){
	assert(ai == NULL);
	assert(newAI != NULL);
	ai = newAI;
}