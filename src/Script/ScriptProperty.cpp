// Emil Hedemalm
// 2013-07-17

#include "ScriptProperty.h"
#include "Globals.h"

ScriptProperty::ScriptProperty(){
	onInteract = onApproach = onLoadMap = onEnterInside = onFrameInside = onLeaveInside = NULL;
}
ScriptProperty::~ScriptProperty(){
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
Script * onInteract;
/// Upon grazing or attempting to get near the entity. Further defined by each game how it's used!
Script * onApproach;
/// Upon loading the map.
Script * onLoadMap; 

/// Physics-based events..! For event-entities.
Script * onEnterInside;
Script * onFrameInside; /// <-- Should contain a flag for if the event repeats while inside or is jsut triggered once.
Script * onLeaveInside;
*/