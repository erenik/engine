// Emil Hedemalm
// 2013-03-17

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "Globals.h"
#include "GameStates/GameState.h"
#include "GameStates/GameStates.h"
#include <Util.h>

class GameState;

#define StateMan		(*StateManager::Instance())

/// External function where the games inquestion register what game states they want available.
void RegisterStates();

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
	void Pause();
	void Resume();

	/// Tries to queue previous state.
	void QueuePreviousState();
	/// Performs switch operations when transitioning states.
	void QueueState(int id);

	/// Sets a global state, that can process global messages and packets
	void SetGlobalState(int id);
	/// Returns state of given ID so that messages may be sent to it before entering it.
	GameState * GetState(int id);
	GameState * GetStateByName(String name);

	/// Returns ID of the global state
	int GlobalID();
	/// Returns ID of the active StateMan.
	int ActiveStateID();
	/// Returns a pointer to the global state
	GameState * GlobalState() { return globalState; };
	/// Returns a pointer to the active StateMan.
	GameState * ActiveState() { return activeState; };
	GameState * PreviousState() { return previousState; };

	// For handling drag-n-drog files.
	void HandleDADFiles(List<String> & files);

	/// If the thread should keep on processing.
	bool shouldLive;
private:
	bool paused;
	/// Enteres queued state (if any)
	void EnterQueuedState();

	/// Registers a state to the state manager for use. Returns true upon success.
	bool RegisterState(GameState * state);

	/// Array with all states
	List<GameState*> stateList;

	/// Currently active global state
	GameState * globalState;
	/// Currently active state.
	GameState * activeState;
	GameState * previousState;
	Queue<GameState *> queuedStates;
};


#endif
