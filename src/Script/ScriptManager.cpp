// Emil Hedemalm
// 2013-07-17

#include "Script.h"
#include "ScriptManager.h"
#include "Input/InputManager.h"

ScriptManager::ScriptManager(){
	// Wosh.
}
ScriptManager::~ScriptManager(){
	// Do nothing with the lists by default!
	for (int i = 0; i < activeEvents.Size(); ++i){
		Script * e = activeEvents[i];
		if (e->flags & DELETE_WHEN_ENDED)
			delete e;
	}
}
ScriptManager * ScriptManager::eventManager = NULL;
ScriptManager * ScriptManager::Instance(){
	assert(eventManager);
	return eventManager;
}
void ScriptManager::Allocate(){
	assert(eventManager == NULL);
	eventManager = new ScriptManager();
}
void ScriptManager::Deallocate(){
	assert(eventManager);
	delete eventManager;
	eventManager = NULL;
}
/** Triggers OnEnter of the event as registers it for processing which is called once per frame via the StateProcessor!
	DO NOTE: that the event manager is NOT in any way responsible for disposing of events, as these will probably best be controlled 
	via the entities, maps or state machines that use them!
	- Alternatively a boolean could be set which toggles this behaviour, but plan ahead and make sure that whatever you do doesn't leak memory!
*/
void ScriptManager::PlayEvent(Script * script)
{
	// If needed, load it first.
	if (!script->loaded)
		script->Load();
	script->OnBegin();
	activeEvents.Add(script);
}
void ScriptManager::Process(float timeInSeconds){
//	std::cout<<"\nEventManager::Process";
	for (int i = 0; i < activeEvents.Size(); ++i){
	//	std::cout<<"\nActive events: "<<activeEvents.Size();
		Script * ev = activeEvents[i];
		ev->Process(timeInSeconds);
		if (ev->state == Script::ENDING){
			ev->OnEnd();
			activeEvents.Remove(ev);
			/// Delete if specified.
			if (ev->flags & DELETE_WHEN_ENDED)
				delete ev;
			/// Step down i since we will remove this element, or one event will be skipped!
			--i;
		}
		if (activeEvents.Size() == 0){
			Input.NavigateUI(false);
		}
		else
			/// Most events assume UI-interaction, yes?
			Input.NavigateUI(true);
	}
}
