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
#endif 
    #include "config.h"
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
#ifdef test
	EstimatorVec3f::Test(9, 4);
	Sleep(20000);
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
#ifdef USE_AUDIO
	AudioManager::Allocate();
#endif
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
    while(!Graphics.enteringMainLoop){
		++spams;
		if (spams > 300){
			std::cout<<"\nWaiting for GraphicsProcessor to enter main rendering-loop.";
			spams = 0;
		}
        Sleep(10);
    }
// Main wait loop. Does nothing but wait for the game to finish.
#ifdef WINDOWS
	while(true)
	{
		// Sleep a bit? No?
		Sleep(500);
		if (StateMan.ActiveStateID() == GameStateID::GAME_STATE_EXITING || 
			(!StateMan.ActiveState() && !StateMan.GlobalState()))
		{
			break;
		}
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
#ifdef USE_AUDIO
	AudioManager::Deallocate();
#endif
	ScriptManager::Deallocate();
	MessageManager::Deallocate();
	GraphicsManager::Deallocate();
	ShaderManager::Deallocate();
	FrameStatistics::Deallocate();
	TextureManager::Deallocate();
	NetworkManager::Deallocate();
#ifdef USE_FTP
	FtpManager::Deallocate();
#endif // USE_FTP
	StateManager::Deallocate();
	MapManager::Deallocate();
	PhysicsManager::Deallocate();
	ModelManager::Deallocate();
//	EntityManager::Deallocate();
	PathManager::Deallocate();
	WaypointManager::Deallocate();
	InputManager::Deallocate();
	PlayerManager::Deallocate();
	PreferencesManager::Deallocate();
	WindowManager::Deallocate();
	CameraManager::Deallocate();

	std::cout<<"\nManagers deallocated.";
	timeTaken = clock() - timeStart;
	std::cout<<"\nWaiting for manager-deallocation: "<<timeTaken / CLOCKS_PER_SEC<<" seconds";
	Sleep(100);
	
	std::cout<<"\nDestroying window.";
	Sleep(10);
	std::cout<<"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
	std::cout<<"\n>>>Main finishing.    >>>";
    std::cout<<"\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
	Sleep(100);
#ifdef WINDOWS

	// De-allocate COM stuffs
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms688715%28v=vs.85%29.aspx
//	CoUninitialize();

	/// Post debug to output window.
//	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	/// Post memory leaks
//	_CrtDumpMemoryLeaks();
	Sleep(10);
	return (int) 0;
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


