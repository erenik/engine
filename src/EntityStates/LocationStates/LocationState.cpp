
#include "LocationState.h"

LocationState::LocationState(Entity * entity)
: EntityState(EntityStateType::LOCATION, entity){
	location = NULL;
};
LocationState::~LocationState(){
	if (location)
		delete location;
};

/// Function when entering this state, providing a pointer to the previous StateMan.
void LocationState::OnEnter(Entity * entity){

}
/// Main processing function
void LocationState::Process(Entity * entity, int timePassed){

}
/// Function when leaving this state, providing a pointer to the next StateMan.
void LocationState::OnExit(Entity * entity){

}
/// Function for handling messages sent to the entity.
void LocationState::ProcessMessage(Message * message){

}
