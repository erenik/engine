// Emil Hedemalm
// 2013-03-17

#include "Initializer.h"
#include "Globals.h"
#include "Command/CommandLine.h"
#include "Managers.h" // Don't include all managers. Ever. Except here and in Initializer/Deallocator.
#include "OS/Sleep.h"
#include "Graphics/Messages/GMSet.h"
#include "File/File.h"
#include "Application/Application.h"

#include "OS/Thread.h"

THREAD_HANDLE initializerThread = NULL;
THREAD_HANDLE deallocatorThread = NULL;
THREAD_HANDLE graphicsThread = NULL;
THREAD_HANDLE audioThread = NULL;
THREAD_HANDLE stateProcessingThread = NULL;

int initializers = 0;

// Initialize function to be run with the thread

THREAD_START(Initialize)
{
    initializers++;
    assert(initializers == 1);

	std::cout<<"\nInitializer thread begun...";

	MesMan.QueueMessages("CreateMainWindow");

	// Call Initialize for all managers to properly initialize them
	StateMan.Initialize();	// Registers all states to be used

	ModelMan.Initialize(); // Loads useful models
	Physics.Initialize();
	Input.Initialize();		// Loadinput-bindings
	NetworkMan.Initialize();
#ifdef USE_FTP
	Ftp.Initialize();
#endif
	WaypointMan.Initialize();

	// Begin threads for rendering, etc.
	CREATE_AND_START_THREAD(GraphicsManager::Processor, graphicsThread);
	// Begin audio thread.
	CREATE_AND_START_THREAD(AudioManager::Processor, audioThread);
	
	// Initialize the mapManager, thereby loading the default map
	MapMan.Initialize();

	// Check that all managers have been initialized before we continue..

	// Run startup.ini if it is available, running all its arguments through the message manager, one line at a time.
	{
		List<String> lines = File::GetLines("startup.ini");
		for (int i = 0; i < lines.Size(); ++i)
		{
			String line = lines[i];
			if (line.StartsWith("//"))
				continue;
			MesMan.QueueMessages(line);
		}
	}

	// Post message to the message manager that all managers have been initialized properly.
	/// Start the state-processing thread!
	CREATE_AND_START_THREAD(StateManager::StateProcessor, stateProcessingThread);

	// Now begin accepting input!
	Input.acceptInput = true;

	StateMan.SetGlobalState(GameStateID::GAME_STATE_GLOBAL);
	std::cout<<"\nInitialization done! Entering main menu...";
	StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));

	/// Everything should be setup, so evaluate any command-line arguments now!
	CommandLine::Evaluate();

	/// Inform that the thread has ended.
	RETURN_NULL(initializerThread);
}


int deallocatorThreadStartCount = 0;
/// Signifies that the application is currently exiting.
bool quittingApplication = false;

THREAD_START(Deallocate)
{
	quittingApplication = true;
    std::cout<<"\nDeallocatorThread started.";
	assert(deallocatorThreadStartCount < 1);
	++deallocatorThreadStartCount;


	/// Stop input.
	MesMan.QueueMessages("AcceptInput:false");
	MesMan.QueueMessages("HideWindows");
	// Unregister all entities.
	GraphicsMan.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
	PhysicsMan.QueueMessage(new PhysicsMessage(PM_UNREGISTER_ALL_ENTITIES));
	AudioMan.QueueMessage(new AudioMessage(AM_STOP_ALL));
	Sleep(50);

	MesMan.QueueMessages("SetActiveState:NULL");
	MesMan.QueueMessages("SetGlobalState:NULL");
	MesMan.QueueMessages("StateMan.DeleteStates");
	MesMan.QueueMessages("NetworkMan.Shutdown");
	MesMan.QueueMessages("MultimediaMan.Shutdown");
	MesMan.QueueMessages("AudioMan.Shutdown");
	MesMan.QueueMessages("GraphicsMan.Shutdown");

	// Stop the state loop	
	double timeStart = clock();
	double timeTaken;

	// Wait for graphics thread to end.
	while(graphicsThread)
	{
		Sleep(10);
	}
	while(audioThread)
	{
		Sleep(10);
	}
	// Notify the message manager and game states to deallocate their windows.
	MesMan.QueueMessages("DeleteWindows");
	
	timeStart = clock();

	// TODO: Send all UIs to be de-allocated properly via the graphics manager?

	/// Post quit message with return value 0! Successful deallocation.
	std::cout<<"\nPosting quit message.";
#ifdef WINDOWS
	PostQuitMessage(0);
#endif

	// Stop graphics processing, now that we hopefully have deallocated all resources that are relevant!
	MesMan.QueueMessages("StateMan.Shutdown");

    std::cout<<"\nWaiting for state processing thread to end..";


	while(stateProcessingThread)
		Sleep(10);

	// Wait until the graphics thread has ended before deallocating more.
	while(!Graphics.finished)
		Sleep(10);

	std::cout<<"\n>>>DeallocatorThread ending...";

	/// Set the application's live to false so that the main loop will exit.
	Application::live = false;

	/// Inform that the thread has ended.
	RETURN_NULL(deallocatorThread);
}
