/// Emil Hedemalm
/// 2014-06-13
/// Window class for handling arbitrary windows in both Win32 and Linux.
/** The general procedure of creating a new window looks like the following:
	
	Window * window = WindowMan.NewWindow();
	window->CreateUI();
	window->CreateGlobalUI();
	window->Create();
	window->Show();

	The first line allocates the base window class. The next two rows makes sure that there is some UI available.
	Custom UIs may be created but at least the GlobalUI must be created somehow.
	After that the OS is requested to actually create it.
	Lastly the OS is requested to show the window to the user/screen.
*/

// Fredrik Larsson && Emil Hedemalm
// 2013-07-03 Linuxifixation!

#ifndef WINDOW_H
#define WINDOW_H

#include "OS.h"
#include "String/AEString.h"
#include "MathLib.h"

#ifdef WINDOWS
#include "Windows.h"
#include "WndProc.h"
#endif

// Some stuff, dunno.
#define DEFAULT_WINDOW_WIDTH    800
#define DEFAULT_WINDOW_HEIGHT   600

class Viewport;
class UserInterface;
class Window;
class Texture;

/// Fetches window that is currently active. Returns NULL if it is not one owned by the application.
Window * ActiveWindow();
/// Fetches the main application window.
Window * MainWindow();


class Window 
{
	friend class GraphicsManager;
#ifdef WINDOWS
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
	friend class WindowManager;
	Window(String name);
public:
	~Window();	

	/// Ensures both UI and GlobalUI has been set.
	void EnsureUIIsCreated();

	/// Toggles full-screen for this window.
	void ToggleFullScreen();

	/// Sets default styles
	void SetDefaults();
	/// Creates the actual window. Returns true upon success.
	bool Create();
	bool IsVisible();
	void Show();
	// Hides the window from user interaction! Called by default for non-main windows when closing them.
	void Hide();
	void BringToTop();

	/// Fills contents of current frame into target texture. Exactly which frame which will be sampled depends on the render-thread.
	void GetFrameContents(Texture * intoTexture);

	/// Fetches the UI which is displayed on top of everything else, used for fade-effects etc. Is created dynamically if not set earlier.
	UserInterface * GetUI();
	/// Fetches the global (system) UI.
	UserInterface * GetGlobalUI();
	/// Should be called after creation.
	UserInterface * CreateUI();
	/// Should be called after creation.
	UserInterface * CreateGlobalUI();

	/// Requested size.
	void SetRequestedSize(Vector2i size);
	/// Relative to parent window.
	void SetRequestedRelativePosition(Vector2i pos);

	bool CreateGLContext();
	bool MakeGLContextCurrent();
	bool DeleteGLContext();
	
	// Reference name used when coding/working with the window.
	String name;
	// Title that should be rendered by the OS.
	String title;

	/// Window-specific render-flags.
	bool renderViewports;

#ifdef WINDOWS
	// Sets up pixel format, Win32 style.
	bool SetupPixelFormat(HDC hDC);

    // The main window class name.
    String windowClassName;
	HWND hWnd;
    // Style to be used for the window.
    DWORD windowStyle;
	// Extended styles.
	DWORD dwExStyle;
	// For painting/rendering.
	/// Device context
	HDC		hdc;			
	/// GL rendering context
	HGLRC	hglrc;		
#endif
	
	// Returns size in pixels.
	Vector2i Size() {return size;};

	Vector4f backgroundColor;

	/// p-=p
	Viewport * MainViewport();
	/// Do NOT touch this outside of the render-thread. Add only using GMSetViewports.
	List<Viewport*> viewports;

	/// UI and GlobalUI for this window. DO NOT TOUCH OUTSIDE OF RENDER THREAD! Looking may be fine...
	UserInterface * ui, * globalUI;

	Vector2i requestedSize;
	Vector2i requestedRelativePosition;

private:


	bool getNextFrame;
	Texture * frameTexture;

	/// If this window is currently the top one being viewed by the user.
	bool inFocus;
	/// Don't render to non-created windows..
	bool created;
	// Internal state
	bool isFullScreen;
	bool resizable;
	// Only one window should be set as "main". If removing this window should also close the entire application!
	bool main;
	Vector2i size;
	Vector2i previousSize;
	// Top-left corner in windows... 
	Vector2i previousPosition;


};


#ifdef LINUX

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
        Window                  window;
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

        /// create the window and it attributes
        void CreateWindow();

        /// connect the OpenGL context to the window
        void Context();

        /// handle all types of events
        void EventHandling();

        /// the object thats going to be rendered
        void render();

        /// show glew, gl, glsl glx version in console
        void Info();

};

#endif // LINUX


#endif // WINDOW_H