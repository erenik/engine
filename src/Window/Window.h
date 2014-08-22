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
#include "Time/Time.h"

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
class Ray;
class DragAndDrop;

struct Monitor 
{
	Monitor();
	/// Center position of this monitor.
	Vector2i center;
	Vector2i size;
	Vector2i topLeftCorner;
#ifdef WINDOWS
	MONITORINFOEX monitorInfo;
	HMONITOR hMonitor;
#endif
};

List<Monitor> GetMonitors();

/// Fetches window that is currently active. Returns NULL if it is not one owned by the application.
Window * ActiveWindow();
/// Fetches the main application window.
Window * MainWindow();
/// Window mouse is currently hovering over.
Window * HoverWindow();

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

	bool IsMain() {return main;};

	/// o-o
	Vector2i GetWindowCoordsFromScreenCoords(Vector2i screenPos);
	
	/// Updates positions, using parent as relative (if specified)
	void UpdatePosition();

	// Not sure how it works really..
	void Move(Vector2i byThisAmount);
	/// Ensures both UI and GlobalUI has been set.
	void EnsureUIIsCreated();

	/// Toggles full-screen for this window.
	void ToggleFullScreen();

	/// Sets default styles
	void SetDefaults();
	/// Creates the actual window. Returns true upon success.
	bool Create();
	/// Must be called from the same thread that created it (on Windows).
	bool Destroy();
	bool IsVisible();
	bool IsFullScreen() { return isFullScreen; };
	void MoveToMonitor(int monitorIndex);

	/// Returns the area available for application-specific rendering. 
#define WorkingArea ClientAreaSize
	Vector2i ClientAreaSize();
	Vector2i GetTopLeftCorner();
	/// Fetches right-edge X-position of the window.
	int GetRight();
	// Moving the window using the center of the window as guide.
	void MoveCenterTo(Vector2i position);
	void Show();
	// Hides the window from user interaction! Called by default for non-main windows when closing them.
	void Hide();
	void BringToTop();

	/// Fills contents of current frame into target texture. Exactly which frame which will be sampled depends on the render-thread.
	void GetFrameContents(Texture * intoTexture);

	/// Fetches the UI which is displayed on top of everything else, used for fade-effects etc. Is created dynamically if not set earlier.
	UserInterface * GetUI();
	/// Fetches the global (system) UI.
	UserInterface * GetGlobalUI(bool fromRenderThread = false);
	/// Should be called after creation.
	UserInterface * CreateUI();
	/// Should be called after creation.
	UserInterface * CreateGlobalUI();

	/// Requested size.
	void SetRequestedSize(Vector2i size);
	/// Relative to parent window.
	void SetRequestedRelativePosition(Vector2i pos);

	int MemLeakTest();
	bool CreateGLContext();
	bool MakeGLContextCurrent();
	bool DeleteGLContext();
	
	// Reference name used when coding/working with the window.
	String name;
	// Title that should be rendered by the OS.
	String title;

	
	/// Disables all render-flags below
	void DisableAllRenders();

	/// Window-specific render-flags.
	bool renderViewports;
	bool renderFPS;
	bool renderState;
	bool renderScene;
	bool renderUI;

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

	/// For drag-n-drop operatrions. See http://msdn.microsoft.com/en-us/library/windows/desktop/ms678405%28v=vs.85%29.aspx
	DragAndDrop * dad;

#endif
	
	Vector4f backgroundColor;

	/// p-=p
	Viewport * MainViewport();
	/// Do NOT touch this outside of the render-thread. Add only using GMSetViewports.
	List<Viewport*> viewports;

	/// UI and GlobalUI for this window. DO NOT TOUCH OUTSIDE OF RENDER THREAD! Looking may be fine...
	UserInterface * ui, * globalUI;

	Vector2i requestedSize;
	Vector2i requestedRelativePosition;


	/** Should NOT be confused with the working/client area of the window! 
	*/
	Vector2i OSWindowSize();
	
	/// Fetches ray using viewport-based co-ordinates (not to be confused with the window/screen-space co-ordinates!)
#define GetRayFromWindowCoordinates GetRayFromScreenCoordinates
	bool GetRayFromScreenCoordinates(int mouseX, int mouseY, Ray & ray);
	bool GetRayFromScreenCoordinates(Vector2i screenCoords, Ray & ray);

	/// All below should only be touched from within the render-thread.
	/// When true, will save the next rendered frame's contents as a screenshot to standard screenshot output directory (./output/screenshots/)
	bool saveScreenshot;
	/// Query to record video.
	bool recordVideo;
	/// Will be true when recording.
	bool isRecording;
	/// When capture started.
	Time captureStart;
	/// Frames stored while recording.
	List<Texture*> frames;
	/// Will dictate general framerate while recording.
	int msBetweenFrames;

	// For screenshots?
	bool getNextFrame;
	Texture * frameTexture;

private:

	
	/// If this window is currently the top one being viewed by the user.
	bool inFocus;
	/// Don't render to non-created windows..
	bool created;
	// Internal state
	bool isFullScreen;
	bool resizable;
	// If currently visible.
	bool visible;
	// Only one window should be set as "main". If removing this window should also close the entire application!
	bool main;
	/// Size of OS area
	Vector2i osWindowSize;
	Vector2i previousSize;
	/// Size of the client window area
	Vector2i clientAreaSize;
	// Top-left corner in windows... 
	Vector2i position;
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