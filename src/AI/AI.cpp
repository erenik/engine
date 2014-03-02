// Emil Hedemalm
// 2013-07-28

#include "AI.h"
#include <cstdlib>
#include "Globals.h"

#include "AIState.h"

AI::AI(int type)
: type(type){
	globalState = previousState = currentState = NULL;
	enabled = true;
}
/// Virtual destructor so subclass destructor callback works correctly! IMPORTANT!
AI::~AI(){
	SAFE_DELETE(globalState);
	SAFE_DELETE(previousState);
	SAFE_DELETE(currentState);
}
void AI::OnEnter(){
	std::cout<<"\nDefault AI::OnEnter called.";
}
void AI::Process(float timeInSeconds){
	std::cout<<"\nDefault AI::Process called.";
}
void AI::OnExit(){
	std::cout<<"\nDefault AI::OnExit called.";
}


/// Queues a new current state.
void AI::QueueState(AIState * newState){
	assert(false && "Implement");
}
/// Queues a new global state.
void AI::QueueGlobalState(AIState * newState){
	assert(false && "Implement");
}

// WOshi.
void AI::ProcessMessage(Message * message){
	std::cout<<"\nDefault AI::ProcessMessage called.";
}