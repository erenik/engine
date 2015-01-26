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
#include "Debugging.h"
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
    #include <GL/glew.h>
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
    #include <GL/glx.h>     // connect X server with OpenGL
    #include "XProc.h"      // XWindow Event Processor
    int ErrorHandler(Display * d, XErrorEvent * e);
    // single buffer attributes
    static int singleBufferAttributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, None};
    // doubble buffer attributes
    static int doubleBufferAttributes[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    /// Program start-up variables!
        XEvent                  event;
        GLXContext              context; // OpenGL context
        Display*                display; // connection to X server
        XVisualInfo*            visual_info;
        Window                  window;
        XSetWindowAttributes    window_attributes;
        Colormap                colormap;
        bool                    swapBuffers;
    void testRender();
// Apple OSX specifics!
#elif defined OSX
    /*
    #include <AppKit/NSWindow.h>
    #include <AppKit/AppKit.h>
    //#include <X11/Xlib.h>
    NSWindow * window = NULL;
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


#include "MathLib/Function.h"

void SIMDTest();

/// Kept in GraphicsProcessor.
extern int fatalGraphicsError;


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
    /// Initialize support for multi-threaded usage of Xlib
    int status = XInitThreads();
    if (status){
        std::cout<<"\n*nix thread-support initialized.";
    }
    else {
        std::cout<<"\nXInitThreads failed.";
        return 0;
    }
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
	
	List<int> list;
	int ints[] = {11,22,33,44};
	list.AddArray(4, ints);
	for (int i = 0; i < list.Size(); ++i)
	{
		std::cout<<"\ni: "<<list[i];
	}


	Vector4f * test = new Vector4f[500];
	delete[] test;
//#define test
#ifdef test

	// Test SIMD o.o
	SIMDTest();
	return 0;
#endif


    // Register window pre-stuffs.
#ifdef WINDOWS
	// For COM-interaction, drag-n-drop, http://msdn.microsoft.com/en-us/library/windows/desktop/ms690134%28v=vs.85%29.aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms695279%28v=vs.85%29.aspx
//	int result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
//	int result = OleInitialize(NULL);
//	assert(result == S_OK);


	// Create the window manager.
	WindowManager::Allocate();
	// Save to global application instance variables.
    hInstance = hInstance;
	std::cout<<"Starting up Win32 application...";
	WindowMan.CreateDefaultWindowClass();
/// Open XServer?
#elif defined USE_X11
    std::cout<<"Starting up XWindowSystem application...";
    display = XOpenDisplay(NULL);
    if (!display){
        std::cout<<"\nERROR: Unable to open connection to XServer!";
        return -1;
    }
    int dummy;
    if(!glXQueryExtension(display, &dummy, &dummy))
    {
        std::cout<<"\nERROR: XServer has no GLX extension!";
        return -2;
        assert(false && "X server has no OpenGL GLX extension");
    }

    /// Find OpenGL-capable RGB visual with depth buffer (device context?)
    visual_info = glXChooseVisual(display, DefaultScreen(display), doubleBufferAttributes);
    if (visual_info == NULL){
        std::cout << "No double buffer" << std::endl;

        visual_info = glXChooseVisual(display, DefaultScreen(display), singleBufferAttributes);
        if (visual_info == NULL){
            assert(false && "no depth buffer");
        }
    }
    else {
        swapBuffers = true;
    }

    /// Set Error handler!
    XSetErrorHandler(ErrorHandler);
/*
    /// Create GL context! ^^
    context = glXCreateContext(display, visual_info, None, true);
    if (context == NULL)
    {
        assert(false && "could not create rendering context");
    }
*/
#endif



// Set window options
#ifdef WINDOWS
	// Do that later, actually..
/// Set linux Window options!
#elif defined USE_X11
    // Each X window always has an associated colormap that provides a level of indirection between pixel values
    // and colors displayed on the screen. The X protocol defines colors using values in the RGB color space.
    colormap = XCreateColormap(display, RootWindow(display, visual_info->screen), visual_info->visual, AllocNone);

    // update window attributes
    window_attributes.colormap = colormap;
    window_attributes.border_pixel = 0;
    window_attributes.event_mask =

    /// Testing to  get clipboard to work...
        EnterWindowMask | LeaveWindowMask | KeymapStateMask |
        VisibilityChangeMask /*| ResizeRedirectMask*/ | SubstructureNotifyMask |
        SubstructureRedirectMask | FocusChangeMask | PropertyChangeMask | ColormapChangeMask |
        OwnerGrabButtonMask |

       // AnyEventMask;
       /// Old stuff
        KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask |
        ExposureMask | ButtonPressMask | StructureNotifyMask |
        PointerMotionMask;

    //  XSelectInput (display, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
#endif


	/// Create window
#ifdef WINDOWS
	

/// Create the window, linux-style!
#elif defined USE_X11
    window = XCreateWindow(display,
                           RootWindow(display, visual_info->screen),
                           0, 0,            /// Position
                           WINDOW_WIDTH, WINDOW_HEIGHT,   /// Size
                           0,
                           visual_info->depth,
                           InputOutput,
                           visual_info->visual,
                           CWBorderPixel | CWColormap | CWEventMask,
                           &window_attributes);
    // set window properties
    XSetStandardProperties(display, window, "main", None, None, argv, argc, NULL);

/*
    // bind the rendering context to the window
    bool bound = glXMakeContextCurrent(display, window, window, context);
    if (bound == false)
    {
        assert(false && "Failed to bind context");
    }
*/
    // display X window on screen
    XMapWindow(display, window);

/// OSX Window creation!
#elif defined OSX & 0

    std::cout<<"\nSetting up NS Applicaiton.";
    [NSApplication sharedApplication];

    std::cout<<"\nWindow maskurr.";
    unsigned int windowMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    NSRect mainDisplayRect = NSMakeRect(100, 100, 200, 200);
    //[[NSScreen mainScreen] frame];
    NSWindow * window = [[NSWindow alloc]
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
    [window setAutodisplay:true];
    [window setContentView:fullScreenView];

    [window makeKeyAndOrderFront: window];
    std::cout<<"\nWindow should now.. .maybe be visible?";
#endif // Setting up window


    /// Set up Window-management details
#ifdef LINUX
    /// Fix so we can intercept Window-Management messages (like pressing the Close-button, ALT+F4, etc!)
    // Ref: http://www.opengl.org/discussion_boards/showthread.php/157469-Properly-destroying-a-window
    Atom wm_protocol = XInternAtom (display, "WM_PROTOCOLS", False);
    Atom wm_close = XInternAtom (display, "WM_DELETE_WINDOW", False);
    // Next we elect to receive the 'close' event from the WM:
    XSetWMProtocols (display, window, &wm_close, 1);

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
#ifdef WINDOWS
	initializerThread = _beginthread(Initialize, NULL, NULL);

	// Reveal the main window to the user now that all managers are allocated.
//	mainWindow->Show();

#elif defined LINUX | defined OSX
#ifndef TEST_RENDER
    int iret1 = pthread_create(&initializerThread, NULL, Initialize, NULL);
    Sleep(50);
    assert(iret1 == 0);
    /// Wait for initializer to complete!
    std::cout<<"\nInitializer thread started!";
    pthread_join(initializerThread, NULL);
  //  return 0;

    std::cout<<"\nInitializer thread joined.";
#endif // TEST_RENDER
#endif // LINUX

	/// Wait until the render thread has been set up properly?
	int spams = 0;
	int errorCode = 0;
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
        Sleep(10);
    }
// Main wait loop. Does nothing but wait for the game to finish.
#ifdef WINDOWS
	while(Application::live)
	{
		// Sleep a bit? No?
		Sleep(500);
	}

/// X11 message loop!
#elif defined USE_X11
    std::cout<<"\nBeginning listening to events...";
   // XNoOp(display);
    int messagesReceived = 0;
    while(StateMan.ActiveStateID() != GAME_STATE_EXITING){
        // Check for queued messages.
        int events = XPending(display);
        /// XEventsQueued(display, QueuedAfterReading);
      //  if (events > 0)
        //    std::cout<<"\nQueued events: "<<events;
        if (events){
            // XNextEvent may block until an event appears, which might not be wanted, to check beforehand how many events are available!
            XNextEvent(display, &event);
            if (XProc(event) != NULL)
                break;
        }
        else {
            // If no messages, allow some sleep?
            Sleep(10);
         //   std::cout<<"\nSleeping";
        }
    }
#endif

	/// Unlink windows processor from our game window, since we're not interested in more messages.
	

	// Start deallocator thread here instead?
	// Call the deallocator thread!
	if (StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING)
		StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EXITING));


	double timeStart = clock();
	double timeTaken;


#ifdef WINDOWS
	extern uintptr_t deallocatorThread;
	while(deallocatorThread)
		Sleep(5);
#elif defined LINUX | defined OSX
    /// Start it if it isnt already!
    extern pthread_t deallocatorThread;
    while (deallocatorThread)
        Sleep(5);
    /// Wait for initializer to complete!
    std::cout<<"\nWaiting for DeallocatorThread...";
    pthread_join(deallocatorThread, NULL);
#endif
	timeTaken = clock() - timeStart;
	std::cout<<"\nWaiting for deallocatorThread total time: "<<timeTaken / CLOCKS_PER_SEC<<" seconds";


    std::cout<<"\nWaiting for state processing thread...";
#ifdef WINDOWS
	// Wait for the state processing thead too.
	extern uintptr_t stateProcessingThread;
	while(stateProcessingThread)
		Sleep(5);
#elif defined LINUX | defined OSX
	extern pthread_t stateProcessingThread;
	while(stateProcessingThread)
		Sleep(5);
#endif

    /// Wait until graphics thread has ended before going on to deallocation!
    while(graphicsThread)
    {
        Sleep(5);
        std::cout<<"Waiting for graphics thread to end before deallocating managers.";
    }
	std::cout<<"\nState processor thread ended.";
	
	std::cout<<"\nDestroy window.";
	
	// Delete ALL windows o-o
	WindowMan.DeleteWindows();


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

	
	std::cout<<"\nManagers deallocated.";
	timeTaken = clock() - timeStart;
	std::cout<<"\nWaiting for manager-deallocation: "<<timeTaken / CLOCKS_PER_SEC<<" seconds";
	
	// Delete mutexes.
	DeleteUIMutex();

	std::cout<<"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
	std::cout<<"\n>>>Main finishing.    >>>";
    std::cout<<"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

#ifdef WINDOWS

	// De-allocate COM stuffs
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms688715%28v=vs.85%29.aspx
//	CoUninitialize();

	/// Post debug to output window.
//	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	/// Post memory leaks
//	_CrtDumpMemoryLeaks();
	Sleep(100);
	if (errorCode > 0)
	{
		std::cout<<"\nApplication ending. Error code: "<<errorCode<<". See log files in /log/ for more info.";
		Sleep(3000);
	}
	return (int) errorCode;
#elif defined LINUX | defined OSX
    return 0;
#endif
}


#ifdef USE_X11

/// Error handler
int ErrorHandler(Display * d, XErrorEvent * e){
    std::cout<<"\nXErrorEvent ";
    switch(e->type){
        case BadAccess:      std::cout<<"BadAccess"; break;
        case BadAlloc:      std::cout<<"BadAlloc"; break;
        case BadAtom:      std::cout<<"BadAtom"; break;
        case BadColor:      std::cout<<"BadColor"; break;
        case BadCursor:      std::cout<<"BadCursor"; break;
        case BadDrawable:      std::cout<<"BadDrawable"; break;
        case BadFont:      std::cout<<"BadFont"; break;
        case BadGC:      std::cout<<"BadGC"; break;
        case BadIDChoice:      std::cout<<"BadIDChoice"; break;
        case BadImplementation:      std::cout<<"BadImplementation"; break;
        case BadLength:      std::cout<<"BadLength"; break;
        case BadMatch:      std::cout<<"BadMatch"; break;
        case BadName:      std::cout<<"BadName"; break;
        case BadPixmap:      std::cout<<"BadPixmap"; break;
        case BadRequest:      std::cout<<"BadRequest"; break;
        case BadValue:      std::cout<<"BadValue"; break;
        case BadWindow:      std::cout<<"BadWindow"; break;
        //case GLXBadContext:      std::cout<<""; break;

        default:      std::cout<<"Unknown/default"; break;
    }
    static const int size = 20;
    char buf[size];
   // XGetErrorText(d, e->type, buf, size);
   // std::cout<<": "<<buf;

    return 0;
}

/// For testing linux gl
void testRender(){
// Setup rendering
    glEnable(GL_DEPTH_TEST); // enable depth buffering
    glDepthFunc(GL_LESS);    // pedantic, GL_LESS is the default
    glClearDepth(1.0);       // pedantic, 1.0 is the default
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 1000.0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


    float x_rot = 0.0;

    // Rendering
    while(true)
    {
            // XNextEvent is required to get the OS to do anything at all... ish? o-o
       //     XNextEvent(display, &event);
            Sleep(10);


            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glRotatef(x_rot, 0.0, 1.0, 0.0);
            glBegin(GL_TRIANGLES);
                glColor3f (  1.0,  0.0,  0.0 ); // red
                glVertex3f(  0.0,  0.5, -1.0 );

                glColor3f (  0.0,  1.0,  0.0 ); // green
                glVertex3f( -0.5, -0.5, -1.0 );

                glColor3f (  0.0,  0.0,  1.0 ); // blue
                glVertex3f(  0.5, -0.5, -1.0 );
            glEnd();
            x_rot += 0.1;
          //  std::cout << x_rot << std::endl;

            if(swapBuffers)
            {
                glXSwapBuffers( display, window );
            }
            else
            {
                glFlush();
            }
    }
};

#endif // LINUX



#include <xmmintrin.h>

#include <intrin.h>


const char* szFeatures[] =
{
    "x87 FPU On Chip",
    "Virtual-8086 Mode Enhancement",
    "Debugging Extensions",
    "Page Size Extensions",
    "Time Stamp Counter",
    "RDMSR and WRMSR Support",
    "Physical Address Extensions",
    "Machine Check Exception",
    "CMPXCHG8B Instruction",
    "APIC On Chip",
    "Unknown1",
    "SYSENTER and SYSEXIT",
    "Memory Type Range Registers",
    "PTE Global Bit",
    "Machine Check Architecture",
    "Conditional Move/Compare Instruction",
    "Page Attribute Table",
    "36-bit Page Size Extension",
    "Processor Serial Number",
    "CFLUSH Extension",
    "Unknown2",
    "Debug Store",
    "Thermal Monitor and Clock Ctrl",
    "MMX Technology",
    "FXSAVE/FXRSTOR",
    "SSE Extensions",
    "SSE2 Extensions",
    "Self Snoop",
    "Multithreading Technology",
    "Thermal Monitor",
    "Unknown4",
    "Pend. Brk. EN."
};

// http://www.codeproject.com/Articles/4522/Introduction-to-SSE-Programming
// https://msdn.microsoft.com/de-de/library/hskdteyh%28v=vs.90%29.aspx
// https://msdn.microsoft.com/en-us/library/hskdteyh.aspx
void SIMDTest()
{
	char CPUString[0x20];
    char CPUBrandString[0x40];
    int CPUInfo[4] = {-1};
    int nSteppingID = 0;
    int nModel = 0;
    int nFamily = 0;
    int nProcessorType = 0;
    int nExtendedmodel = 0;
    int nExtendedfamily = 0;
    int nBrandIndex = 0;
    int nCLFLUSHcachelinesize = 0;
    int nLogicalProcessors = 0;
    int nAPICPhysicalID = 0;
    int nFeatureInfo = 0;
    int nCacheLineSize = 0;
    int nL2Associativity = 0;
    int nCacheSizeK = 0;
    int nPhysicalAddress = 0;
    int nVirtualAddress = 0;
    int nRet = 0;
    unsigned    nIds, nExIds, i;

    bool    bSSE3Instructions = false;
    bool    bMONITOR_MWAIT = false;
    bool    bCPLQualifiedDebugStore = false;
    bool    bVirtualMachineExtensions = false;
    bool    bEnhancedIntelSpeedStepTechnology = false;
    bool    bThermalMonitor2 = false;
    bool    bSupplementalSSE3 = false;
    bool    bL1ContextID = false;
    bool    bCMPXCHG16B = false;
    bool    bxTPRUpdateControl = false;
    bool    bPerfDebugCapabilityMSR = false;
    bool    bSSE41Extensions = false;
    bool    bSSE42Extensions = false;
    bool    bPOPCNT = false;

    bool    bMultithreading = false;

    bool    bLAHF_SAHFAvailable = false;
    bool    bCmpLegacy = false;
    bool    bSVM = false;
    bool    bExtApicSpace = false;
    bool    bAltMovCr8 = false;
    bool    bLZCNT = false;
    bool    bSSE4A = false;
    bool    bMisalignedSSE = false;
    bool    bPREFETCH = false;
    bool    bSKINITandDEV = false;
    bool    bSYSCALL_SYSRETAvailable = false;
    bool    bExecuteDisableBitAvailable = false;
    bool    bMMXExtensions = false;
    bool    bFFXSR = false;
    bool    b1GBSupport = false;
    bool    bRDTSCP = false;
    bool    b64Available = false;
    bool    b3DNowExt = false;
    bool    b3DNow = false;
    bool    bNestedPaging = false;
    bool    bLBRVisualization = false;
    bool    bFP128 = false;
    bool    bMOVOptimization = false;

	// __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
	nIds = CPUInfo[0];
    memset(CPUString, 0, sizeof(CPUString));
    *((int*)CPUString) = CPUInfo[1];
    *((int*)(CPUString+4)) = CPUInfo[3];
    *((int*)(CPUString+8)) = CPUInfo[2];

 // Get the information associated with each valid Id
    for (i=0; i<=nIds; ++i)
    {
        __cpuid(CPUInfo, i);
        printf_s("\nFor InfoType %d\n", i); 
        printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        // Interpret CPU feature information.
        if  (i == 1)
        {
            nSteppingID = CPUInfo[0] & 0xf;
            nModel = (CPUInfo[0] >> 4) & 0xf;
            nFamily = (CPUInfo[0] >> 8) & 0xf;
            nProcessorType = (CPUInfo[0] >> 12) & 0x3;
            nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
            nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
            nBrandIndex = CPUInfo[1] & 0xff;
            nCLFLUSHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
            nLogicalProcessors = ((CPUInfo[1] >> 16) & 0xff);
            nAPICPhysicalID = (CPUInfo[1] >> 24) & 0xff;
            bSSE3Instructions = (CPUInfo[2] & 0x1) || false;
            bMONITOR_MWAIT = (CPUInfo[2] & 0x8) || false;
            bCPLQualifiedDebugStore = (CPUInfo[2] & 0x10) || false;
            bVirtualMachineExtensions = (CPUInfo[2] & 0x20) || false;
            bEnhancedIntelSpeedStepTechnology = (CPUInfo[2] & 0x80) || false;
            bThermalMonitor2 = (CPUInfo[2] & 0x100) || false;
            bSupplementalSSE3 = (CPUInfo[2] & 0x200) || false;
            bL1ContextID = (CPUInfo[2] & 0x300) || false;
            bCMPXCHG16B= (CPUInfo[2] & 0x2000) || false;
            bxTPRUpdateControl = (CPUInfo[2] & 0x4000) || false;
            bPerfDebugCapabilityMSR = (CPUInfo[2] & 0x8000) || false;
            bSSE41Extensions = (CPUInfo[2] & 0x80000) || false;
            bSSE42Extensions = (CPUInfo[2] & 0x100000) || false;
            bPOPCNT= (CPUInfo[2] & 0x800000) || false;
            nFeatureInfo = CPUInfo[3];
            bMultithreading = (nFeatureInfo & (1 << 28)) || false;
        }
    }

    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (i=0x80000000; i<=nExIds; ++i)
    {
        __cpuid(CPUInfo, i);
        printf_s("\nFor InfoType %x\n", i); 
        printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
        printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
        printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
        printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

        if  (i == 0x80000001)
        {
            bLAHF_SAHFAvailable = (CPUInfo[2] & 0x1) || false;
            bCmpLegacy = (CPUInfo[2] & 0x2) || false;
            bSVM = (CPUInfo[2] & 0x4) || false;
            bExtApicSpace = (CPUInfo[2] & 0x8) || false;
            bAltMovCr8 = (CPUInfo[2] & 0x10) || false;
            bLZCNT = (CPUInfo[2] & 0x20) || false;
            bSSE4A = (CPUInfo[2] & 0x40) || false;
            bMisalignedSSE = (CPUInfo[2] & 0x80) || false;
            bPREFETCH = (CPUInfo[2] & 0x100) || false;
            bSKINITandDEV = (CPUInfo[2] & 0x1000) || false;
            bSYSCALL_SYSRETAvailable = (CPUInfo[3] & 0x800) || false;
            bExecuteDisableBitAvailable = (CPUInfo[3] & 0x10000) || false;
            bMMXExtensions = (CPUInfo[3] & 0x40000) || false;
            bFFXSR = (CPUInfo[3] & 0x200000) || false;
            b1GBSupport = (CPUInfo[3] & 0x400000) || false;
            bRDTSCP = (CPUInfo[3] & 0x8000000) || false;
            b64Available = (CPUInfo[3] & 0x20000000) || false;
            b3DNowExt = (CPUInfo[3] & 0x40000000) || false;
            b3DNow = (CPUInfo[3] & 0x80000000) || false;
        }

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        else if  (i == 0x80000006)
        {
            nCacheLineSize = CPUInfo[2] & 0xff;
            nL2Associativity = (CPUInfo[2] >> 12) & 0xf;
            nCacheSizeK = (CPUInfo[2] >> 16) & 0xffff;
        }
        else if  (i == 0x80000008)
        {
           nPhysicalAddress = CPUInfo[0] & 0xff;
           nVirtualAddress = (CPUInfo[0] >> 8) & 0xff;
        }
        else if  (i == 0x8000000A)
        {
            bNestedPaging = (CPUInfo[3] & 0x1) || false;
            bLBRVisualization = (CPUInfo[3] & 0x2) || false;
        }
        else if  (i == 0x8000001A)
        {
            bFP128 = (CPUInfo[0] & 0x1) || false;
            bMOVOptimization = (CPUInfo[0] & 0x2) || false;
        }
    }

    // Display all the information in user-friendly format.

    printf_s("\n\nCPU String: %s\n", CPUString);

    if  (nIds >= 1)
    {
        if  (nSteppingID)
            printf_s("Stepping ID = %d\n", nSteppingID);
        if  (nModel)
            printf_s("Model = %d\n", nModel);
        if  (nFamily)
            printf_s("Family = %d\n", nFamily);
        if  (nProcessorType)
            printf_s("Processor Type = %d\n", nProcessorType);
        if  (nExtendedmodel)
            printf_s("Extended model = %d\n", nExtendedmodel);
        if  (nExtendedfamily)
            printf_s("Extended family = %d\n", nExtendedfamily);
        if  (nBrandIndex)
            printf_s("Brand Index = %d\n", nBrandIndex);
        if  (nCLFLUSHcachelinesize)
            printf_s("CLFLUSH cache line size = %d\n",
                     nCLFLUSHcachelinesize);
        if (bMultithreading && (nLogicalProcessors > 0))
           printf_s("Logical Processor Count = %d\n", nLogicalProcessors);
        if  (nAPICPhysicalID)
            printf_s("APIC Physical ID = %d\n", nAPICPhysicalID);

        if  (nFeatureInfo || bSSE3Instructions ||
             bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
             bVirtualMachineExtensions || bEnhancedIntelSpeedStepTechnology ||
             bThermalMonitor2 || bSupplementalSSE3 || bL1ContextID || 
             bCMPXCHG16B || bxTPRUpdateControl || bPerfDebugCapabilityMSR || 
             bSSE41Extensions || bSSE42Extensions || bPOPCNT || 
             bLAHF_SAHFAvailable || bCmpLegacy || bSVM ||
             bExtApicSpace || bAltMovCr8 ||
             bLZCNT || bSSE4A || bMisalignedSSE ||
             bPREFETCH || bSKINITandDEV || bSYSCALL_SYSRETAvailable || 
             bExecuteDisableBitAvailable || bMMXExtensions || bFFXSR || b1GBSupport ||
             bRDTSCP || b64Available || b3DNowExt || b3DNow || bNestedPaging || 
             bLBRVisualization || bFP128 || bMOVOptimization )
        {
            printf_s("\nThe following features are supported:\n");

            if  (bSSE3Instructions)
                printf_s("\tSSE3\n");
            if  (bMONITOR_MWAIT)
                printf_s("\tMONITOR/MWAIT\n");
            if  (bCPLQualifiedDebugStore)
                printf_s("\tCPL Qualified Debug Store\n");
            if  (bVirtualMachineExtensions)
                printf_s("\tVirtual Machine Extensions\n");
            if  (bEnhancedIntelSpeedStepTechnology)
                printf_s("\tEnhanced Intel SpeedStep Technology\n");
            if  (bThermalMonitor2)
                printf_s("\tThermal Monitor 2\n");
            if  (bSupplementalSSE3)
                printf_s("\tSupplemental Streaming SIMD Extensions 3\n");
            if  (bL1ContextID)
                printf_s("\tL1 Context ID\n");
            if  (bCMPXCHG16B)
                printf_s("\tCMPXCHG16B Instruction\n");
            if  (bxTPRUpdateControl)
                printf_s("\txTPR Update Control\n");
            if  (bPerfDebugCapabilityMSR)
                printf_s("\tPerf\\Debug Capability MSR\n");
            if  (bSSE41Extensions)
                printf_s("\tSSE4.1 Extensions\n");
            if  (bSSE42Extensions)
                printf_s("\tSSE4.2 Extensions\n");
            if  (bPOPCNT)
                printf_s("\tPPOPCNT Instruction\n");

            i = 0;
            nIds = 1;
            while (i < (sizeof(szFeatures)/sizeof(const char*)))
            {
                if  (nFeatureInfo & nIds)
                {
                    printf_s("\t");
                    printf_s(szFeatures[i]);
                    printf_s("\n");
                }

                nIds <<= 1;
                ++i;
            }
            if (bLAHF_SAHFAvailable)
                printf_s("\tLAHF/SAHF in 64-bit mode\n");
            if (bCmpLegacy)
                printf_s("\tCore multi-processing legacy mode\n");
            if (bSVM)
                printf_s("\tSecure Virtual Machine\n");
            if (bExtApicSpace)
                printf_s("\tExtended APIC Register Space\n");
            if (bAltMovCr8)
                printf_s("\tAltMovCr8\n");
            if (bLZCNT)
                printf_s("\tLZCNT instruction\n");
            if (bSSE4A)
                printf_s("\tSSE4A (EXTRQ, INSERTQ, MOVNTSD, MOVNTSS)\n");
            if (bMisalignedSSE)
                printf_s("\tMisaligned SSE mode\n");
            if (bPREFETCH)
                printf_s("\tPREFETCH and PREFETCHW Instructions\n");
            if (bSKINITandDEV)
                printf_s("\tSKINIT and DEV support\n");
            if (bSYSCALL_SYSRETAvailable)
                printf_s("\tSYSCALL/SYSRET in 64-bit mode\n");
            if (bExecuteDisableBitAvailable)
                printf_s("\tExecute Disable Bit\n");
            if (bMMXExtensions)
                printf_s("\tExtensions to MMX Instructions\n");
            if (bFFXSR)
                printf_s("\tFFXSR\n");
            if (b1GBSupport)
                printf_s("\t1GB page support\n");
            if (bRDTSCP)
                printf_s("\tRDTSCP instruction\n");
            if (b64Available)
                printf_s("\t64 bit Technology\n");
            if (b3DNowExt)
                printf_s("\t3Dnow Ext\n");
            if (b3DNow)
                printf_s("\t3Dnow! instructions\n");
            if (bNestedPaging)
                printf_s("\tNested Paging\n");
            if (bLBRVisualization)
                printf_s("\tLBR Visualization\n");
            if (bFP128)
                printf_s("\tFP128 optimization\n");
            if (bMOVOptimization)
                printf_s("\tMOVU Optimization\n");
        }
    }

    if  (nExIds >= 0x80000004)
        printf_s("\nCPU Brand String: %s\n", CPUBrandString);

    if  (nExIds >= 0x80000006)
    {
        printf_s("Cache Line Size = %d\n", nCacheLineSize);
        printf_s("L2 Associativity = %d\n", nL2Associativity);
        printf_s("Cache Size = %dK\n", nCacheSizeK);
    }

    if  (nExIds >= 0x80000008)
    {
        printf_s("Physical Address = %d\n", nPhysicalAddress);
        printf_s("Virtual Address = %d\n", nVirtualAddress);
    }

	#define ARRAY_SIZE (4096 * 1024)

	/*

	// Stuff.
	std::cout<<"\nTesting division using regular floating point and SSE SIMD extensions.";
	std::cout<<"\nTest array size: "<<ARRAY_SIZE;
	float * arr1 = new float[ARRAY_SIZE];
	float * arr2 = new float[ARRAY_SIZE];
	std::cout<<"\nFirst numbers";
	// Fill with random values.
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		arr1[i] = (float)rand();
		arr2[i] = (float)rand();
		if (i < 8)
		{
			std::cout<<"\n"<<i<<". "<<arr1[i]<<" op "<<arr2[i];
		}
	}

	float * _arr1, * _arr2;
	_arr1 = (float*) _aligned_malloc(ARRAY_SIZE * sizeof(float), 16);
	_arr2 = (float*) _aligned_malloc(ARRAY_SIZE * sizeof(float), 16);
// Copy values so they can be cross-examined.
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		_arr1[i] = arr1[i];
		_arr2[i] = arr2[i];
	}
	

	// Do division.
	Timer timer;
	timer.Start();
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		arr1[i] = arr1[i] / arr2[i];
//		arr1[i] = arr1[i] - arr2[i];
//		arr1[i] = arr1[i] + arr2[i];
//		arr1[i] = arr1[i] * arr2[i];
	}
	timer.Stop();
	int ms = timer.GetMs();

	// Do division.
	// https://msdn.microsoft.com/en-us/library/c9848chc%28v=vs.90%29.aspx Arithmetic operations! o.o
	int loop = ARRAY_SIZE / 4;
	__m128 * left = (__m128*) _arr1;
	__m128 * right = (__m128*) _arr2;
	__m128 * result = (__m128*)_arr1;
	timer.Start();
	for (int i = 0; i < loop; ++i)
	{
		*result = _mm_div_ps(*left, *right);
//		*result = _mm_sub_ps(*left, *right);
//		*result = _mm_add_ps(*left, *right);
//		*result = _mm_mul_ps(*left, *right);
		++left;
		++right;
		++result;
	}
	timer.Stop();
	int ms2 = timer.GetMs();
	std::cout<<"\nTime taken, regular: "<<ms<<" SSE:"<<ms2;

	// Print results of both to ensure validity.
	for (int i = 0; i < 8; ++i)
	{
		std::cout<<"\nStandard: "<<arr1[i]<<" SSE: "<<_arr1[i];
	}

	for (int i = ARRAY_SIZE - 8; i < ARRAY_SIZE; ++i)
	{
		std::cout<<"\nStandard: "<<arr1[i]<<" SSE: "<<_arr1[i];
	}


	delete[] arr1;
	delete[] arr2;

	_aligned_free(_arr1);
	_aligned_free(_arr2);

*/


	/// Testing vectors..!
	Vector4f one(1,0,-1,0.2), two(0.1,-0.2,1.7,1.0);
#ifdef USE_SSE
	one.PrepareForSIMD();
	two.PrepareForSIMD();
#endif
//	Vector4f::useSSE = false;
	Timer timer;
	timer.Start();
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		one = one + two;
	}
	timer.Stop();
	int ms = timer.GetMs();
	one = Vector4f(1,0,-1,0.2), two = Vector4f(0.1,-0.2,1.7,1.0);
#ifdef USE_SSE
	one.PrepareForSIMD();
	two.PrepareForSIMD();
#endif
//	Vector4f::useSSE = true;
	timer.Start();
	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		one = one + two;
		if (i < 4 || i > ARRAY_SIZE - 4)
#ifdef USE_SSE
			std::cout<<"\ni: "<<one.data.m128_f32[0]<<" "<<one.data.m128_f32[1]<<" "<<one.data.m128_f32[2]<<" "<<one.data.m128_f32[3];
#else
			std::cout<<"\ni: "<<one[0]<<" "<<one[1]<<" "<<one[2]<<" "<<one[3];
#endif
	}
	timer.Stop();
	int ms2 = timer.GetMs();
	std::cout<<"\nVector addition, standard: "<<ms<<" SSE: "<<ms2;
}
