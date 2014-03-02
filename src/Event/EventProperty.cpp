// Emil Hedemalm
// 2013-07-17

#include "EventProperty.h"
#include "Globals.h"

EventProperty::EventProperty(){
	onInteract = onApproach = onLoadMap = onEnterInside = onFrameInside = onLeaveInside = NULL;
}
EventProperty::~EventProperty(){
	/// Should probably deallocate the events..! Or?
	SAFE_DELETE(onInteract);
	SAFE_DELETE(onApproach);
	SAFE_DELETE(onLoadMap);

	SAFE_DELETE(onEnterInside);
	SAFE_DELETE(onFrameInside);
	SAFE_DELETE(onLeaveInside);

};

/*
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
*/