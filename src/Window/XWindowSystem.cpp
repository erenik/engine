/// Emil Hedemalm
/// 2015-04-18
/// Handler for communicating with the X AppWindow system,.. due to all particularities and diffuse inclusion files and weird definition-hoggings..

#include "Window/WindowSystem.h"

#ifdef USE_X11

// Force-skip inclusion of Time-class, as it clashes with X11 system.
#include "AppWindow.h"
#undef Time

#include "XWindowSystem.h"

#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <GL/glx.h>     // connect X server with OpenGL
#include "XProc.h"      // XWindow Event Processor

#include "File/LogFile.h"

int ErrorHandler(Display * d, XErrorEvent * e);
// single buffer attributes
static int singleBufferAttributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, None};
// doubble buffer window_attributes
static int doubleBufferAttributes[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

/// Program start-up variables!
XEvent event;
GLXContext              context; // OpenGL context
Display *               xDisplay = NULL; // connection to X server
XVisualInfo *           xVisualInfo = NULL;
Window                  window;
XSetWindowAttributes    xWindowAttributes;
Colormap                colormap;
bool XWindowSystem::swapBuffers = false;

void testRender();

/// o.o
List<Window> xWindowHandles;

bool XWindowSystem::InitThreadSupport()
{
    std::cout<<"\nXWindowSystem::InitThreadSupport";
    /// Initialize support for multi-threaded usage of Xlib
    int status = XInitThreads();
    if (status){
        LogMain("XInitThreads - multi-threading enabled.", INFO);
        std::cout<<"\n*nix thread-support initialized.";
    }
    else {
        assert(false);
        std::cout<<"\nXInitThreads failed.";
    }
    return status != 0;
}

// Connects to the X server (WindowSystem) using XOpenDisplay.
bool XWindowSystem::Initialize()
{
    LogMain("Initializiting XWindowSystem", INFO);
    assert(XWindowSystem::InitThreadSupport());
    LogMain("XOpenDisplay", INFO);
	xDisplay = XOpenDisplay(NULL);
    if (xDisplay == NULL)
    	return false;

  //  visual = DefaultVisual(xDisplay, 0);
  //  depth  = DefaultDepth(xDisplay, 0);
    XSetWindowAttributes    frame_attributes;
    frame_attributes.background_pixel = XWhitePixel(xDisplay, 0);
   

	int dummy;
    if(!glXQueryExtension(xDisplay, &dummy, &dummy))
    {
        std::cout<<"\nERROR: XServer has no GLX extension!";
        return -2;
        assert(false && "X server has no OpenGL GLX extension");
    }

    /// Find OpenGL-capable RGB visual with depth buffer (device context?)
    LogMain("glXChooseVisual", INFO);
    xVisualInfo = glXChooseVisual(xDisplay, DefaultScreen(xDisplay), doubleBufferAttributes);
    if (xVisualInfo == NULL){
        std::cout << "No double buffer" << std::endl;

        xVisualInfo = glXChooseVisual(xDisplay, DefaultScreen(xDisplay), singleBufferAttributes);
        if (xVisualInfo == NULL){
            assert(false && "no depth buffer");
        }
    }
    else {
        swapBuffers = true;
    }

    /// Set Error handler!
    XSetErrorHandler(ErrorHandler);
}

/// Closes connection to the X server.
bool XWindowSystem::Shutdown()
{    
    XCloseDisplay(xDisplay);
    /// Free allocated resources.
    if (xVisualInfo)
        XFree(xVisualInfo);
}



/// Number of attached screens.
List<DeviceScreen> XWindowSystem::Screens()
{
    List<DeviceScreen> screens;
    if (xDisplay == 0)
        return screens;
    int screen_count = XScreenCount(xDisplay);
    // go through all screens and show the info in the console
    for(int scr = 0; scr < screen_count; scr++)
    {
        DeviceScreen screen;
        int screen_height = XDisplayHeight(xDisplay, scr);
        screen.size.y = screen_height;
        int screen_width = XDisplayWidth(xDisplay, scr);
        screen.size.x = screen_width;
        
        int screen_cells = XDisplayCells(xDisplay, scr); // number of entries in the default colormap
        int screen_planes = XDisplayPlanes(xDisplay, scr); // depth of the root AppWindow of the specified screen
        char* screen_string = XDisplayString(xDisplay); // string when the current xDisplay was opened


        std::cout << "Screen " << scr << std::endl;
        std::cout << "\t " << screen_string << std::endl;
        std::cout << "\t :" << screen_width << " x " << screen_height << std::endl;
        std::cout << "\t :" << screen_cells << " entries in default colormap" << std::endl;
        std::cout << "\t :" << screen_planes << " depth" << std::endl;
        screens.AddItem(screen);
    }
    return screens;
}


void XWindowSystem::SetupDefaultWindowProperties()
{
    // Each X AppWindow always has an associated colormap that provides a level of indirection between pixel values
    // and colors displayed on the screen. The X protocol defines colors using values in the RGB color space.
    colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, xVisualInfo->screen), xVisualInfo->visual, AllocNone);

    // update AppWindow attributes
    xWindowAttributes.colormap = colormap;
    xWindowAttributes.border_pixel = 0;
    xWindowAttributes.event_mask =

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
    //  XSelectInput (xDisplay, AppWindow, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
}


void XWindowSystem::CreateDefaultWindow()
{
    assert(false);
    /*
    window = XCreateWindow(xDisplay,
                           RootWindow(xDisplay, xVisualInfo->screen),
                           0, 0,            /// Position
                           800, 600,   /// Size
                           0,
                           xVisualInfo->depth,
                           InputOutput,
                           xVisualInfo->visual,
                           CWBorderPixel | CWColormap | CWEventMask,
                           &window_attributes);
    // set AppWindow properties
    XSetStandardProperties(xDisplay, window, "main", None, None, NULL, 0, NULL);
    // Should be replaced with XSetWMProperties, according to the specification..

/*
    // bind the rendering context to the AppWindow
    bool bound = glXMakeContextCurrent(xDisplay, AppWindow, AppWindow, context);
    if (bound == false)
    {
        assert(false && "Failed to bind context");
    }
*/
    // xDisplay X AppWindow on screen
   // XMapWindow(xDisplay, window);

}

void XWindowSystem::SetupProtocols()
{
    assert(false);
    /*
    /// Fix so we can intercept AppWindow-Management messages (like pressing the Close-button, ALT+F4, etc!)
    // Ref: http://www.opengl.org/discussion_boards/showthread.php/157469-Properly-destroying-a-AppWindow
    Atom wm_protocol = XInternAtom (xDisplay, "WM_PROTOCOLS", False);
    Atom wm_close = XInternAtom (xDisplay, "WM_DELETE_WINDOW", False);
    // Next we elect to receive the 'close' event from the WM:
    XSetWMProtocols (xDisplay, window, &wm_close, 1);
    */
}

void XWindowSystem::Resize(AppWindow * window, Vector2i newSize)
{
    // Fetch the XWindow equivalent?
    XResizeWindow(xDisplay, window->xWindowHandle, newSize.x, newSize.y);
}

void XWindowSystem::ToggleFullScreen(AppWindow * window)
{
    assert(false && "Implement");
//    XResizeWindow(xDisplay, xWindowHandle, Graphics.ScreenWidth(), Graphics.ScreenHeight());
}

bool XWindowSystem::CreateGLContext(AppWindow * forWindow)
{

    return true;
}

bool XWindowSystem::DestroyGLContext(AppWindow * forWindow)
{
    glXMakeContextCurrent(xDisplay, None, None, NULL); // Release context.
    glXDestroyContext(xDisplay, context);
    return true;
}

#include "StateManager.h"
#include "AppStates/AppStates.h"
#include "OS/Sleep.h"

/// X11 message loop!
void XWindowSystem::MainLoop()
{
    std::cout<<"\nBeginning listening to events...";
   // XNoOp(xDisplay);
    int messagesReceived = 0;
    while(StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING){
        // Check for queued messages.
        int events = XPending(xDisplay);
        /// XEventsQueued(xDisplay, QueuedAfterReading);
      //  if (events > 0)
        //    std::cout<<"\nQueued events: "<<events;
        if (events){
            // XNextEvent may block until an event appears, which might not be wanted, to check beforehand how many events are available!
            XNextEvent(xDisplay, &event);
            if (XProc() != NULL)
                break;
        }
        else {
            // If no messages, allow some sleep?
            SleepThread(10);
         //   std::cout<<"\nSleeping";
        }
    }
}

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
    glViewport(0, 0, 800, 600);


    float x_rot = 0.0;

    // Rendering
    while(true)
    {
            // XNextEvent is required to get the OS to do anything at all... ish? o-o
       //     XNextEvent(xDisplay, &event);
            SleepThread(10);


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

            if(XWindowSystem::swapBuffers)
            {
                glXSwapBuffers( xDisplay, window );
            }
            else
            {
                glFlush();
            }
    }
};

#endif // USE_X11


