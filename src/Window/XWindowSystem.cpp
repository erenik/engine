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
Display *                display; // connection to X server
XVisualInfo*            visual_info;
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
	display = XOpenDisplay(NULL);
	return (display != NULL);

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

}


/// Number of attached screens.
List<DeviceScreen> XWindowSystem::Screens()
{
    int screen_count = XScreenCount(display);
    List<DeviceScreen> screens;
    // go through all screens and show the info in the console
    for(int scr = 0; scr < screen_count; scr++)
    {
        DeviceScreen screen;
        int screen_height = XDisplayHeight(display, scr);
        screen.size.y = screen_height;
        int screen_width = XDisplayWidth(display, scr);
        screen.size.x = screen_width;
        
        int screen_cells = XDisplayCells(display, scr); // number of entries in the default colormap
        int screen_planes = XDisplayPlanes(display, scr); // depth of the root AppWindow of the specified screen
        char* screen_string = XDisplayString(display); // string when the current display was opened


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
    colormap = XCreateColormap(display, RootWindow(display, visual_info->screen), visual_info->visual, AllocNone);

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
    //  XSelectInput (display, AppWindow, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask);
}


void XWindowSystem::CreateDefaultWindow()
{
    window = XCreateWindow(display,
                           RootWindow(display, visual_info->screen),
                           0, 0,            /// Position
                           800, 600,   /// Size
                           0,
                           visual_info->depth,
                           InputOutput,
                           visual_info->visual,
                           CWBorderPixel | CWColormap | CWEventMask,
                           &window_attributes);
    // set AppWindow properties
    XSetStandardProperties(display, window, "main", None, None, NULL, 0, NULL);
    // Should be replaced with XSetWMProperties, according to the specification..

/*
    // bind the rendering context to the AppWindow
    bool bound = glXMakeContextCurrent(display, AppWindow, AppWindow, context);
    if (bound == false)
    {
        assert(false && "Failed to bind context");
    }
*/
    // display X AppWindow on screen
    XMapWindow(display, window);
}

void XWindowSystem::SetupProtocols()
{
    /// Fix so we can intercept AppWindow-Management messages (like pressing the Close-button, ALT+F4, etc!)
    // Ref: http://www.opengl.org/discussion_boards/showthread.php/157469-Properly-destroying-a-AppWindow
    Atom wm_protocol = XInternAtom (display, "WM_PROTOCOLS", False);
    Atom wm_close = XInternAtom (display, "WM_DELETE_WINDOW", False);
    // Next we elect to receive the 'close' event from the WM:
    XSetWMProtocols (display, window, &wm_close, 1);
}

void XWindowSystem::Resize(AppWindow * window, Vector2i newSize)
{
    // Fetch the XWindow equivalent?
    XResizeWindow(display, window->xWindowHandle, newSize.x, newSize.y);
}

void XWindowSystem::ToggleFullScreen(AppWindow * window)
{
    assert(false && "Implement");
//    XResizeWindow(display, xWindowHandle, Graphics.ScreenWidth(), Graphics.ScreenHeight());
}

#include "StateManager.h"
#include "AppStates/AppStates.h"
#include "OS/Sleep.h"

/// X11 message loop!
void XWindowSystem::MainLoop()
{
    std::cout<<"\nBeginning listening to events...";
   // XNoOp(display);
    int messagesReceived = 0;
    while(StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING){
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
       //     XNextEvent(display, &event);
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
                glXSwapBuffers( display, window );
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

extern Display*                display; // connection to X server

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
    display = XOpenDisplay( display_name ); // defaults to the value of the DISPLAY environment variable.
    if ( display == NULL )
    {
        assert(false && "Cant connect to X server");
    }
}
void xWindow::Visual()
{
    // check GLX extension, needed for connecting X server to OpenGL
    int dummy;
    if(!glXQueryExtension(display, &dummy, &dummy))
    {
        assert(false && "X server has no OpenGL GLX extension");
    }

    // find OpenGL-capable RGB visual with depth buffer
    visual_info = glXChooseVisual(display, DefaultScreen(display), doubleBufferAttributes);
    if (visual_info == NULL)
    {
        std::cout << "No double buffer" << std::endl;

        visual_info = glXChooseVisual(display, DefaultScreen(display), singleBufferAttributes);
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
    colormap = XCreateColormap(display, RootWindow(display, visual_info->screen), visual_info->visual, AllocNone);

    // update AppWindow attributes
    window_attributes_set.colormap = colormap;
    window_attributes_set.border_pixel = 0;
    window_attributes_set.event_mask = KeyPressMask | ExposureMask | ButtonPressMask | StructureNotifyMask | ResizeRedirectMask;

    // create the AppWindow
    AppWindow = XCreateWindow(display, RootWindow(display, visual_info->screen), x, y, width, height, 0,
                           visual_info->depth, InputOutput, visual_info->visual,
                           CWBorderPixel | CWColormap | CWEventMask, &window_attributes_set);

    // set AppWindow properties
    XSetStandardProperties(display, AppWindow, "main", None, None, argv, argc, NULL);

}
void xWindow::Context()
{
    context = glXCreateContext(display, visual_info, None, true);
    if (context == NULL)
    {
        assert(false && "could not create rendering context");
    }


    // bind the rendering context to the AppWindow
    bool bound = glXMakeContextCurrent(display, AppWindow, AppWindow, context);
    if (bound == false)
    {
        assert(false && "Failed to bind context");
    }
}

int xWindow::show()
{
    // open a connection to the X server that controls the display
    Connect( NULL );

    // find OpenGL-capable RGB visual with depth buffer
    Visual();

    // create AppWindow
    CreateWindow();

    // bind the rendering context to AppWindow
    Context();

    // display X AppWindow on screen
    XMapWindow(display, AppWindow);

    Info();

    return true;
}
void xWindow::EventHandling()
{
    // XNextEvent is required to get the OS to do anything at all... ish? o-o
    XNextEvent(display, &event);
    if(event.type == Expose)
    {
        XGetWindowAttributes(display, AppWindow, &window_attributes);
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
                glXSwapBuffers( display, AppWindow );
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
        while(XPending(display) > 0)
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
    int screen_count = XScreenCount(display);
    // go through all screens and show the info in the console
    for(int scr = 0; scr < screen_count; scr++)
    {
        int screen_height = XDisplayHeight(display, scr);
        y = screen_height;
        int screen_width = XDisplayWidth(display, scr);
        x = screen_width;
        int screen_cells = XDisplayCells(display, scr); // number of entries in the default colormap
        int screen_planes = XDisplayPlanes(display, scr); // depth of the root AppWindow of the specified screen
        char* screen_string = XDisplayString(display); // string when the current display was opened

        std::cout << "Screen " << scr << std::endl;
        std::cout << "\t " << screen_string << std::endl;
        std::cout << "\t :" << screen_width << " x " << screen_height << std::endl;
        std::cout << "\t :" << screen_cells << " entries in default colormap" << std::endl;
        std::cout << "\t :" << screen_planes << " depth" << std::endl;
    }
}

void xWindow::Info()
{
    int screen_count = XScreenCount(display);

    // go through all screens and show the info in the console
    for(int scr = 0; scr < screen_count; scr++)
    {
        int screen_height = XDisplayHeight(display, scr);
        int screen_width = XDisplayWidth(display, scr);
        int screen_cells = XDisplayCells(display, scr); // number of entries in the default colormap
        int screen_planes = XDisplayPlanes(display, scr); // depth of the root AppWindow of the specified screen
        char* screen_string = XDisplayString(display); // string when the current display was opened

        std::cout << "Screen " << scr << std::endl;
        std::cout << "\t " << screen_string << std::endl;
        std::cout << "\t :" << screen_width << " x " << screen_height << std::endl;
        std::cout << "\t :" << screen_cells << " entries in default colormap" << std::endl;
        std::cout << "\t :" << screen_planes << " depth" << std::endl;
    }

    int major_version, minor_version;
    // retrive version
    if(!glXQueryVersion(display, &major_version, &minor_version))
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


