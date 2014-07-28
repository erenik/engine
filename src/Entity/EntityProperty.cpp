/// Emil Hedemalm
/// 2013-02-08


#include "EntityProperty.h"
#include "EntityPropertyState.h"
#include "../Entity/Entity.h"
#include <cassert>

EntityProperty::EntityProperty(String name, int id, Entity * owner)
: name(name), owner(owner), id(id)
	
{
	currentState = NULL;
	previousState = NULL;
	queuedState = NULL;
	globalState = NULL;
};

EntityProperty::~EntityProperty()
{
	if (currentState) delete currentState;
	if (previousState) delete previousState;
	if (queuedState) delete queuedState;
	if (globalState) delete globalState;
};

/// Sets global entity state!
void EntityProperty::SetGlobalState(EntityPropertyState * newState){
	assert(newState && "newState not allocated in EntityProperty::EnterState");
	// Save old state
	if (globalState){
		globalState->OnExit();
		delete globalState;
	}
	globalState = newState;
	globalState->OnEnter();
}


/// Swaps state straight away, keeping the queued state.
void EntityProperty::EnterState(EntityPropertyState * newState){
	assert(newState && "newState not allocated in EntityProperty::EnterState");
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
bool EntityProperty::EnterQueuedState(){
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
void EntityProperty::RevertToPreviousState(){
	currentState->OnExit();
	EntityPropertyState * tmp = currentState;
	currentState = previousState;
	previousState = tmp;
	currentState->OnEnter();
};

/// If reacting to collisions...
void EntityProperty::OnCollision(Collision & data)
{
}

/// Returns the ID of this specific property (used when identifying it within an entity later on).
int EntityProperty::GetID()
{
	return id;
}

/// Callback for when one or more paths have been invalidated to due changes in the map.
void EntityProperty::PathsInvalidated(){
	if (globalState){
		globalState->PathsInvalidated();
	}
}

/// Bleh o-o
void EntityProperty::Process(int timeInMs)
{
	if (globalState)
		globalState->Process(timeInMs);
	if (currentState)
		currentState->Process(timeInMs);
}


/// Sent to both global and current state
void EntityProperty::ProcessMessage(Message * message){
	if (globalState)
		globalState->ProcessMessage(message);
	if (currentState)
		currentState->ProcessMessage(message);
}