// Emil Hedemalm
// 2013-03-17

#include "Initializer.h"
#include "Globals.h"
#include "Command/CommandLine.h"
#include "Managers.h" // Don't include all managers. Ever. Except here and in Initializer/Deallocator.
#include "OS/Sleep.h"
#include "Graphics/Messages/GMSet.h"

#ifdef WINDOWS
#include <process.h>
uintptr_t initializerThread = NULL;
uintptr_t deallocatorThread = NULL;
uintptr_t graphicsThread = NULL;
uintptr_t stateProcessingThread = NULL;
#define RETURN_NULL	return;
#elif defined LINUX | defined OSX
pthread_t initializerThread = NULL;
pthread_t deallocatorThread = NULL;
pthread_t graphicsThread = NULL;
pthread_t stateProcessingThread = NULL;
#define RETURN_NULL	return NULL;
#endif

int initializers = 0;



// Initialize function to be run with the thread
#ifdef WINDOWS
void Initialize(void * vArgs){
#elif defined LINUX | defined OSX
void * Initialize(void * vArgs){
#endif

    initializers++;
    assert(initializers == 1);

	std::cout<<"\nInitializer thread begun...";

	// Call Initialize for all managers to properly initialize them
	StateMan.Initialize();	// Registers all states to be used

	ModelMan.Initialize(); // Loads useful models
	Graphics.Initialize();	// Load settings and OS/Hardwave defaults
	Physics.Initialize();
	Input.Initialize();		// Loadinput-bindings
	NetworkMan.Initialize();
#ifdef USE_FTP
	Ftp.Initialize();
#endif
	WaypointMan.Initialize();
#ifdef USE_AUDIO
	AudioMan.Initialize();
#endif

	// Begin threads for rendering, etc.
#ifdef WINDOWS
	graphicsThread = _beginthread(GraphicsManager::Processor, NULL, NULL);
#elif defined LINUX | defined OSX
    int iret1 = pthread_create(&graphicsThread, NULL, GraphicsManager::Processor, NULL);
#endif

	// Initialize the mapManager, thereby loading the default map
	MapMan.Initialize();

	// Allocate all UIs
	// Begin loading heavier things after we've set up the rendering thread.
	StateMan.CreateUserInterfaces();

	// Check that all managers have been initialized before we continue...


	// TexMan.loadTextures(GAME_STATE_INITIALIZATION);
	// TexMan.loadTextures(GAME_STATE_MAIN_MENU);

	// Post message to the message manager that all managers have been initialized properly.
	/// Start the state-processing thread!
#ifdef WINDOWS
	stateProcessingThread = _beginthread(StateManager::StateProcessor, NULL, NULL);
#elif defined LINUX | defined OSX
	int iret2 = pthread_create(&stateProcessingThread, NULL, StateManager::StateProcessor, NULL);
#endif

	// Now begin accepting input!
	Input.acceptInput = true;

	StateMan.SetGlobalState(GameStateID::GAME_STATE_GLOBAL);
	std::cout<<"\nInitialization done! Entering main menu...";
	StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));

	/// Everything should be setup, so evaluate any command-line arguments now!
	CommandLine::Evaluate();

/// Skip some stuff if linux until rendering works, please.
#ifdef WINDOWS
#endif // LINUX_TEST

#ifdef WINDOWS
	initializerThread = NULL;
	_endthread();
#endif
}


int deallocatorThreadStartCount = 0;



#ifdef WINDOWS
void Deallocate(void *vArgs){
#elif defined LINUX | defined OSX
void * Deallocate(void *vArgs){
#endif

    std::cout<<"\nDeallocatorThread started.";
	assert(deallocatorThreadStartCount < 1);
	++deallocatorThreadStartCount;

	/// Stop input.
	Input.acceptInput = false;
	Sleep(10);

	StateMan.SetGlobalState(NULL);
	// Stop audio
//	audio.shouldLive = false;
	// Stop the state loop
	StateMan.shouldLive = false;

	double timeStart = clock();
	double timeTaken;

	// Start with stopping the network before deallocating anything to avoid errors
	NetworkMan.Shutdown();

	

	timeStart = clock();
	/// When they are removed, deallocate them.
	while(Graphics.GetUI() || Graphics.GetGlobalUI())
	{
		/// Remove UIs from rendering.
		Graphics.QueueMessage(new GMSetUI(NULL));
		Graphics.QueueMessage(new GMSetGlobalUI(NULL));
		Sleep(5);
	}
	StateMan.DeallocateUserInterfaces();
	timeTaken = clock() - timeStart;
	std::cout<<"\nDeallocating UIs: "<<timeTaken / CLOCKS_PER_SEC<<" seconds";

	/// Post quit message with return value 0! Successful deallocation.
	std::cout<<"\nPosting quit message.";
#ifdef WINDOWS
	PostQuitMessage(0);
#endif

	// Stop graphics processing, now that we hopefully have deallocated all resources that are relevant!
	Graphics.shouldLive = false;

    std::cout<<"\nWaiting for state processing thread to end..";
#ifdef WINDOWS
	while(stateProcessingThread){
		Sleep(10);
	}
#elif defined LINUX | defined OSX
    pthread_join(stateProcessingThread, NULL);
#endif

	/// Remove bindings between active elements, such as multimedia and audio that cannot be destroyed without planning.
	MultimediaMan.Shutdown();
	AudioMan.Shutdown();

	std::cout<<"\n>>>DeallocatorThread ending...";

	/// Deallocation thread quitting!
#ifdef WINDOWS
	deallocatorThread = NULL;
	_endthread();
	/// Inform that we're done here.
#elif defined LINUX | defined OSX
    deallocatorThread = NULL;
#endif
}
