/// Emil Hedemalm
/// 2013-02-08

#ifndef LOCATION_STATE_H
#define LOCATION_STATE_H

#include "EntityStates/EntityState.h"
#include "../Locations/Location.h"

class LocationState : public EntityState {
public:
	LocationState(Entity * owner);
	~LocationState();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(Entity * entity);
	/// Main processing function
	void Process(Entity * entity, int timePassed);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(Entity * entity);

	/// Function for handling messages sent to the entity.
	virtual void ProcessMessage(Message * message);

	/// Pointer to the location data, detailed in Location.h
	Location * location;
	/// Flag if it's currently enabled for interaction.
	bool enabled;
};

#endif
