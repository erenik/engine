/// Emil Hedemalm
/// 2013-02-08


#include "StateProperty.h"
#include "EntityState.h"
#include "../Entity/Entity.h"
#include <cassert>

StateProperty::StateProperty(Entity * entity)
: entity(entity){
	currentState = NULL;
	previousState = NULL;
	queuedState = NULL;
	globalState = NULL;
};

StateProperty::~StateProperty()
{
	if (currentState) delete currentState;
	if (previousState) delete previousState;
	if (queuedState) delete queuedState;
	if (globalState) delete globalState;
};

/// Sets global entity state!
void StateProperty::SetGlobalState(EntityState * newState){
	assert(newState && "newState not allocated in StateProperty::EnterState");
	// Save old state
	if (globalState){
		globalState->OnExit();
		delete globalState;
	}
	globalState = newState;
	globalState->OnEnter();
}


/// Swaps state straight away, keeping the queued state.
void StateProperty::EnterState(EntityState * newState){
	assert(newState && "newState not allocated in StateProperty::EnterState");
	// Save old state
	if (previousState)
		delete previousState;
	previousState = currentState;
	if (currentState){
		currentState->OnExit();
	}
	currentState = newState;
	newState->OnEnter();
}

/// Enters queued state. Returns false if no state was queued.
bool StateProperty::EnterQueuedState(){
	if (queuedState == NULL)
		return false;
	// Re-point stuff
	if (previousState)
		delete previousState;
	previousState = currentState;
	currentState = queuedState;
	queuedState = NULL;
	// Process onExit/onEnter
	previousState->OnExit();
	currentState->OnEnter();
	return true;
}

/// Enters the previous state again.
void StateProperty::RevertToPreviousState(){
	currentState->OnExit();
	EntityState * tmp = currentState;
	currentState = previousState;
	previousState = tmp;
	currentState->OnEnter();
};

/// Callback for when one or more paths have been invalidated to due changes in the map.
void StateProperty::PathsInvalidated(){
	if (globalState){
		globalState->PathsInvalidated();
	}
}

/// Bleh o-o
void StateProperty::Process(float timePassed){
	if (globalState)
		globalState->Process(timePassed);
	if (currentState)
		currentState->Process(timePassed);
}


/// Sent to both global and current state
void StateProperty::ProcessMessage(Message * message){
	if (globalState)
		globalState->ProcessMessage(message);
	if (currentState)
		currentState->ProcessMessage(message);
}