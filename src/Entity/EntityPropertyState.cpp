/// Emil Hedemalm
/// 2013-02-08

#include "EntityPropertyState.h"

EntityPropertyState::EntityPropertyState(int stateType, EntitySharedPtr owner)
{ 
	this->stateID = stateType; 
	entity = owner;
}

EntityPropertyState::~EntityPropertyState()
{
}

/// Callback function for when paths are invalidated.
void EntityPropertyState::PathsInvalidated(){
	std::cout<<"\nEntityPropertyState::PathsInvalidated called.";
}

