// Emil Hedemalm
// 2013-07-17

#ifndef EVENT_PROPERTY_H
#define EVENT_PROPERTY_H

#include "Script.h"

class ScriptProperty {
public:
	ScriptProperty();
	~ScriptProperty();

	/// Will be the save/load and reference stuffs.
	List<String> eventSources;

	/// For action/RPG/etc. games, action-reaction..!
	Script * onInteract;
	/// Upon grazing or attempting to get near the entity. Further defined by each game how it's used!
	Script * onApproach;
	/// Upon loading the map.
	Script * onLoadMap; 

	/// Physics-based events..! For event-entities.
	Script * onEnterInside;
	Script * onFrameInside; /// <-- Should contain a flag for if the event repeats while inside or is jsut triggered once.
	Script * onLeaveInside;

private:
};

#endif
