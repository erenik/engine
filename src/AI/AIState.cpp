/// Emil Hedemalm
/// 2013-07-28


#include "AIState.h"

AIState::AIState(int stateType, EntitySharedPtr owner){ 
	this->stateID = stateType; 
	entity = owner;
}

/// Callback function for when paths are invalidated.
void AIState::PathsInvalidated(){
	std::cout<<"\nAIState::PathsInvalidated called.";
}
