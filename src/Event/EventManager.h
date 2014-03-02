// Emil Hedemalm
// 2013-07-17

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#define EventMan (*EventManager::Instance())
#define BeginEvent	PlayEvent

#include <List/List.h>

class Event;

/** A class to handle any events, whether it be for playing music, 
	initiating a cut-scene at a certain point or a trigger for a trap.
*/
class EventManager {
private:
	EventManager();
	~EventManager();
	static EventManager * eventManager;
public:
	static EventManager * Instance();
	static void Allocate();
	static void Deallocate();
	/** Triggers OnEnter of the event and registers it for processing which is called once per frame via the StateProcessor!
		DO NOTE: that the event manager is NOT in any way responsible for disposing of events, as these will probably best be controlled 
		via the entities, maps or state machines that use them!
		- Alternatively a boolean could be set which toggles this behaviour, but plan ahead and make sure that whatever you do doesn't leak memory!
	*/
	void PlayEvent(Event * event);
	void Process(float timeInSeconds);
	List<Event*> GetActiveEvents(){ return activeEvents; };
private:
	List<Event*> activeEvents;
	List<Event*> finishedEvents;
};

#endif
