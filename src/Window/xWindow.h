/// Emil Hedemalm
/// 2015-04-18
/// Linux-specific shit

#include "OS/OS.h"

#ifdef LINUX
#undef Time

#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <GL/glx.h>     // connect X server with OpenGL

#include <assert.h>
#include <iostream>

class xWindow
{
    public:
        xWindow(int width, int height, int argc=NULL, char **argv=NULL );

        int show();

        void setupGL(int width, int height);

        /// update
        void update();


        /// single buffer attributes
        int singleBufferAttributes[10];
        /// doubble buffer attributes
        int doubleBufferAttributes[10];

        /// Saves in the argument parameters
        static bool GetScreenSize(int &x, int &y);

    private:
        char**                  argv;
        int                     argc;

        XEvent                  event;
        XVisualInfo*            visual_info;
        GLXContext              context; // OpenGL context
        AppWindow                  AppWindow;
        XSetWindowAttributes    window_attributes_set;
        XWindowAttributes       window_attributes;
        Colormap                colormap;
        bool                    swapBuffers;

        int                     width;
        int                     height;
        int                     x;
        int                     y;

        float                   x_rot;

        bool                    show_info = true;

        /// open a connection to the X server that controls the display
        void Connect(char* display_name);

        /// find OpenGL-capable RGB visual with depth buffer
        void Visual();

        /// create the AppWindow and it attributes
        void CreateWindow();

        /// connect the OpenGL context to the AppWindow
        void Context();

        /// handle all types of events
        void EventHandling();

        /// the object thats going to be rendered
        void render();

        /// show glew, gl, glsl glx version in console
        void Info();

};

#endif // LINUX

