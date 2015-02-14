// Emil Hedemalm
// 2013-03-17

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "Globals.h"
#include "AppStates/AppState.h"
#include "AppStates/AppStates.h"
#include <Util.h>
#include "Mutex/Mutex.h"

class AppState;
class PhysicsMessage;
class GraphicsMessage;

#define StateMan		(*StateManager::Instance())

/** External function where the games inquestion register what game states they want available.
	Describe how to use it as well..?
	...
	Typical function calls inside would be:

	MyGameState * myState = new MyGameState();
	StateMan.RegisterState(myState);
	StateMan.RegisterState(new MyState2());

	Followed by a call to set which application/game-state to enter first after the game engine is initialized.

	StateMan.QueueState(myState);
	
*/
void RegisterStates();

#define PhysicsQueue (StateMan.physicsQueue)
#define GraphicsQueue (StateMan.graphicsQueue)

/// Handles switches between game states, re-linking of key bindings, etc.
class StateManager{
private:
	StateManager();
	static StateManager * stateMan;
	friend void RegisterStates();
public:
	static void Allocate();
	static StateManager * Instance();
	static void Deallocate();
	~StateManager();


	/// Queues used to queue messages to be sent in batches to the other thread(s)
	List<PhysicsMessage*> physicsQueue;
	List<GraphicsMessage*> graphicsQueue;

	/// Deletes all application states (global and current/active ones).
	void DeleteStates();

	/// Registers states to be used by the state manager.
	void Initialize();
	/// Queries all states to create their own input-bindings.
	void CreateDefaultInputBindings();
	/// Queries all states to create their own user interfaces.
	void CreateUserInterfaces();
	/// Queries all states to deallocate their UIs
	void DeallocateUserInterfaces();

	/// Thread function for processing the active state, which might calculate events, timers, AI and all game objects.
#ifdef WINDOWS
	static void StateProcessor(void *vArgs);
#elif defined LINUX | defined OSX
	static void * StateProcessor(void *vArgs);
#endif
	/// Processes the active StateMan.
	void Process();
	/** For pausing entity-state and game state updates while paused.
		Will NOT affect input, audio or network updates as these are vital.
	*/
	bool IsPaused();

	// Pauses using a mutex.
	void Pause();
	void Resume();

	/// Tries to queue previous state.
	void QueuePreviousState();
	/// Performs switch operations when transitioning states.
	void QueueState(AppState * state);
	/// Performs switch operations when transitioning states.
	void QueueGlobalState(AppState * state);

	/// Sets a global state, that can process global messages and packets
	void SetGlobalState(int id);
	/// Sets a global state, that can process global messages and packets
	void SetGlobalState(AppState * newGlobalState);
	/// Sets active/current state.
	void SetActiveState(AppState * state);


	/// Returns state of given ID so that messages may be sent to it before entering it.
	AppState * GetStateByID(int id);
	AppState * GetStateByName(String name);

	/// Returns ID of the global state
	int GlobalID();
	/// Returns ID of the active StateMan.
	int ActiveStateID();
	/// Returns a pointer to the global state
	AppState * GlobalState() { return globalState; };
	/// Returns a pointer to the active StateMan.
	AppState * ActiveState() { return activeState; };
	AppState * PreviousState() { return previousState; };

	// For handling drag-n-drog files.
	void HandleDADFiles(List<String> & files);

	/// If the thread should keep on processing.
	bool shouldLive;
private:

	/// o-o
	Mutex stateProcessingMutex;

	bool paused;
	/// Enteres queued state (if any)
	void EnterQueuedState();
	void EnterQueuedGlobalState();

	/// Registers a state to the state manager for use. Returns true upon success.
	bool RegisterState(AppState * state);

	/// Array with all states
	List<AppState*> stateList;

	/// Currently active global state
	AppState * globalState;
	/// Currently active state.
	AppState * activeState;
	AppState * previousState;
	AppState * queuedGlobalState;
	Queue<AppState *> queuedStates;
};


#endif
