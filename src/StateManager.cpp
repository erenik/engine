/// Emil Hedemalm
/// 2013-03-07

#include "StateManager.h"
#include "Message/MessageManager.h"
//// Don't include all managers. Ever.
#include "Input/InputManager.h"
#include "Maps/MapManager.h"
#include "OS/Sleep.h"
#include "Script/ScriptManager.h"
#include "Network/NetworkManager.h"
#include "Window/WindowManager.h"
#include "Entity/EntityManager.h"
#include "Graphics/GraphicsManager.h"
#include "Physics/PhysicsManager.h"

#include "File/LogFile.h"

/// Global statemanager.
StateManager * StateManager::stateMan = NULL;

/// Thread for the state processor
extern uintptr_t stateProcessingThread;

void StateManager::Allocate()
{
	assert(stateMan == NULL);
	stateMan = new StateManager();
}
StateManager * StateManager::Instance(){
	assert(stateMan && "StateManager not initialized");
	return stateMan;
}
void StateManager::Deallocate()
{
	assert(stateMan);
	delete(stateMan);
	stateMan = NULL;
}

StateManager::StateManager()
{
	globalState = NULL;
	activeState = NULL;
	previousState = NULL;
	queuedGlobalState = NULL;
	shouldLive = true;
	paused = false;
};

StateManager::~StateManager()
{
	DeleteStates();
};

/// Deletes all application states (global and current/active ones).
void StateManager::DeleteStates()
{
	previousState = globalState = activeState = NULL;
	this->stateList.ClearAndDelete();
}

void StateManager::Initialize()
{
	/// Create and queue the init-state right away!
	AppState * initState = new Initialization();
	RegisterState(initState);
	QueueState(initState);
	RegisterStates();
	RegisterState(new Exit());


	// o.o
	stateProcessingMutex.Create("StateProcessingMutex");
};

/// Queries all states to create their own input-bindings.
void StateManager::CreateDefaultInputBindings(){
	for (int i = 0; i < stateList.Size(); ++i){
		stateList[i]->CreateDefaultBindings();
	}
}

/// Queries all states to create their own user interfaces.
void StateManager::CreateUserInterfaces()
{
	assert(false);
    for (int i = 0; i < stateList.Size(); ++i){
        AppState * gs = stateList[i];
        assert(stateList[i]);
		if (gs->ui)
			delete gs->ui;
		gs->ui = NULL;
		std::cout<<"\nCreating user interface for "<<gs->name;
        stateList[i]->CreateUserInterface();
	}
}
/// Queries all states to deallocate their UIs
void StateManager::DeallocateUserInterfaces(){
	for (int i = 0; i < stateList.Size(); ++i){
        AppState * gs = stateList[i];
        assert(stateList[i]);
	//	std::cout<<"\nDeallocating user interface for "<<gs->name;
        stateList[i]->DeallocateUserInterface();
	}
}

bool StateManager::RegisterState(AppState * i_state){
	stateList.Add(i_state);
	assert(stateList.Size() < GameStateID::MAX_GAME_STATES);
	return false;
};

/// Tries to queue previous state.
void StateManager::QueuePreviousState(){
	QueueState(previousState);
}

void StateManager::QueueState(AppState * gs)
{
	if (!gs)
	{
		std::cout<<"Trying to queue null-state.";
		return;
	}
	queuedStates.Push(gs);
}

/// Performs switch operations when transitioning states.
void StateManager::QueueGlobalState(AppState * state)
{	
	if (!state)
		return;
	queuedGlobalState = state;
}


/// Enteres queued state (if any)
void StateManager::EnterQueuedState()
{
	if (queuedStates.isOff())
		return;
	AppState * queuedState = queuedStates.Pop();
	if (activeState)				// If we were in a valid state,
		activeState->OnExit(queuedState);	// Call onExit for the previous state
	previousState = activeState;
	queuedState->OnEnter(activeState);	// Then onEnter for the new state
	activeState = queuedState;			// Then reset the active state pointer!
	queuedState = NULL;
	return;							// And break the loop ^^
}

void StateManager::EnterQueuedGlobalState()
{
	if (!queuedGlobalState)
		return;
	if (globalState)				// If we were in a valid state,
		globalState->OnExit(queuedGlobalState);	// Call onExit for the previous state
	queuedGlobalState->OnEnter(globalState);	// Then onEnter for the new state
	globalState = queuedGlobalState;			// Then reset the active state pointer!
	queuedGlobalState = NULL;
	return;							// And break the loop ^^

}


void StateManager::SetGlobalState(int id)
{
	AppState *newGlobalState = NULL;
	// Try to find the requested state
	for (int i = 0; i < stateList.Size(); ++i){
		if (stateList[i]->id == id){
			newGlobalState = stateList[i];
			break;
		}
	}
	SetGlobalState(newGlobalState);
}

/// Sets a global state, that can process global messages and packets
void StateManager::SetGlobalState(AppState * newGlobalState)
{
	if(globalState)
		globalState->OnExit(newGlobalState);
	if (newGlobalState)
		newGlobalState->OnEnter(globalState);
	globalState = newGlobalState;
}

/// Sets active/current state.
void StateManager::SetActiveState(AppState * state)
{
	if (activeState)
		activeState->OnExit(state);
	if (state)
		state->OnEnter(activeState);
	previousState = activeState;
	activeState = state;
}

AppState * StateManager::GetStateByID(int id){
	for (int i = 0; i < stateList.Size(); ++i){
		if (stateList[i]->id == id)
			return stateList[i];
	}
	return NULL;
}

AppState * StateManager::GetStateByName(String name){
	for (int i = 0; i < stateList.Size(); ++i)
		if (stateList[i]->name == name)
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
#include <Ole2.h>
#endif


/// Signifies that the application is currently exiting.
extern bool quittingApplication;

/// Thread function for processing the active state, which might calculate events, timers, AI and all game objects.
#ifdef WINDOWS
void StateManager::StateProcessor(void * vArgs)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms695279%28v=vs.85%29.aspx
	int result = OleInitialize(NULL);
//	int result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	assert(result == S_OK);

#elif defined LINUX | defined OSX
void * StateManager::StateProcessor(void * vArgs){
#endif
    std::cout<<"\n===========================================______________________-----";
    std::cout<<"\nSTATE_PROCESSOR_OF_DOOM_STARTHED";
    std::cout<<"\n===========================================______________________-----";
	long long time = Timer::GetCurrentTimeMs();
	long long newTime = Timer::GetCurrentTimeMs();
	
	LogMain("State processor starting", INFO);
	while(StateMan.shouldLive)
	{
		// For catching them errors.
		try 
		{
			/// Pause for a bit if the processing mutex is claimed.
			if (StateMan.stateProcessingMutex.Claim(-1))
			{
				/// Update time
				time = newTime;
				newTime = Timer::GetCurrentTimeMs();
				int timeDiffInMs = newTime - time;
				timeDiffInMs %= 200; // Max 200 ms per simulation-iteration?
				float timeDiffF = ((float)timeDiffInMs) * 0.001f;
				/// Enter new state if queued.
				if (!quittingApplication)
				{
					StateMan.EnterQueuedGlobalState();
					StateMan.EnterQueuedState();
					ScriptMan.Process(timeDiffInMs);
					/// Wosh.
					if (!StateMan.IsPaused()){
						/// Process the active StateMan.
						if (StateMan.GlobalState())
							StateMan.GlobalState()->Process(timeDiffInMs);
						if (StateMan.ActiveState())
							StateMan.ActiveState()->Process(timeDiffInMs);
						if (MapMan.ActiveMap())
							MapMan.ActiveMap()->Process(timeDiffInMs);
						// Clean-up.
						EntityMan.DeleteUnusedEntities(timeDiffInMs);
					}
					/// If not in any state, sleep a bit, yo.
					else
						Sleep(5);
					/// If not in focus, sleep moar!
				//	if (!WindowMan.InFocus())
				//		Sleep(5);

					/// Process network, sending packets and receiving packets
					NetworkMan.ProcessNetwork();

					/// Get input from XBox devices if possible
					Input.UpdateDeviceStates();
					// Main message loop for all extra created windows, since they are dependent on the thread they were created in...
		#ifdef WINDOWS
					// Get messages and dispatch them to WndProc
					MSG msg;
					// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644943%28v=vs.85%29.aspx
					while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING)
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					// TODO: Add linux version in an elif for more created windows?
		#endif
					/// Process network packets if applicable
					MesMan.ProcessPackets();
				
				}

				/// Always process messages, even if quitting, as some messages need to be processed here 
				/// (like properly destroying windows)
				MesMan.ProcessMessages();

				// Post messages, if any.
				if (StateMan.graphicsQueue.Size())
					GraphicsMan.QueueMessages(StateMan.graphicsQueue);
				StateMan.graphicsQueue.Clear();
				if (StateMan.physicsQueue.Size())
					PhysicsMan.QueueMessages(StateMan.physicsQueue);
				StateMan.physicsQueue.Clear();

				/// Release mutex at the end of the frame.
				StateMan.stateProcessingMutex.Release();

			}
			else {
				/// Unable to grab mutex? Then sleep for a bit.
				Sleep(40);
			}
		}
		catch (...)
		{
			LogMain("An unexpected error occurred in the main processing thread (StateManager::StateProcessor).", ERROR);
			std::cout<<"An unexpected error occurred.";
			Sleep(100);
		}
	}
	LogMain("State processing thread ending", INFO);

	std::cout<<"\n>>> StateProcessingThread ending...";
#ifdef WINDOWS
	// De-allocate COM stuffs
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms688715%28v=vs.85%29.aspx
	// CoUninitialize();
	OleUninitialize();


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

void StateManager::Pause()
{
	while(!stateProcessingMutex.Claim(-1))
		;
	paused = true;
}

void StateManager::Resume()
{
	paused = false;
	stateProcessingMutex.Release();
}
