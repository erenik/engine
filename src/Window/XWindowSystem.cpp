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

int ErrorHandler(Display * d, XErrorEvent * e);
// single buffer attributes
static int singleBufferAttributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, None};
// doubble buffer attributes
static int doubleBufferAttributes[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

/// Program start-up variables!
XEvent                  event;
GLXContext              context; // OpenGL context
Display *               xDisplay; // connection to X server
XVisualInfo *           xVisualInfo;
Window                  window;
XSetWindowAttributes    window_attributes;
Colormap                colormap;
bool                    swapBuffers;

void testRender();

/// o.o
List<Window> xWindowHandles;

bool XWindowSystem::InitThreadSupport()
{
    /// Initialize support for multi-threaded usage of Xlib
    int status = XInitThreads();
    if (status){
        std::cout<<"\n*nix thread-support initialized.";
    }
    else {
        std::cout<<"\nXInitThreads failed.";
    }
    return status != 0;
}

// Connects to the X server (WindowSystem) using XOpenDisplay.
bool XWindowSystem::Initialize()
{
	xDisplay = XOpenDisplay(NULL);
	return (xDisplay != NULL);

	int dummy;
    if(!glXQueryExtension(xDisplay, &dummy, &dummy))
    {
        std::cout<<"\nERROR: XServer has no GLX extension!";
        return -2;
        assert(false && "X server has no OpenGL GLX extension");
    }

    /// Find OpenGL-capable RGB visual with depth buffer (device context?)
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
}



/// Number of attached screens.
List<DeviceScreen> XWindowSystem::Screens()
{
    int screen_count = XScreenCount(xDisplay);
    List<DeviceScreen> screens;
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
    //  XSelectInput (xDisplay, AppWindow, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
}


void XWindowSystem::CreateDefaultWindow()
{
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
    XMapWindow(xDisplay, window);
}

void XWindowSystem::SetupProtocols()
{
    /// Fix so we can intercept AppWindow-Management messages (like pressing the Close-button, ALT+F4, etc!)
    // Ref: http://www.opengl.org/discussion_boards/showthread.php/157469-Properly-destroying-a-AppWindow
    Atom wm_protocol = XInternAtom (xDisplay, "WM_PROTOCOLS", False);
    Atom wm_close = XInternAtom (xDisplay, "WM_DELETE_WINDOW", False);
    // Next we elect to receive the 'close' event from the WM:
    XSetWMProtocols (xDisplay, window, &wm_close, 1);
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
            if (XProc(event) != NULL)
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

            if(swapBuffers)
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


/* /// OLD SHIT

#ifdef LINUX

extern Display*                xDisplay; // connection to X server

xWindow::xWindow(int width, int height, int argc, char **argv)
: singleBufferAttributes {GLX_RGBA, GLX_DEPTH_SIZE, 24, None},
  doubleBufferAttributes {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None}
{
    this->width = width;
    this->height = height;
    x = 0;
    y = 0;
    this->argv = argv;
    this->argc = argc;

    x_rot = 0.0; // temporary for rotating triangle
}

void xWindow::Connect(char* display_name)
{
    xDisplay = XOpenDisplay( display_name ); // defaults to the value of the DISPLAY environment variable.
    if ( xDisplay == NULL )
    {
        assert(false && "Cant connect to X server");
    }
}
void xWindow::Visual()
{
    // check GLX extension, needed for connecting X server to OpenGL
    int dummy;
    if(!glXQueryExtension(xDisplay, &dummy, &dummy))
    {
        assert(false && "X server has no OpenGL GLX extension");
    }

    // find OpenGL-capable RGB visual with depth buffer
    visual_info = glXChooseVisual(xDisplay, DefaultScreen(xDisplay), doubleBufferAttributes);
    if (visual_info == NULL)
    {
        std::cout << "No double buffer" << std::endl;

        visual_info = glXChooseVisual(xDisplay, DefaultScreen(xDisplay), singleBufferAttributes);
        if (visual_info == NULL)
        {
            assert(false && "no depth buffer");
        }
    }
    else
    {
        swapBuffers = true;
    }
}
void xWindow::CreateWindow()
{
   // Each X AppWindow always has an associated colormap that provides a level of indirection between pixel values
    // and colors displayed on the screen. The X protocol defines colors using values in the RGB color space.
    colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, visual_info->screen), visual_info->visual, AllocNone);

    // update AppWindow attributes
    window_attributes_set.colormap = colormap;
    window_attributes_set.border_pixel = 0;
    window_attributes_set.event_mask = KeyPressMask | ExposureMask | ButtonPressMask | StructureNotifyMask | ResizeRedirectMask;

    // create the AppWindow
    AppWindow = XCreateWindow(xDisplay, RootWindow(xDisplay, visual_info->screen), x, y, width, height, 0,
                           visual_info->depth, InputOutput, visual_info->visual,
                           CWBorderPixel | CWColormap | CWEventMask, &window_attributes_set);

    // set AppWindow properties
    XSetStandardProperties(xDisplay, AppWindow, "main", None, None, argv, argc, NULL);

}
void xWindow::Context()
{
    context = glXCreateContext(xDisplay, visual_info, None, true);
    if (context == NULL)
    {
        assert(false && "could not create rendering context");
    }


    // bind the rendering context to the AppWindow
    bool bound = glXMakeContextCurrent(xDisplay, AppWindow, AppWindow, context);
    if (bound == false)
    {
        assert(false && "Failed to bind context");
    }
}

int xWindow::show()
{
    // open a connection to the X server that controls the xDisplay
    Connect( NULL );

    // find OpenGL-capable RGB visual with depth buffer
    Visual();

    // create AppWindow
    CreateWindow();

    // bind the rendering context to AppWindow
    Context();

    // xDisplay X AppWindow on screen
    XMapWindow(xDisplay, AppWindow);

    Info();

    return true;
}
void xWindow::EventHandling()
{
    // XNextEvent is required to get the OS to do anything at all... ish? o-o
    XNextEvent(xDisplay, &event);
    if(event.type == Expose)
    {
        XGetWindowAttributes(xDisplay, AppWindow, &window_attributes);
        setupGL(window_attributes.width, window_attributes.height);
        render();
    }

}
void xWindow::setupGL(int width, int height)
{
    // Setup rendering
    glEnable(GL_DEPTH_TEST); // enable depth buffering
    glDepthFunc(GL_LESS);    // pedantic, GL_LESS is the default
    glClearDepth(1.0);       // pedantic, 1.0 is the default
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 1000.0);
    glViewport(0, 0, width, height);
    glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
}
void xWindow::render()
{
    // place whats rendered here

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glTranslatef(0.0, 0.0, -2.0);
            glRotatef(x_rot, 0.0, 1.0, 0.0);
            glBegin(GL_TRIANGLES); // front side
                glColor3f (  1.0,  0.0,  0.0 ); // red
                glVertex3f(  0.0,  0.5,  0.0 );

                glColor3f (  0.0,  1.0,  0.0 ); // green
                glVertex3f( -0.5, -0.5,  0.0 );

                glColor3f (  0.0,  0.0,  1.0 ); // blue
                glVertex3f(  0.5, -0.5,  0.0 );
            glEnd();

            if(swapBuffers)
            {
                glXSwapBuffers( xDisplay, AppWindow );
            }
            else
            {
                glFlush();
            }

}


void xWindow::update()
{
    while(true)
    {
        while(XPending(xDisplay) > 0)
        {
            // handles event
            EventHandling();
        }


        // what to be rendered
        render();

        // temporay rotation speed
        x_rot += 2.0;
    }
}

/// Saves in the argument parameters
bool xWindow::GetScreenSize(int &x, int &y){
    int screen_count = XScreenCount(xDisplay);
    // go through all screens and show the info in the console
    for(int scr = 0; scr < screen_count; scr++)
    {
        int screen_height = XDisplayHeight(xDisplay, scr);
        y = screen_height;
        int screen_width = XDisplayWidth(xDisplay, scr);
        x = screen_width;
        int screen_cells = XDisplayCells(xDisplay, scr); // number of entries in the default colormap
        int screen_planes = XDisplayPlanes(xDisplay, scr); // depth of the root AppWindow of the specified screen
        char* screen_string = XDisplayString(xDisplay); // string when the current xDisplay was opened

        std::cout << "Screen " << scr << std::endl;
        std::cout << "\t " << screen_string << std::endl;
        std::cout << "\t :" << screen_width << " x " << screen_height << std::endl;
        std::cout << "\t :" << screen_cells << " entries in default colormap" << std::endl;
        std::cout << "\t :" << screen_planes << " depth" << std::endl;
    }
}

void xWindow::Info()
{
    int screen_count = XScreenCount(xDisplay);

    // go through all screens and show the info in the console
    for(int scr = 0; scr < screen_count; scr++)
    {
        int screen_height = XDisplayHeight(xDisplay, scr);
        int screen_width = XDisplayWidth(xDisplay, scr);
        int screen_cells = XDisplayCells(xDisplay, scr); // number of entries in the default colormap
        int screen_planes = XDisplayPlanes(xDisplay, scr); // depth of the root AppWindow of the specified screen
        char* screen_string = XDisplayString(xDisplay); // string when the current xDisplay was opened

        std::cout << "Screen " << scr << std::endl;
        std::cout << "\t " << screen_string << std::endl;
        std::cout << "\t :" << screen_width << " x " << screen_height << std::endl;
        std::cout << "\t :" << screen_cells << " entries in default colormap" << std::endl;
        std::cout << "\t :" << screen_planes << " depth" << std::endl;
    }

    int major_version, minor_version;
    // retrive version
    if(!glXQueryVersion(xDisplay, &major_version, &minor_version))
    {
        assert(false && "No GLX version present");
    }
    std::cout << "GLX" << std::endl;
    std::cout << "\tMajor :\t" << major_version << std::endl;
    std::cout << "\tMinor :\t" << minor_version << std::endl;

    // initializing glew
    GLenum err = glewInit();

    // check if glew is ok
    if (GLEW_OK != err)
    {
        assert(false && "Error: " && glewGetErrorString(err));
    }
    std::cout << "GLEW version" << std::endl;
    std::cout << "\t" << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "GL version" << std::endl;
    std::cout << "\t" << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version" << std::endl;
    std::cout << "\t" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

*/


