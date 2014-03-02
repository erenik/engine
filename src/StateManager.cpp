/// Emil Hedemalm
/// 2013-03-07

#include "StateManager.h"
#include "Message/MessageManager.h"
//// Don't include all managers. Ever.
#include "Input/InputManager.h"
#include "Game/GameType.h"
#include "Maps/MapManager.h"
#include "OS/Sleep.h"
#include "Event/EventManager.h"
#include "Network/NetworkManager.h"
#include "OS/WindowManager.h"
#include "Audio/AudioManager.h"

/// Global statemanager.
StateManager * StateManager::stateMan = NULL;

/// Thread for the state processor
extern uintptr_t stateProcessingThread;

void StateManager::Allocate(){
	assert(stateMan == NULL);
	stateMan = new StateManager();
}
StateManager * StateManager::Instance(){
	assert(stateMan && "StateManager not initialized");
	return stateMan;
}
void StateManager::Deallocate(){
	assert(stateMan);
	delete(stateMan);
	stateMan = NULL;
}

StateManager::StateManager(){
	globalState = NULL;
	activeState = NULL;
	previousState = NULL;
	shouldLive = true;
	paused = false;
};

StateManager::~StateManager(){
	if (globalState)
		globalState->OnExit(NULL);
	if (activeState)
		activeState->OnExit(NULL);
	while (stateList.Size()){
        GameState * gs = stateList[0];
        std::cout<<"\nDeleting game state: "<<gs->stateName;
        stateList.Remove(gs);
		delete gs;
	}
};

void StateManager::Initialize(){
	/// Create and queue the init-state right away!
	GameState * initState = new Initialization();
	RegisterState(initState);
	QueueState(initState->GetID());
	RegisterStates();
	RegisterState(new Exit());
};

/// Queries all states to create their own input-bindings.
void StateManager::CreateDefaultInputBindings(){
	for (int i = 0; i < stateList.Size(); ++i){
		stateList[i]->CreateDefaultBindings();
	}
}

/// Queries all states to create their own user interfaces.
void StateManager::CreateUserInterfaces(){
    for (int i = 0; i < stateList.Size(); ++i){
        GameState * gs = stateList[i];
        assert(stateList[i]);
		if (gs->ui)
			delete gs->ui;
		gs->ui = NULL;
		std::cout<<"\nCreating user interface for "<<gs->stateName;
        stateList[i]->CreateUserInterface();
	}
}
/// Queries all states to deallocate their UIs
void StateManager::DeallocateUserInterfaces(){
	for (int i = 0; i < stateList.Size(); ++i){
        GameState * gs = stateList[i];
        assert(stateList[i]);
	//	std::cout<<"\nDeallocating user interface for "<<gs->stateName;
        stateList[i]->DeallocateUserInterface();
	}
}

bool StateManager::RegisterState(GameState * i_state){
	stateList.Add(i_state);
	assert(stateList.Size() < MAX_GAME_STATES);
	return false;
};

/// Tries to queue previous state.
void StateManager::QueuePreviousState(){
	QueueState(previousState->id);
}

void StateManager::QueueState(int id){
	for (int i = 0; i < stateList.Size(); ++i){
		if (stateList[i]->id == id){
			queuedStates.Push(stateList[i]);
			return;
		}
	}
	std::cout<<"\nERROR: Trying to queue an unknown/non-existant game state.";
}

/// Enteres queued state (if any)
void StateManager::EnterQueuedState(){
	if (queuedStates.isOff())
		return;
	GameState * queuedState = queuedStates.Pop();
	if (activeState)				// If we were in a valid state,
		activeState->OnExit(queuedState);	// Call onExit for the previous state
	previousState = activeState;
	queuedState->OnEnter(activeState);	// Then onEnter for the new state
	activeState = queuedState;			// Then reset the active state pointer!
	queuedState = NULL;
	Resume(); // By default, unpause if paused.
	return;							// And break the loop ^^
}

void StateManager::SetGlobalState(int id){
	GameState *newGlobalState = NULL;
	// Try to find the requested state
	for (int i = 0; i < stateList.Size(); ++i){
		if (stateList[i]->id == id){
			newGlobalState = stateList[i];
			break;
		}
	}
	if(globalState)
		globalState->OnExit(newGlobalState);
	if (newGlobalState)
		newGlobalState->OnEnter(globalState);
	globalState = newGlobalState;
}

GameState * StateManager::GetState(int id){
	for (int i = 0; i < stateList.Size(); ++i){
		if (stateList[i]->id == id)
			return stateList[i];
	}
	return NULL;
}

GameState * StateManager::GetStateByName(String name){
	for (int i = 0; i < stateList.Size(); ++i)
		if (stateList[i]->stateName == name)
			return stateList[i];
	return NULL;
}

/// Returns ID of the global StateMan.
int StateManager::GlobalID(){
	if (globalState)
		return globalState->id;
	return 0;
}
/// Returns ID of the active StateMan.
int StateManager::ActiveStateID(){
	if (activeState)
		return activeState->id;
	return 0;
}

// For handling drag-n-drog files.
void StateManager::HandleDADFiles(List<String> & files){
	if (globalState)
		globalState->HandleDADFiles(files);
	if (activeState)
		activeState->HandleDADFiles(files);
}

#include <ctime>

#ifdef WINDOWS
#include <process.h>
#endif

/// Thread function for processing the active state, which might calculate events, timers, AI and all game objects.
#ifdef WINDOWS
    void StateManager::StateProcessor(void * vArgs){
#elif defined LINUX | defined OSX
	void * StateManager::StateProcessor(void * vArgs){
#endif
    std::cout<<"\n===========================================______________________-----";
    std::cout<<"\nSTATE_PROCESSOR_OF_DOOM_STARTHED";
    std::cout<<"\n===========================================______________________-----";
	long long time = Timer::GetCurrentTimeMs();
	long long newTime = Timer::GetCurrentTimeMs();
	while(StateMan.shouldLive){
		/// Update time
		time = newTime;
		newTime = Timer::GetCurrentTimeMs();
		long timeDiff = newTime - time;
		float timeDiffF = ((float)timeDiff) * 0.001f;
		/// Enter new state if queued.
		StateMan.EnterQueuedState();
		EventMan.Process(timeDiffF);
		/// Wosh.
		if (!StateMan.IsPaused()){
			/// Process the active StateMan.
			if (StateMan.GlobalState())
                StateMan.GlobalState()->Process(timeDiffF);
			if (StateMan.ActiveState())
				StateMan.ActiveState()->Process(timeDiffF);
			if (MapMan.ActiveMap())
				MapMan.ActiveMap()->Process(timeDiffF);
		}
		/// If not in any state, sleep a bit, yo.
		else
			Sleep(5);
		/// If not in focus, sleep moar!
	//	if (!WindowMan.InFocus())
	//		Sleep(5);

		// Process audio
		AudioMan.Update();
		/// Process network, sending packets and receiving packets
		NetworkMan.ProcessNetwork();

		/// Process messages we've received.
		MesMan.ProcessMessages();
		/// Process network packets if applicable
		MesMan.ProcessPackets();
		/// Sleep a bit, always :P // Do in specific states, not here.
		// Sleep(10);

		/// Get input from XBox devices if possible
		Input.UpdateDeviceStates();
	}
	std::cout<<"\n>>> StateProcessingThread ending...";
#ifdef WINDOWS
	stateProcessingThread = NULL;
	_endthread();
#elif defined LINUX | defined OSX
    stateProcessingThread = NULL;
#endif

}


/** For pausing entity-state and game state updates while paused.
	Will NOT affect input, audio or network updates as these are vital.
*/
bool StateManager::IsPaused(){
	return paused;
}
void StateManager::Pause(){
	paused = true;
}
void StateManager::Resume(){
	paused = false;
}
