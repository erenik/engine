
#ifndef LOCATION_STATE_H
#define LOCATION_STATE_H

#include "../AIState.h"

class LocationState : AIState {
public:
	LocationState();
	~LocationState(){};

	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(EntitySharedPtr entity);
	/// Main processing function
	void Process(EntitySharedPtr entity);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(EntitySharedPtr entity);

	/// Function for handling messages sent to the entity.
	void onMessage(AIMessage * message);
private:
	/// Flag if it's currently enabled for interaction.
	bool enabled;
}

#endif
