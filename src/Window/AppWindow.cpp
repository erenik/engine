// Emil Hedemalm && Fredrik Larsson
// 2015-05-20 (2013-07-03)

#include "OS/Sleep.h"
#include "AppWindow.h"
#include "Application/Application.h"
#include "AppWindowManager.h"
#include "Viewport.h"
#include "UI/UserInterface.h"
#include "Graphics/GLBuffers.h"
#include "Debug.h"

#include "DragAndDrop.h"
#include "String/StringUtil.h"

#include "WindowSystem.h"
#include "Graphics/OpenGL.h"
#include "File/LogFile.h"
#undef Time

#ifdef USE_X11
	#include "XWindowSystem.h"
	#include <GL/glx.h>
	extern Display * xDisplay; // main X-server communication line.
	extern XVisualInfo * xVisualInfo;
	extern XSetWindowAttributes    xWindowAttributes;
#endif

#include "Message/MessageManager.h"

/// If true, UI may be created automatically when pushing UI elements to a window currently lacking UI. Default true.
bool AppWindow::createUIAutomatically = true;

/// List of active monitors.
List<Monitor> monitors;

/// Constructorrr
Monitor::Monitor()
{
#ifdef WINDOWS
	this->monitorInfo.cbSize = sizeof(MONITORINFOEX);
#endif
}

#ifdef WINDOWS
int CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lpRect, LPARAM lParam)
{
	Monitor monitor;
	monitor.hMonitor = hMonitor;

	BOOL result = GetMonitorInfoW(hMonitor, &monitor.monitorInfo);
	assert(result);
	MONITORINFOEX & info = monitor.monitorInfo;
	monitor.topLeftCorner = Vector2i(info.rcMonitor.left, info.rcMonitor.top);
	monitor.size = Vector2i(info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top).AbsoluteValues();
	monitor.center = monitor.size * 0.5f + monitor.topLeftCorner;
	monitors.Add(monitor);
	std::cout<<"Blubb";
	// Continue enumeration.
	return true;
}
#endif

List<Monitor> GetMonitors()
{
#ifdef WINDOWS

	monitors.Clear();

	// http://msdn.microsoft.com/en-us/library/dd162610%28v=vs.85%29.aspx	
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);

	return monitors;
#endif
}

/// Fetches AppWindow that is currently active. Returns NULL if it is not one owned by the application.
AppWindow * ActiveWindow()
{
	return WindowMan.GetCurrentlyActiveWindow();	
}
/// Fetches the main application AppWindow.
AppWindow * MainWindow()
{
	return WindowMan.MainWindow();
}
/// AppWindow mouse is currently hovering over.
AppWindow * HoverWindow()
{
	return WindowMan.HoverWindow();
}


AppWindow::AppWindow(String name, String displayName)
	: name(name), displayName(displayName)
{	
	resizable = true;
	isFullScreen = false;
	main = false;
	globalUI = ui = NULL;
	inFocus = false;
	getNextFrame = false;
	frameTexture = NULL;

	visible = false;

	renderViewports = true;
	renderFPS = false;
	renderState = true;
	renderScene = true;
	renderUI = true;
	swapBuffers = false;

	backgroundColor = Vector4f(0.5f,.5f,.5f,1);

#ifdef WINDOWS
	hWnd = NULL;
	hdc = NULL;
	hglrc = NULL;

	windowStyle = 0;
	dwExStyle = 0;
	dragAndDrop = NULL;
#elif defined LINUX
	xWindowHandle = 0;
	xGLContext = 0;
	xGLWindow = 0;
#endif

	created = false;
	saveScreenshot = false;
	/// 50 ms, 20 fps recording speed.
	isRecording = false;
	recordVideo = false;
	msBetweenFrames = 50;

	hideOnEsc = true;

	frame = NULL;
}

AppWindow::~AppWindow()
{
/// Destroy AppWindow o/o
#ifdef WINDOWS
	if (dragAndDrop)
		delete dragAndDrop;	
#elif defined USE_X11
    XDestroyWindow(xDisplay, xWindowHandle);
#endif

	/// Probably dynamically allocated, yeah.
	viewports.ClearAndDelete();
}

/// o-o
Vector2i AppWindow::GetWindowCoordsFromScreenCoords(Vector2i screenPos)
{
#ifdef WINDOWS
	POINT pt;
	pt.x = screenPos[0];
	pt.y = screenPos[1];
	BOOL ok = ScreenToClient(hWnd, &pt);
	if (!ok)
		return Vector2i(-1,-1);
	// Invert Y because windows calculates the Y coordinate inversly from what we do.
	return Vector2i(pt.x, clientAreaSize[1] - pt.y);
#endif
	assert(false && "Implement");
	return Vector2i(0,0);
}

/// Updates positions, using parent as relative (if specified)
void AppWindow::UpdatePosition()
{
#ifdef WINDOWS
	if (requestedRelativePosition[0] || requestedRelativePosition[1] )
	{	
		RECT parentRect;
		BOOL success = GetWindowRect(MainWindow()->hWnd, &parentRect);
		position = Vector2i(requestedRelativePosition);
		position[0] += parentRect.left;
		position[1] += parentRect.top;
	}
	MoveWindow(hWnd, position[0], position[1], osWindowSize[0], osWindowSize[1], true);
#else
	assert(false);
#endif
}

void AppWindow::Move(Vector2i byThisAmount)
{
	// get current pos?
	Vector2i currentPos = this->position;
	Vector2i newPos = currentPos + byThisAmount;
	Vector2i size = this->OSWindowSize();
#ifdef WINDOWS
	MoveWindow(hWnd, newPos[0], newPos[1], size[0], size[1], true); 
#endif
}

/// Ensures both UI and GlobalUI has been set.
void AppWindow::EnsureUIIsCreated()
{
	assert(ui);
	if (!ui)
	{
		std::cout<<"\nNo UI! Creating it for you.";
		CreateUI();
	}
	assert(globalUI);
	if (!globalUI)
	{
		std::cout<<"\nNo global UI! Creating it for you.";
		CreateGlobalUI();
	}
}

Vector2i AppWindow::GetMonitorSize() {
	Vector2i size;
#ifdef WINDOWS
	// First extract screen to full-size in.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd145064%28v=vs.85%29.aspx
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX lpmi;
	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd144901%28v=vs.85%29.aspx
	lpmi.cbSize = sizeof(MONITORINFOEX);
	BOOL getMonitorInfoResult = GetMonitorInfoW(hMonitor, &lpmi);
	if (getMonitorInfoResult)
	{
		size[0] = lpmi.rcMonitor.right - lpmi.rcMonitor.left;
		size[1] = lpmi.rcMonitor.bottom - lpmi.rcMonitor.top;
		bool doOffset = false;
		if (doOffset)
		{
			size += Vector2i(4, 4);
		}
	}
#endif 
	return size;
}

/// Toggles full-screen for this AppWindow.
void AppWindow::ToggleFullScreen()
{
	// Check full-screen state
	if (isFullScreen)
	{
	    LogMain("Setting previous window size: "+VectorString(previousSize), DEBUG);

	    /// If was full-screen, go back to previous-size!
#ifdef WINDOWS
		/// Set AppWindow style
		windowStyle =
			WS_CAPTION |		// The AppWindow has a title bar (includes the WS_BORDER style).
			WS_MAXIMIZEBOX |	// The AppWindow has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
			WS_MINIMIZEBOX |	// The AppWindow has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
			WS_OVERLAPPED |		// The AppWindow is an overlapped AppWindow. An overlapped AppWindow has a title bar and a border. Same as the WS_TILED style.
			WS_SIZEBOX |		// The AppWindow has a sizing border. Same as the WS_THICKFRAME style.
			WS_SYSMENU |		// The AppWindow has a AppWindow menu on its title bar. The WS_CAPTION style must also be specified.
			WS_THICKFRAME |		// The AppWindow has a sizing border. Same as the WS_SIZEBOX style.
			WS_SYSMENU | WS_POPUP | 
		//	WS_CLIPCHILDREN |  // If parent AppWindow should not be re-drawn where child windows area..
			WS_CLIPSIBLINGS | 
			WS_VISIBLE | // Have no idea..
			0;

		/// If not resizable, de-flag it
		if (!resizable)
			windowStyle &= ~WS_SIZEBOX;

		/*
		// Set device mode for non-full-screen?
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);       // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth    = width;            // Selected Screen Width
		dmScreenSettings.dmPelsHeight   = height;           // Selected Screen Height
		dmScreenSettings.dmBitsPerPel   = 32;             // Selected Bits Per Pixel
//		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		bool result = ChangeDisplaySettings(&dmScreenSettings, CDS_RESET);
*/
		// Set some mode..
		SetWindowLongPtr(hWnd, GWL_STYLE, windowStyle);
		// And move it.
		MoveWindow(hWnd, previousPosition[0], previousPosition[1], previousSize[0], previousSize[1], true);
#elif defined LINUX
	    // Fetch the XWindow equivalent?
	    XMoveResizeWindow(xDisplay, xWindowHandle, 50, 50, previousSize.x, previousSize.y);
       // assert(false);
#endif
        isFullScreen = false;
	}
	else {
		LogMain("Full-screening window..", DEBUG);
		/// Save away old sizes		
#ifdef WINDOWS
		Vector2i newMin, newSize;
		// Extract old AppWindow-coordinates first?
		RECT rect;
		BOOL success = GetWindowRect(hWnd, &rect);
		if (success)
		{
			previousPosition = Vector2i(rect.left, rect.top);
			previousSize = Vector2i(rect.right - rect.left, rect.bottom - rect.top);
		}

		// First extract screen to full-size in.
		// http://msdn.microsoft.com/en-us/library/windows/desktop/dd145064%28v=vs.85%29.aspx
		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX lpmi;
		// http://msdn.microsoft.com/en-us/library/windows/desktop/dd144901%28v=vs.85%29.aspx
		lpmi.cbSize = sizeof(MONITORINFOEX);
		BOOL getMonitorInfoResult = GetMonitorInfoW(hMonitor, &lpmi);
		if (getMonitorInfoResult)
		{
			newMin[0] = lpmi.rcMonitor.left;
			newMin[1] = lpmi.rcMonitor.top;
			newSize[0] = lpmi.rcMonitor.right - lpmi.rcMonitor.left;
			newSize[1] = lpmi.rcMonitor.bottom - lpmi.rcMonitor.top;
			bool doOffset = false;
			if (doOffset)
			{
				newMin -= Vector2i(2,2);
				newSize += Vector2i(4,4);
			}
		}
		else {
			assert(false);
			//int screenWidth = Graphics.ScreenWidth();
			//int screenHeight = Graphics.ScreenHeight();
			//newMin = Vector2i(-2,-1);
			//newSize = Vector2i(Graphics.ScreenWidth()+4, Graphics.ScreenHeight()+2);
		}
		// Sets full-screen style if specified
		SetWindowLongPtr(hWnd, GWL_STYLE,
//			WS_SYSMENU |
			WS_POPUP | 
		//	WS_CLIPCHILDREN | 
			WS_CLIPSIBLINGS | WS_VISIBLE);
		MoveWindow(hWnd, newMin[0], newMin[1], newSize[0], newSize[1], true);
#elif defined LINUX
		// Get screen size?
		Vector2i newSize = PrimaryScreen().size;
	//	newSize = previousSize + Vector2i(200,200);
		XMoveResizeWindow(xDisplay, xWindowHandle, 0, 0, newSize.x, newSize.y);
#endif
		isFullScreen = true;
	}
}


void AppWindow::SetDefaults()
{
#ifdef WINDOWS
	/// Set AppWindow style
	windowStyle =
		WS_CAPTION |		// The AppWindow has a title bar (includes the WS_BORDER style).
		WS_MAXIMIZEBOX |	// The AppWindow has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
		WS_MINIMIZEBOX |	// The AppWindow has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
		WS_OVERLAPPED |		// The AppWindow is an overlapped AppWindow. An overlapped AppWindow has a title bar and a border. Same as the WS_TILED style.
		WS_SIZEBOX |		// The AppWindow has a sizing border. Same as the WS_THICKFRAME style.
		WS_SYSMENU |		// The AppWindow has a AppWindow menu on its title bar. The WS_CAPTION style must also be specified.
		WS_THICKFRAME |		// The AppWindow has a sizing border. Same as the WS_SIZEBOX style.
//		WS_VISIBLE | // Have no idea..
//			WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		WS_CLIPSIBLINGS |
// WS_CLIPCHILDREN | 
		0;

	/// If not resizable, de-flag it
	if (!resizable)
		windowStyle &= ~WS_SIZEBOX;

	/// Set extended AppWindow styles
	dwExStyle = WS_EX_ACCEPTFILES /// Accept drag-and-drop files
		| WS_EX_APPWINDOW		// Forces a top-level AppWindow onto the taskbar when the AppWindow is visible.
		| WS_EX_STATICEDGE;
#endif
}

/// Create the actual window..
bool AppWindow::Create()
{
#ifdef WINDOWS
	// Set defaults if not done already?
	if (windowStyle == 0)
	{
		std::cout<<"\nSetting default AppWindow styles.";
		SetDefaults();
	}

	// Convert strings..
	static TCHAR szWindowClass[50];
	static TCHAR szTitle[50];
	wcscpy(szTitle, displayName.wc_str());
	wcscpy(szWindowClass, WindowMan.defaultWcx.lpszClassName);
	
	HWND parent = NULL;
	if (!this->main && WindowMan.MainWindow())
	{
		std::cout<<"\nAdding as child to main AppWindow.";
		parent = WindowMan.MainWindow()->hWnd;
	}
	

	Vector2i size(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	// Default size to 2/3 of primary monitor width and height?
	size = GetMonitorSize() * 0.66f;
	LogMain("Using half monitor size for window creation: "+ VectorString(size), INFO);

	if (requestedSize[0] && requestedSize[1] )
		size = requestedSize;
	Vector2i position(CW_USEDEFAULT,CW_USEDEFAULT);
	if (requestedRelativePosition[0] || requestedRelativePosition[1] )
	{	
		RECT parentRect;
		BOOL success = GetWindowRect(parent, &parentRect);
		position = Vector2i(requestedRelativePosition);
		position[0] += parentRect.left;
		position[1] += parentRect.top;
	}

	bool useParenting = false;
	if (!useParenting)
		parent = NULL;

	hWnd = CreateWindowExW(
		dwExStyle,
		szWindowClass, szTitle,
		windowStyle,
		position[0], position[1],
		size[0], size[1],
		parent, 
		NULL,
		Application::hInstance, NULL);
/*	hWnd = CreateWindow(				// The parameters to CreateWindow explained:
		szWindowClass,					// szWindowClass: the name of the application
		szTitle,						// szTitle: the text that appears in the title bar
		windowStyle,					// windowStyle: the type of AppWindow to create
		CW_USEDEFAULT, CW_USEDEFAULT,	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
		500, 100,						// 500, 100: initial size (width, length)
		NULL,							// NULL: the parent of this AppWindow
		NULL,							// NULL: this application does not have a menu bar
		hInstance,						// hInstance: the first parameter from WinMain
		NULL							// NULL: not used in this application
	);
	*/
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Game Engine Win32 Error"),
			NULL);
		return false;
	}

	// Create a standard viewport at least. Always good to have one.
	Viewport * vp = NewA(Viewport);
	vp->window = this;
	viewports.Add(vp);

#elif defined LINUX
	xWindowHandle = XCreateWindow(xDisplay,
                           RootWindow(xDisplay, xVisualInfo->screen),
                           0, 0,            /// Position
                           800, 600,   /// Size
                           0,
                           xVisualInfo->depth,
                           InputOutput,
                           xVisualInfo->visual,
                           CWBorderPixel | CWColormap | CWEventMask,
                           &xWindowAttributes);
	if (xWindowHandle)
	    std::cout<<"\nXCreateWindow created window "<<xWindowHandle<<" successfully";
	else
		std::cout<<"\nXCreateWindow failed.";

	/// Create graphical context... o.o
    XFontStruct * fontinfo = XLoadQueryFont(xDisplay, "10x20");
    XGCValues gr_values;
    gr_values.font = fontinfo->fid;
    gr_values.foreground = XBlackPixel(xDisplay, 0);

    GC graphical_context;
    graphical_context = XCreateGC(xDisplay, xWindowHandle, GCFont+GCForeground, &gr_values);
 
	// set AppWindow properties
    XSetStandardProperties(xDisplay, xWindowHandle, "main", None, None, NULL, 0, NULL);
    // xDisplay X AppWindow on screen
    XMapWindow(xDisplay, xWindowHandle);
    // Set title text
    XStoreName(xDisplay, xWindowHandle, Application::name.c_str());
    /// Fix so we can intercept AppWindow-Management messages (like pressing the Close-button, ALT+F4, etc!)
    // Ref: http://www.opengl.org/discussion_boards/showthread.php/157469-Properly-destroying-a-AppWindow
    Atom wm_protocol = XInternAtom (xDisplay, "WM_PROTOCOLS", False);
    Atom wm_close = XInternAtom (xDisplay, "WM_DELETE_WINDOW", False);
    // Next we elect to receive the 'close' event from the WM:
    XSetWMProtocols (xDisplay, xWindowHandle, &wm_close, 1);
    std::cout<<"\nXCreateWindow: "<<xWindowHandle;
#endif
	created = true;
	return true;
}

void AppWindow::Close()
{
	if (this->main)
	{
		if (Application::queryOnQuit)
			MesMan.QueueMessages("Query(QuitApplication)");
		else 
			MesMan.QueueMessages("QuitApplication");
	}
	else {
		this->Hide();
	}
}


/// Must be called from the same thread that created it (on Windows).
bool AppWindow::Destroy()
{
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms632682%28v=vs.85%29.aspx
	// If the specified AppWindow is a parent or owner AppWindow, DestroyWindow automatically destroys the associated 
	// child or owned windows when it destroys the parent or owner AppWindow. The function first destroys child or 
	// owned windows, and then it destroys the parent or owner AppWindow.
	// A thread cannot use DestroyWindow to destroy a AppWindow created by a different thread. 
//	if (main)
//	{
		BOOL result = DestroyWindow(hWnd);
		assert(result);
//	}
//	return result;
#else
	assert(false);
#endif
	return true;
}

bool AppWindow::IsVisible()
{
	return visible;
}


void AppWindow::MoveToMonitor(int monitorIndex)
{
	List<Monitor> monitors = GetMonitors();
	if (monitors.Size() <= monitorIndex)
		return;
	Monitor monitor = monitors[monitorIndex];
	// Move so we are somewhere in the middle of the monitor?
	MoveCenterTo(monitor.center);
}

Vector2i AppWindow::OSWindowSize()
{
#ifdef WINDOWS
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return Vector2i(rect.right - rect.left, rect.bottom - rect.top);
#elif defined USE_X11
	return clientAreaSize;
#else
	blooo
#endif
}

// o-o
bool AppWindow::GetRayFromScreenCoordinates(int mouseX, int mouseY, Ray & ray)
{
	return GetRayFromScreenCoordinates(Vector2i(mouseX, mouseY), ray);
}

bool AppWindow::GetRayFromScreenCoordinates(Vector2i screenCoords, Ray & ray)
{
	// Get viewport the screen-coordinate are inside.
	for (int i = 0; i < viewports.Size(); ++i)
	{
		Viewport * vp = viewports[i];
		// Check if inside
		if (screenCoords[0] < vp->absMin[0] ||
			screenCoords[0] > vp->absMax[0])
			continue;
		if (screenCoords[1] < vp->absMin[1] ||
			screenCoords[1] > vp->absMax[1])
			continue;
		// Is inside! o.o
		Vector2i viewportCoords = screenCoords - vp->absMin;
		return vp->GetRayFromViewportCoordinates(viewportCoords, ray);
	}
	// No good viewport :(
	return false;
}


void AppWindow::SetMinimumSize(Vector2i newMinimumSize) {
	minimumSize = newMinimumSize;
}

/// Since some parts of the AppWindow may be used by the OS, this has to be taken into consideration when rendering.
Vector2i AppWindow::ClientAreaSize()
{
#ifdef WINDOWS
	RECT rect;
	GetClientRect(hWnd, &rect);
	return Vector2i(rect.right - rect.left, rect.bottom - rect.top);
#elif defined USE_X11
	return clientAreaSize;
#else
	assert(false);
#endif
}

Vector2i AppWindow::GetTopLeftCorner()
{
#ifdef WINDOWS
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return Vector2i(rect.left, rect.top);
#else
	assert(false);
#endif
}

/// Fetches right-edge X-position of the AppWindow.
int AppWindow::GetRight()
{
#ifdef WINDOWS
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return rect.right;
#else
	assert(false);
#endif
}


// Moving the AppWindow using the center of the AppWindow as guide.
void AppWindow::MoveCenterTo(Vector2i position)
{
#ifdef WINDOWS
	Vector2i topLeftCorner = GetTopLeftCorner();
	Vector2i size = OSWindowSize();
	Vector2i halfSize = size * 0.5f;
	Vector2i newTopLeft = position - halfSize;
	MoveWindow(hWnd, newTopLeft[0], newTopLeft[1], size[0], size[1], true);
#else
	assert(false);
#endif
}


void AppWindow::Show()
{
#ifdef WINDOWS
	// Then start accepting messages from WndProc
	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	if (this->main)
		ShowWindow(hWnd, Application::nCmdShow);
	else
		ShowWindow(hWnd, SW_SHOWNOACTIVATE);
	UpdateWindow(hWnd);

	if (!dragAndDrop)
		dragAndDrop = new DragAndDrop();
	/// Enable Drag-n-drop for the AppWindow for non-File types too.
#ifdef USE_OLE
	int result = RegisterDragDrop(hWnd, dragAndDrop);
	switch(result)
	{
		case DRAGDROP_E_INVALIDHWND:
			std::cout<<"Invalid hwNd";
			break;
		case DRAGDROP_E_ALREADYREGISTERED:
			std::cout<<"ar;edastret";
			break;
		case E_OUTOFMEMORY:
			std::cout<<"Invalid MEM";
			break;
	}
#endif
#else
	XMapWindow(xDisplay, xWindowHandle);
#endif
}

// Hides the AppWindow from user interaction! Called by default for non-main windows when closing them.
void AppWindow::Hide()
{
#ifdef WINDOWS
	ShowWindow(hWnd, SW_HIDE);
#else
	assert(false);
#endif
	WindowMan.OnWindowHidden(this);
}

void AppWindow::BringToTop()
{
#ifdef WINDOWS
	BringWindowToTop(hWnd);
#else
	assert(false);
#endif
}

/// Fills contents of current frame into target texture. Exactly which frame which will be sampled depends on the render-thread.
void AppWindow::GetFrameContents(Texture * intoTexture)
{
	// Set bool to capture contents.
	getNextFrame = true;
	frameTexture = intoTexture;
	// Wait until captured.
	while(getNextFrame)
		;
	// 
	frameTexture = NULL;
}

void AppWindow::SetBackgroundColor(const Vector4f & color, bool applyToViewports)
{
	backgroundColor = color;
	if (applyToViewports)
	{
		for (int i = 0; i < viewports.Size(); ++i)
		{
			Viewport * vp = viewports[i];
			vp->backgroundColor = backgroundColor;
		}
	}
}

/// Fetches the (global) UI which is displayed on top of everything else, used for fade-effects etc. Is created dynamically if not set earlier.
UserInterface * AppWindow::GetUI()
{
//	assert(ui && "Should have been created at start");
	if (AppWindow::createUIAutomatically && !ui)
	{
		ui = new UserInterface();
		ui->CreateRoot();
	}
	return ui;
}

/// Fetches the global (system) UI.
UserInterface * AppWindow::GetGlobalUI(bool fromRenderThread)
{
	if (!globalUI && fromRenderThread)
		CreateGlobalUI();
//	assert(globalUI && "Should have been created at start."); 
	// BS. Create it if it's really needed.
	return globalUI;	
}

UserInterface * AppWindow::CreateGlobalUI()
{
	assert(!globalUI);
	globalUI = NewA(UserInterface);
	globalUI->name = name + "WindowGlobalUI";
	globalUI->CreateRoot();
	return globalUI;
}

UserInterface * AppWindow::CreateUI()
{
	assert(ui == NULL);
	ui = NewA(UserInterface);
	ui->CreateRoot();
	ui->name = name + "-DefaultUI";
	return ui;
}

/// Requested size.
void AppWindow::SetRequestedSize(Vector2i size)
{
	requestedSize = size;
}

/// Relative to parent AppWindow.
void AppWindow::SetRequestedRelativePosition(Vector2i pos)
{
	requestedRelativePosition = pos;
}

void AppWindow::RenderGrid(bool renderIt)
{
	for (int j = 0; j < viewports.Size(); ++j)
	{
		Viewport * vp = viewports[j];
		vp->renderGrid = renderIt;
	}
}

int AppWindow::MemLeakTest()
{
#define MEMLEAKTEST
#ifdef MEMLEAKTEST
#ifdef WINDOWS
	while(true)
	{
		HDC hDC = GetDC(hWnd);
		assert(hDC);
		bool result = SetupPixelFormat(hDC);
		assert(result);
	
		HGLRC hGLContext = wglCreateContext(hDC);
		if (hGLContext == NULL)
		{
			int error = GetLastError();
			std::cout<<"\nError in wglCreateContext: "<<error;
			if (error == ERROR_INVALID_PIXEL_FORMAT)
			{
				std::cout<<" invalid pixel format o-o";
			}
			ReleaseDC(hWnd,hDC);
			continue;
		}
		std::cout<<"\nhglrc, AppWindow gl context: "<<hGLContext;
		wglMakeCurrent(hDC, hGLContext);
	//	SleepThread(50);
		// Test generating buffers
		for (int i = 0; i < 10; ++i)
		{
			GLBuffers::New();
		}

		GLBuffers::FreeAll();

		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(hGLContext);
		ReleaseDC(hWnd,hDC);
	}
#endif // WINDOWS
#endif // MEMLEAKTEST
	return 0;
}

#ifdef LINUX
void PrintGLXFBConfig(GLXFBConfig * glxfbConfig, int numSettings, int max = 5)
{
  	String conf;
  	std::cout<<"\nNone: "<<None<<" GLX_FBCONFIG_ID: "<<GLX_FBCONFIG_ID;
  	for (int i = 0; i < numSettings && i < max; ++i)
  	{

  		int value = (int64)glxfbConfig[i];
  		conf += "\nValue "+String(i)+": "+String(value);
  		if (value == GLX_FBCONFIG_ID)
  			conf += "GLX_FBCONFIG_ID";
  	}
  	LogGraphics("glxfbConfig details: "+conf, INFO);
}
#endif

bool AppWindow::CreateGLContext()
{
	std::cout<<"\nCreateGLContext for window "<<name;
	assert(created);
	if (!created)
	{
		LogGraphics("Window not created yet.", ERROR);	
		return false;
	}
#ifdef WINDOWS
	assert(hdc == 0);
	hdc = GetDC(hWnd);
	assert(hdc);
	bool result = SetupPixelFormat(hdc);
	assert(result);
	hglrc = wglCreateContext(hdc);		// Create rendering context
	assert(hglrc);
	return result;
// Linux XWindow system
#elif defined USE_X11
	/// Create GL context! ^^
    std::cout<<"\nCreating GLX context...";
    assert(xDisplay);
    assert(xVisualInfo);

    int major, minor;
	glXQueryVersion(xDisplay, &major, &minor);
	LogGraphics("GLX version: "+String(major)+"."+String(minor), INFO);

	void * shareContext = 0;
	bool directRender = true;
	assert(xGLContext == 0);
//	xGLContext = glXCreateContext(xDisplay, xVisualInfo, (GLXContext)shareContext, directRender);
	xGLContext = glXCreateContext(xDisplay, xVisualInfo, None, true);

	// Test render..?
//	TestRender();

	assert(xGLContext);
  	return true;
#else
  	fel
#endif // OS-dependent code.
}

bool AppWindow::MakeGLContextCurrent()
{
	if (!created)
	{
		LogGraphics("Trying to make GL context current on a non-created window", ERROR);
		return false;
	}
#ifdef WINDOWS
	// If a new AppWindow..
	if (hglrc == 0)
	{
		AppWindow * mainWindow = WindowMan.MainWindow();
		assert(mainWindow && "Should have created context manually..?");
		// Create a new context for it..
		CreateGLContext();
		// .. but link it to the primary AppWindow's gl-context too! o+
		bool share = true;
		if (share)
		{
			BOOL result = wglShareLists(mainWindow->hglrc, hglrc);
			assert(result);
		}
		std::cout<<"\nCreated new GL context and linked it to the main AppWindow.";
	}
	// Make it current
	BOOL result = wglMakeCurrent(hdc, hglrc);	
	return result == TRUE;
#elif defined USE_X11
    // Lock it.
    XLockDisplay(xDisplay);
	bool result = glXMakeCurrent(xDisplay, xWindowHandle, (GLXContext)	 xGLContext);
	XUnlockDisplay(xDisplay);
	return result;
#endif
}

void AppWindow::TestRender()
{
	MakeGLContextCurrent();
#ifdef USE_X11
	//	glXMakeContextCurrent(xDisplay, xWindowHandle, xWindowHandle, (GLXContext)xGLContext);
	float triangleRot = 0.f;
	XWindowAttributes 		window_attributes;
	while(true)
	{
		// Spawn a thread for each?
		// bind the rendering context to the window
		XGetWindowAttributes(xDisplay, xWindowHandle, &window_attributes);
		SetupViewProjectionGL(window_attributes.width, window_attributes.height);
		RenderTestTriangle(triangleRot);
		triangleRot += 0.1f;
		if(XWindowSystem::swapBuffers)
		{
			glXSwapBuffers(xDisplay, xWindowHandle);
		}
		else
		{
			glFlush();
		}
		SleepThread(10);
		
	}
#endif
}

bool AppWindow::DeleteGLContext()
{
#ifdef WINDOWS
	// Deselect rendering context
	BOOL result = wglMakeCurrent(hdc, NULL);		
	if (!result)
	{
		int error = GetLastError();
		if (error == ERROR_INVALID_HANDLE)
		{
			hdc = NULL;
		}
		else
			assert(result == TRUE);
		// Unable to make current? Means we probably cannot delete it either.
		return false;
	}
	// And delete it
	if (hglrc)
	{
		// Need to de-allocate other stuff first?
		// http://stackoverflow.com/questions/10879796/wgldeletecontext-access-violation
		try {
			result = wglDeleteContext(hglrc);
		} catch (...)
		{
			std::cout<<"\nError calling wglDeleteContext";
			hglrc = 0;
			return false;
		}
		hglrc = 0;
		int error = GetLastError();
		if (error)
			std::cout<<"\nError: "<<error;
		hglrc = NULL;
		assert(result == TRUE);
	}
	// Delete hdc last?
	if (hdc)
	{
		result = ReleaseDC(hWnd, hdc);
		if (!result)
		{
			int error = GetLastError();
			if (error == ERROR_INVALID_HANDLE)
			{
				hdc = NULL;
			}
			else if (error == ERROR_DC_NOT_FOUND)
			{
				hdc = NULL;
			}
			else if (error == ERROR_SUCCESS)
			{
				 /// lol, succeeded?
			}
			else
				assert(result == TRUE);
		}
	}
	return result == TRUE;
#elif defined USE_X11
	int result = XWindowSystem::DestroyGLContext(this);
	return result;
#endif // OS-specific code
}

/// Disables all render-flags below
void AppWindow::DisableAllRenders()
{
	renderViewports = 
		renderFPS = 
		renderState =
		renderScene =
		renderUI = false;
}

/// p-=p
Viewport * AppWindow::MainViewport()
{
	if (viewports.Size())
		return viewports[0];
	else 
	{
		// Create default viewport if none exist.
		Viewport * viewport = new Viewport();
		viewport->window = this;
		viewports.Add(viewport);
		return viewport;
	}
	return NULL;
}


#ifdef WINDOWS
//function to set the pixel format for the device context
/*      Function:       SetupPixelFormat
        Purpose:        This function will be responsible
                                for setting the pixel format for the
                                device context.
*/
bool AppWindow::SetupPixelFormat(HDC hDC)
{
    /*      Pixel format index
    */
    int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),          //size of structure
        1,                                      //default version
        PFD_DRAW_TO_WINDOW |                    //AppWindow drawing support
        PFD_SUPPORT_OPENGL |                    //opengl support
        PFD_DOUBLEBUFFER,                       //double buffering support
        PFD_TYPE_RGBA,                          //RGBA color mode
        32,                                     //32 bit color mode
        0, 0, 0, 0, 0, 0,                       //ignore color bits
        0,                                      //no alpha buffer
        0,                                      //ignore shift bit
        0,                                      //no accumulation buffer
        0, 0, 0, 0,                             //ignore accumulation bits
        24,                                     //16 bit z-buffer size
        0,                                      //no stencil buffer
        0,                                      //no aux buffer
        PFD_MAIN_PLANE,                         //main drawing plane
        0,                                      //reserved
        0, 0, 0 };                              //layer masks ignored

    /*      Choose best matching format*/
    nPixelFormat = ChoosePixelFormat(hDC, &pfd);

    /*      Set the pixel format to the device context*/
    BOOL result = SetPixelFormat(hDC, nPixelFormat, &pfd);
	return result == TRUE;
}
#endif

#include "Message/WindowMessage.h"
#include "Graphics/Messages/GMSet.h"
#include "StateManager.h"

void AppWindow::SetClientAreaSize(Vector2i newClientAreaSize) {
	clientAreaSize = newClientAreaSize;
	clientAreaSize = Vector2i::Maximum(clientAreaSize, minimumSize);
}

void AppWindow::OnSizeUpdated()
{
    /// Inform that the window's position has changed?
    WindowMessage * mes = new WindowMessage(WMes::SIZE_UPDATED);
    mes->size = this->clientAreaSize;
    MesMan.QueueMessage(mes);	
	if (this == MainWindow())
		QueueGraphics(new GMSetResolution(this->clientAreaSize, false));
}
