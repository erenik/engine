/// Emil Hedemalm
/// 2013-03-07
/// Main file of ze whole program!

#include <iostream>

#include <cstdlib>
#include <fcntl.h>
#include <ctime>

#include "Application/Application.h"
#include "Window/WindowSystem.h"
#include "Managers.h" // Don't include all managers. Ever. Except here and in Initializer/Deallocator.
#include "OS/Sleep.h"

#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <string.h>
#include "Initializer.h"
#include "Debug.h"
#include "Command/CommandLine.h"

/// Unit test includes
#include "PhysicsLib/Estimator.h"

/// Win32-specifics
#ifdef WINDOWS
    #define _CRTDBG_MAP_ALLOC
    #include <malloc.h>
    #include <stdlib.h>
#ifdef _MSC_VER // MSVC++ specific header
#define MSVC
    #include <crtdbg.h>
#ifdef _DEBUG
	// Visual leak detector, o.o, http://vld.codeplex.com/
	#include <vld.h> 
#endif
#endif 
//    #include "config.h"
    #include <io.h>
#ifndef UNICODE
#define UNICODE
#endif
    #include <windows.h>
    #include <tchar.h>
    #include <process.h>
    
    // Global variables
    extern uintptr_t initializerThread;

	// For COM-interaction, drag-n-drop, http://msdn.microsoft.com/en-us/library/windows/desktop/ms690134%28v=vs.85%29.aspx
	#include <Ole2.h>

/// Linux-specifics!
#elif defined USE_X11
// Apple OSX specifics!
#elif defined OSX
    /*
    #include <AppKit/NSWindow.h>
    #include <AppKit/AppKit.h>
    //#include <X11/Xlib.h>
    NSWindow * AppWindow = NULL;
    */
#endif // OS-specifics


/// Thread references
/// Windows includes and globals
#ifdef WINDOWS
    #include <windows.h>
    #include <process.h>
    /// Render/Physics/Multimedia-thread.
    extern uintptr_t graphicsThread;
/// Linux includes and globals
#elif defined USE_X11
    /// Render/Physics/Multimedia-thread.
    extern pthread_t graphicsThread;
    // Global variables
    extern pthread_t initializerThread;
#endif // OSXx

// POSIX threads
#if defined LINUX | defined OSX
#endif // POSIX threads

#include "File/LogFile.h"
#include "MathLib/Function.h"

extern bool UnitTests();
// void SIMDTest();

/// Kept in GraphicsProcessor.
extern int fatalGraphicsError;
extern int errorCode;


/// Le main! (^o-o^);
#ifdef WINDOWS
// WinMain - Start the application and it's threads
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	// Save away entry-point args straight away.
	Application::hInstance = hInstance;
	Application::nCmdShow = nCmdShow;
#elif defined LINUX | defined OSX
int main(int argc, char **argv)
{
#endif
	// Set application-defaults here already?
	/// Call to set application name, root directories for various features, etc.
	SetApplicationDefaults();
	/// Load base setup
	List<String> rows = File::GetLines("Setup.txt");
	for (int i = 0; i < rows.Size(); ++i)
	{
		String str = rows[i];
		if (str.StartsWith("LogLevel"))
			SetLogLevel(str);
		if (str.StartsWith("DefaultAudioDriver"))
			AudioManager::SetDefaultAudioDriver(str);
	}
	/// Allocate allocators.
	String::InitializeAllocator();
	/// Initialize math lib
	Expression::InitializeConstants();

	/*
	Function func;
	bool ok = func.LoadFunction("distance = PI * (0.1 * a)");
//	bool ok = func.LoadFunction("distance = v0 * t + a * t ^ 2 / 2");
	if (!ok)
		return 0;

	List<int> results;

	for (int i = 0; i < 40; ++i)
	{
		Variable var("a", i);
		List<Variable> variables(var);

		ExpressionResult result =  func.Evaluate(variables);
		switch(result.type)
		{
			case DataType::FLOAT:
				std::cout<<"\nResult: "<<result.fResult;	
				break;
			case DataType::INTEGER:
				std::cout<<"\nResult: "<<result.iResult;
				break;
			default:
				std::cout<<"\nNo workie?";
				break;
		}
	}
	
	return 0;
	*/

/// Save away command-line arguments for future processing.
#ifdef WINDOWS
	String args = GetCommandLine();
	CommandLine::argString = args;
	CommandLine::args = args.Tokenize(" ");
#elif defined LINUX
    for (int i = 0; i < argc; ++i){
        String s = argv[i];
        std::cout<<"\ncmd: "<<s;
        CommandLine::args.Add(s);
    }
#endif

//#define DEBUG_MALLOC_ERROR_BREAK
#ifdef DEBUG_MALLOC_ERROR_BREAK

    for (int i  =0; i < 5; ++i){
        StateManager::Allocate();
        std::cout<<"\nAllocated\n\n";

        #define STRALLOC { \
        String * strArr = new String[5]; \
        delete[] strArr; \
        }

        STRALLOC

        StateMan.Initialize();
        std::cout<<"\nStateMan initialized.\n\n";

        STRALLOC
        List<String> list;
        for (int i = 0; i < 10000; ++i){
            list.Add("Woshi in da koshi");
            List<String> moreLists = list[i].Tokenize(" s");
        }
#define CREATE_UIS
#ifdef CREATE_UIS
        StateMan.CreateUserInterfaces();
        std::cout<<"\nCreated user interfaces.\n\n";

        StateMan.DeallocateUserInterfaces();
        std::cout<<"\nDeleted user interfaces.\n\n";

#endif // Create UIS
        STRALLOC

        StateManager::Deallocate();
        std::cout<<"\nDeallocated\n\n";

        list.Clear();

        STRALLOC

    }


    return 0;
#endif // Debug malloc


/// Set C-runtime flags and threading options
#ifdef WINDOWS
    #ifdef MSVC
    	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    	_CrtSetReportMode(_CRT_WARN , _CRTDBG_MODE_FILE);
    	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    #endif
#elif defined USE_X11
#endif


// For debug, setting current directory while launching from within VS
// #define SET_ROOT_DIR
#ifdef SET_ROOT_DIR
#ifdef ROOT_DIR
#ifdef WINDOWS
	// Set working directory
	String path = ROOT_DIR;
	BOOL setWorkDirResult = SetCurrentDirectory(path.wc_str());
	if (!setWorkDirResult)
	{
		assert(false && "Failed to set working directory!");
	}
#endif // WINDOWS
#endif // ROOT_DIR
#endif // SET_ROOT_DIR


#ifdef WINDOWS
#ifdef CONSOLE_ENABLED
	// Allocate debug console
	AllocConsole();

	// redirect unbuffered STDOUT to the console
	long lStdHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE);
	int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	FILE * fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	std::ios::sync_with_stdio();
#endif // CONSOLE_ENABLED
#endif // WINDOWS

	// Unit tests here if wanted.
	if (UnitTests())
		return 0;

    // Register AppWindow pre-stuffs.
#ifdef WINDOWS
	// For COM-interaction, drag-n-drop, http://msdn.microsoft.com/en-us/library/windows/desktop/ms690134%28v=vs.85%29.aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms695279%28v=vs.85%29.aspx
//	int result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
//	int result = OleInitialize(NULL);
//	assert(result == S_OK);
	// Create the AppWindow manager.
	WindowManager::Allocate();
	// Save to global application instance variables.
    hInstance = hInstance;
	std::cout<<"Starting up Win32 application...";
	WindowMan.CreateDefaultWindowClass();
/// Open XServer?
#elif defined USE_X11
/*
    /// Create GL context! ^^
    context = glXCreateContext(display, visual_info, None, true);
    if (context == NULL)
    {
        assert(false && "could not create rendering context");
    }
*/
#endif

// Set AppWindow options
#ifdef WINDOWS
	// Do that later, actually..
/// Set linux AppWindow options!
#elif defined USE_X11
#endif

/// Create AppWindow .. done elsewhere/later, or perhaps not at all.
#ifdef WINDOWS
	

/// Create the AppWindow, linux-style!
#elif defined USE_X11
/// OSX AppWindow creation!
#elif defined OSX & 0
    std::cout<<"\nSetting up NS Applicaiton.";
    [NSApplication sharedApplication];

    std::cout<<"\nWindow maskurr.";
    unsigned int windowMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    NSRect mainDisplayRect = NSMakeRect(100, 100, 200, 200);
    //[[NSScreen mainScreen] frame];
    NSWindow * AppWindow = [[NSWindow alloc]
                         initWithContentRect: mainDisplayRect
                         styleMask: windowMask
                         backing: NSBackingStoreBuffered
                         defer: NO];
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        0
    };
    NSOpenGLPixelFormat * pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLView * fullScreenView = [[NSOpenGLView alloc]
                                     initWithFrame:mainDisplayRect
                                     pixelFormat:pixelFormat
                                     ];
    [AppWindow setAutodisplay:true];
    [AppWindow setContentView:fullScreenView];

    [AppWindow makeKeyAndOrderFront: AppWindow];
    std::cout<<"\nWindow should now.. .maybe be visible?";
#endif // Setting up AppWindow


    /// Set up AppWindow-management details
#ifdef LINUX
#endif
	
	// Create mutexes.
	CreateUIMutex();

	// Initialize all managers
	CameraManager::Allocate();
	PreferencesManager::Allocate();
	PlayerManager::Allocate();
	MessageManager::Allocate();
	ShaderManager::Allocate();
	GraphicsManager::Allocate();
	TextureManager::Allocate();
	NetworkManager::Allocate();
#ifdef USE_FTP
	FtpManager::Allocate();
#endif // USE_FTP
	StateManager::Allocate();
	MapManager::Allocate();
	PhysicsManager::Allocate();
	InputManager::Allocate();
	ModelManager::Allocate();
	EntityManager::Allocate();
	PathManager::Allocate();
	WaypointManager::Allocate();
	ScriptManager::Allocate();
	AudioManager::Allocate();
	GameVariableManager::Allocate();
	FrameStatistics::Allocate();
	ChatManager::Allocate();
	AnimationManager::Allocate();
	TrackManager::Allocate();
	MultimediaManager::Allocate();
	GridObjectTypeManager::Allocate();

//#define TEST_RENDER

	// Start the initializer thread
    CREATE_AND_START_THREAD(Initialize, initializerThread);
    /*
#ifdef WINDOWS
	initializerThread = _beginthread(Initialize, NULL, NULL);
	// Reveal the main AppWindow to the user now that all managers are allocated.
//	mainWindow->Show();

#elif defined LINUX | defined OSX
#ifndef TEST_RENDER
    int iret1 = pthread_create(&initializerThread, NULL, Initialize, NULL);
    SleepThread(50);
    assert(iret1 == 0);
    /// Wait for initializer to complete!
    std::cout<<"\nInitializer thread started!";
    pthread_join(initializerThread, NULL);
  //  return 0;

    std::cout<<"\nInitializer thread joined.";
#endif // TEST_RENDER
#endif // LINUX
*/
	/// Wait until the render thread has been set up properly?
	int spams = 0;
    while(!Graphics.enteringMainLoop){
		++spams;
		if (spams > 300){
			std::cout<<"\nWaiting for GraphicsProcessor to enter main rendering-loop.";
			spams = 0;
			if (fatalGraphicsError > 0)
			{
				errorCode = fatalGraphicsError;
				break;
			}
		}
        SleepThread(10);
    }
// Main wait loop. Does nothing but wait for the game to finish.
	while(Application::live)
	{
		// Sleep a bit? No?
		SleepThread(1000);
	}

	/// Unlink windows processor from our game AppWindow, since we're not interested in more messages.
	

	// Start deallocator thread here instead?
	// Call the deallocator thread!
	if (StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING)
		StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EXITING));


	double timeStart = clock();
	double timeTaken;

	extern THREAD_HANDLE deallocatorThread;
	while(deallocatorThread)
		SleepThread(5);
    /// Wait for initializer to complete!
    std::cout<<"\nWaiting for DeallocatorThread...";

	timeTaken = clock() - timeStart;
	std::cout<<"\nWaiting for deallocatorThread total time: "<<timeTaken / CLOCKS_PER_SEC<<" seconds";

    std::cout<<"\nWaiting for state processing thread...";
	extern THREAD_HANDLE stateProcessingThread;
	while(stateProcessingThread)
		SleepThread(5);

    /// Wait until graphics thread has ended before going on to deallocation!
    while(graphicsThread)
    {
        SleepThread(5);
        std::cout<<"Waiting for graphics thread to end before deallocating managers.";
    }
	std::cout<<"\nState processor thread ended.";
	
	// Delete ALL windows o-o
//	WindowMan.DeleteWindows();
	
	std::cout<<"\nDeallocating all managers.";
	timeStart = clock();

	// Deallocate all managers
	GridObjectTypeManager::Deallocate();
	MultimediaManager::Deallocate();
	TrackManager::Deallocate();
	AnimationManager::Deallocate();
	ChatManager::Deallocate();
	GameVariableManager::Deallocate();
	AudioManager::Deallocate();
	ScriptManager::Deallocate();
	MessageManager::Deallocate();
	ShaderManager::Deallocate();
	FrameStatistics::Deallocate();
	TextureManager::Deallocate();
	NetworkManager::Deallocate();
#ifdef USE_FTP
	FtpManager::Deallocate();
#endif // USE_FTP
	StateManager::Deallocate();
	MapManager::Deallocate();
	// Delete all entities, yo! wtf
	EntityManager::Deallocate();
	GraphicsManager::Deallocate();
	PhysicsManager::Deallocate();
	ModelManager::Deallocate();
	PathManager::Deallocate();
	WaypointManager::Deallocate();
	InputManager::Deallocate();
	PlayerManager::Deallocate();
	PreferencesManager::Deallocate();
	WindowManager::Deallocate();
	CameraManager::Deallocate();
	UserInterface::DeleteAll();

	/// Delete any remaining small resources.
	Light::FreeAll();

	
	std::cout<<"\nManagers deallocated.";
	timeTaken = clock() - timeStart;
	std::cout<<"\nWaiting for manager-deallocation: "<<timeTaken / CLOCKS_PER_SEC<<" seconds";
	
	// Delete mutexes.
	DeleteUIMutex();
//	MathLib::Free();

	std::cout<<"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
	std::cout<<"\n>>>Main finishing.    >>>";
    std::cout<<"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

	SleepThread(100);
	if (errorCode != 0)
	{
		if (errorCode == -1)
		{
			extern String errorMessage;
			std::cout<<"\nFatal Error: "<<errorMessage;
		}
		else
			std::cout<<"\nApplication ending. Error code: "<<errorCode<<". See log files in /log/ for more info.";
		SleepThread(3000);
	}
	return (int) errorCode;
}


