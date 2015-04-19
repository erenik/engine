/// Emil Hedemalm
/// 2014-06-13
/// AppWindow class for handling arbitrary windows in both Win32 and Linux.
/** The general procedure of creating a new AppWindow looks like the following:
	
	AppWindow * AppWindow = WindowMan.NewWindow();
	window->CreateUI();
	window->CreateGlobalUI();
	window->Create();
	window->Show();

	The first line allocates the base AppWindow class. The next two rows makes sure that there is some UI available.
	Custom UIs may be created but at least the GlobalUI must be created somehow.
	After that the OS is requested to actually create it.
	Lastly the OS is requested to show the AppWindow to the user/screen.
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
class AppWindow;
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

/// Fetches AppWindow that is currently active. Returns NULL if it is not one owned by the application.
AppWindow * ActiveWindow();
/// Fetches the main application AppWindow.
AppWindow * MainWindow();
/// AppWindow mouse is currently hovering over.
AppWindow * HoverWindow();

Align(16)
class AppWindow
{
	friend class GraphicsManager;
#ifdef WINDOWS
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
	friend class WindowManager;
	/// Default constructor, one name.
	AppWindow(String name, String displayName);
public:
	~AppWindow();	

	bool IsMain() {return main;};

	/// o-o
	Vector2i GetWindowCoordsFromScreenCoords(Vector2i screenPos);
	
	/// Updates positions, using parent as relative (if specified)
	void UpdatePosition();

	// Not sure how it works really..
	void Move(Vector2i byThisAmount);
	/// Ensures both UI and GlobalUI has been set.
	void EnsureUIIsCreated();

	/// Toggles full-screen for this AppWindow.
	void ToggleFullScreen();

	/// Sets default styles
	void SetDefaults();
	/// Creates the actual AppWindow. Returns true upon success.
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
	/// Fetches right-edge X-position of the AppWindow.
	int GetRight();
	// Moving the AppWindow using the center of the AppWindow as guide.
	void MoveCenterTo(Vector2i position);
	void Show();
	// Hides the AppWindow from user interaction! Called by default for non-main windows when closing them.
	void Hide();
	void BringToTop();

	/// Fills contents of current frame into target texture. Exactly which frame which will be sampled depends on the render-thread.
	void GetFrameContents(Texture * intoTexture);

	void SetBackgroundColor(const Vector4f & color, bool applyToViewports);

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
	/// Relative to parent AppWindow.
	void SetRequestedRelativePosition(Vector2i pos);

	// Render settings
	void RenderGrid(bool renderIt);

	int MemLeakTest();
	bool CreateGLContext();
	bool MakeGLContextCurrent();
	bool DeleteGLContext();
	
	// Reference name used when coding/working with the AppWindow.
	String name;
	// Title that should be rendered by the OS.
	String displayName;

	
	/// Disables all render-flags below
	void DisableAllRenders();

	/// AppWindow-specific render-flags.
	bool renderViewports;
	bool renderFPS;
	bool renderState;
	bool renderScene;
	bool renderUI;

#ifdef WINDOWS
	// Sets up pixel format, Win32 style.
	bool SetupPixelFormat(HDC hDC);
    // The main AppWindow class name.
    String windowClassName;
	HWND hWnd;
    // Style to be used for the AppWindow.
    DWORD windowStyle;
	// Extended styles.
	DWORD dwExStyle;
	// For painting/rendering.
	/// Device context
	HDC		hdc;			
	/// GL rendering context
	HGLRC	hglrc;		
	/// For drag-n-drop operatrions. See http://msdn.microsoft.com/en-us/library/windows/desktop/ms678405%28v=vs.85%29.aspx
	DragAndDrop * dragAndDrop;

#elif defined LINUX
	int32 xWindowHandle; // Handle in XWindowSystem, 32-bit.
	void * xGLContext;
#endif
	
	Vector4f backgroundColor;

	/// p-=p
	Viewport * MainViewport();
	/// Do NOT touch this outside of the render-thread. Add only using GMSetViewports.
	List<Viewport*> viewports;

	/// UI and GlobalUI for this AppWindow. DO NOT TOUCH OUTSIDE OF RENDER THREAD! Looking may be fine...
	UserInterface * ui, * globalUI;

	Vector2i requestedSize;
	Vector2i requestedRelativePosition;


	/** Should NOT be confused with the working/client area of the AppWindow! 
	*/
	Vector2i OSWindowSize();
	
	/// Fetches ray using viewport-based co-ordinates (not to be confused with the Window/screen-space co-ordinates!)
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
	Texture * frame;
	List<Texture*> frames;
	/// Will dictate general framerate while recording.
	int msBetweenFrames;

	// For screenshots?
	bool getNextFrame;
	Texture * frameTexture;

	/** For child windows, if true, will hide it when the user press Esc if no elements are present in the UI stack. 
		If not, only focus will shift to the main AppWindow.
		True by default.
	*/
	bool hideOnEsc;
private:

	
	/// If this AppWindow is currently the top one being viewed by the user.
	bool inFocus;
	/// Don't render to non-created windows..
	bool created;
	// Internal state
	bool isFullScreen;
	bool resizable;
	// If currently visible.
	bool visible;
	// Only one AppWindow should be set as "main". If removing this AppWindow should also close the entire application!
	bool main;
	/// Size of OS area
	Vector2i osWindowSize;
	Vector2i previousSize;
	/// Size of the client AppWindow area
	Vector2i clientAreaSize;
	// Top-left corner in windows... 
	Vector2i position;
	Vector2i previousPosition;


};

#endif // WINDOW_H