// Emil Hedemalm
// 2013-07-17

#ifndef EVENT_PROPERTY_H
#define EVENT_PROPERTY_H

#include "Event.h"

class EventProperty {
public:
	EventProperty();
	~EventProperty();

	/// Will be the save/load and reference stuffs.
	List<String> eventSources;

	/// For action/RPG/etc. games, action-reaction..!
	Event * onInteract;
	/// Upon grazing or attempting to get near the entity. Further defined by each game how it's used!
	Event * onApproach;
	/// Upon loading the map.
	Event * onLoadMap; 

	/// Physics-based events..! For event-entities.
	Event * onEnterInside;
	Event * onFrameInside; /// <-- Should contain a flag for if the event repeats while inside or is jsut triggered once.
	Event * onLeaveInside;

private:
};

#endif
