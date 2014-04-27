// Emil Hedemalm
// 2013-07-17

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#define ScriptMan (*ScriptManager::Instance())
#define BeginEvent	PlayScript
#define PlayEvent	PlayScript

#include <List/List.h>

class Script;

/** A class to handle any events, whether it be for playing music, 
	initiating a cut-scene at a certain point or a trigger for a trap.
*/
class ScriptManager {
private:
	ScriptManager();
	~ScriptManager();
	static ScriptManager * eventManager;
public:
	static ScriptManager * Instance();
	static void Allocate();
	static void Deallocate();
	/** Triggers OnEnter of the event and registers it for processing which is called once per frame via the StateProcessor!
		DO NOTE: that the event manager is NOT in any way responsible for disposing of events, as these will probably best be controlled 
		via the entities, maps or state machines that use them!
		- Alternatively a boolean could be set which toggles this behaviour, but plan ahead and make sure that whatever you do doesn't leak memory!
	*/
	void PlayScript(Script * script);
	void Process(long long timeInSeconds);
	List<Script*> GetActiveEvents(){ return activeEvents; };
private:
	List<Script*> activeEvents;
	List<Script*> finishedEvents;
};

#endif
