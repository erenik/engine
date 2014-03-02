// Fredrik Larsson && Emil Hedemalm
// 2013-07-03 Linuxifixation!

#include "Window.h"

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
   // Each X window always has an associated colormap that provides a level of indirection between pixel values
    // and colors displayed on the screen. The X protocol defines colors using values in the RGB color space.
    colormap = XCreateColormap(display, RootWindow(display, visual_info->screen), visual_info->visual, AllocNone);

    // update window attributes
    window_attributes_set.colormap = colormap;
    window_attributes_set.border_pixel = 0;
    window_attributes_set.event_mask = KeyPressMask | ExposureMask | ButtonPressMask | StructureNotifyMask | ResizeRedirectMask;

    // create the window
    window = XCreateWindow(display, RootWindow(display, visual_info->screen), x, y, width, height, 0,
                           visual_info->depth, InputOutput, visual_info->visual,
                           CWBorderPixel | CWColormap | CWEventMask, &window_attributes_set);

    // set window properties
    XSetStandardProperties(display, window, "main", None, None, argv, argc, NULL);

}
void xWindow::Context()
{
    context = glXCreateContext(display, visual_info, None, true);
    if (context == NULL)
    {
        assert(false && "could not create rendering context");
    }


    // bind the rendering context to the window
    bool bound = glXMakeContextCurrent(display, window, window, context);
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

    // create window
    CreateWindow();

    // bind the rendering context to window
    Context();

    // display X window on screen
    XMapWindow(display, window);

    Info();

    return true;
}
void xWindow::EventHandling()
{
    // XNextEvent is required to get the OS to do anything at all... ish? o-o
    XNextEvent(display, &event);
    if(event.type == Expose)
    {
        XGetWindowAttributes(display, window, &window_attributes);
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
                glXSwapBuffers( display, window );
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
        int screen_planes = XDisplayPlanes(display, scr); // depth of the root window of the specified screen
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
        int screen_planes = XDisplayPlanes(display, scr); // depth of the root window of the specified screen
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


#endif
