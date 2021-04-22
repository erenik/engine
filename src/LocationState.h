
#ifndef LOCATION_STATE_H
#define LOCATION_STATE_H

#include "../AIState.h"

class LocationState : AIState {
public:
	LocationState();
	~LocationState(){};

	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(Entity* entity);
	/// Main processing function
	void Process(Entity* entity);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(Entity* entity);

	/// Function for handling messages sent to the entity.
	void onMessage(AIMessage * message);
private:
	/// Flag if it's currently enabled for interaction.
	bool enabled;
}

#endif
