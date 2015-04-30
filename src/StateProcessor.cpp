/// Emil Hedemalm
/// 2015-04-18
/// Main application state, logic and AppWindow-processing thread.

#include "StateManager.h"

#include <ctime>
// #include "Widnow/WindowManager.h"
#include "Managers.h"
#include "File/LogFile.h"
#include "OS/Sleep.h"

extern THREAD_HANDLE stateProcessingThread;

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
	int result = S_OK;
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms695279%28v=vs.85%29.aspx
//	int result = OleInitialize(NULL);
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
						SleepThread(5);
					/// If not in focus, sleep moar!
				//	if (!WindowMan.InFocus())
				//		SleepThread(5);

					/// Process network, sending packets and receiving packets
					NetworkMan.ProcessNetwork();

					/// Get input from XBox devices if possible
					Input.UpdateDeviceStates();
					/// Clear previous frame input before fetching new from the AppWindow system.
					Input.ClearPreviousFrameStats();

					/// Process messages received to our windows from the OS/Window-system.
					WindowMan.ProcessMessages();
					/// Process network packets if applicable
					MesMan.ProcessPackets();
				
				}

				/// Always process messages, even if quitting, as some messages need to be processed here 
				/// (like properly destroying windows)
				MesMan.ProcessMessages();

				// Post messages, if any.
				StateMan.PostMessagesToOtherManagers();
				/// Release mutex at the end of the frame.
				StateMan.stateProcessingMutex.Release();

			}
			else {
				/// Unable to grab mutex? Then sleep for a bit.
				SleepThread(40);
			}
		}
		catch (...)
		{
			LogMain("An unexpected error occurred in the main processing thread (StateManager::StateProcessor).", ERROR);
			std::cout<<"An unexpected error occurred.";
			SleepThread(100);
		}
	}
	LogMain("State processing thread ending", INFO);

	/// Mark application for destruction if not done so already?
//	Application::live = false;

	std::cout<<"\n>>> StateProcessingThread ending...";
#ifdef WINDOWS
	// De-allocate COM stuffs
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms688715%28v=vs.85%29.aspx
	// CoUninitialize();
	OleUninitialize();
	stateProcessingThread = 0;
	_endthread();
#elif defined LINUX | defined OSX
    stateProcessingThread = 0;
#endif

}


