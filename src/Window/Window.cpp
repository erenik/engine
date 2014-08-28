// Fredrik Larsson && Emil Hedemalm
// 2013-07-03 Linuxifixation!

#include "Window.h"
#include "Application/Application.h"
#include "WindowManager.h"
#include "Viewport.h"
#include "UI/UserInterface.h"
#include "Graphics/GLBuffers.h"

#include "DragAndDrop.h"

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

	bool result = GetMonitorInfoW(hMonitor, &monitor.monitorInfo);
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

/// Fetches window that is currently active. Returns NULL if it is not one owned by the application.
Window * ActiveWindow()
{
	return WindowMan.GetCurrentlyActiveWindow();	
}
/// Fetches the main application window.
Window * MainWindow()
{
	return WindowMan.MainWindow();
}
/// Window mouse is currently hovering over.
Window * HoverWindow()
{
	return WindowMan.HoverWindow();
}


Window::Window(String name)
{
	this->name = title = name;
	
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

	backgroundColor = Vector4f(0.5f,.5f,.5f,1);

#ifdef WINDOWS
	hWnd = NULL;
	hdc = NULL;
	hglrc = NULL;

	windowStyle = 0;
	dwExStyle = 0;
	dad = NULL;
#endif

	saveScreenshot = false;
	/// 50 ms, 20 fps recording speed.
	isRecording = false;
	recordVideo = false;
	msBetweenFrames = 50;
}

Window::~Window()
{
/// Destroy WINDOW o/o
#ifdef WINDOWS
	
#elif defined USE_X11
    XDestroyWindow(display, window);
    XCloseDisplay(display);
#endif

	if (dad)
		delete dad;

	/// Probably dynamically allocated, yeah.
	viewports.ClearAndDelete();
}

/// o-o
Vector2i Window::GetWindowCoordsFromScreenCoords(Vector2i screenPos)
{
#ifdef WINDOWS
	POINT pt;
	pt.x = screenPos.x;
	pt.y = screenPos.y;
	BOOL ok = ScreenToClient(hWnd, &pt);
	if (!ok)
		return Vector2i(-1,-1);
#endif
	// Invert Y because windows calculates the Y coordinate inversly from what we do.
	return Vector2i(pt.x, clientAreaSize.y - pt.y);
}

/// Updates positions, using parent as relative (if specified)
void Window::UpdatePosition()
{
	if (requestedRelativePosition.x || requestedRelativePosition.y )
	{	
		RECT parentRect;
		bool success = GetWindowRect(MainWindow()->hWnd, &parentRect);
		position = Vector2i(requestedRelativePosition);
		position.x += parentRect.left;
		position.y += parentRect.top;
	}
	MoveWindow(hWnd, position.x, position.y, osWindowSize.x, osWindowSize.y, true);
}

void Window::Move(Vector2i byThisAmount)
{
	// get current pos?
	Vector2i currentPos = this->position;
	Vector2i newPos = currentPos + byThisAmount;
	Vector2i size = this->OSWindowSize();
#ifdef WINDOWS
	MoveWindow(hWnd, newPos.x, newPos.y, size.x, size.y, true); 
#endif
}

/// Ensures both UI and GlobalUI has been set.
void Window::EnsureUIIsCreated()
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

/// Toggles full-screen for this window.
void Window::ToggleFullScreen()
{
		std::cout<<"\nGraphicsManager::ToggleFullScreen";
	// Check full-screen state
	if (isFullScreen){
	    /// If was full-screen, go back to previous-size!
#ifdef WINDOWS
		/// Set window style
		windowStyle =
			WS_CAPTION |		// The window has a title bar (includes the WS_BORDER style).
			WS_MAXIMIZEBOX |	// The window has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
			WS_MINIMIZEBOX |	// The window has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
			WS_OVERLAPPED |		// The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style.
			WS_SIZEBOX |		// The window has a sizing border. Same as the WS_THICKFRAME style.
			WS_SYSMENU |		// The window has a window menu on its title bar. The WS_CAPTION style must also be specified.
			WS_THICKFRAME |		// The window has a sizing border. Same as the WS_SIZEBOX style.
			WS_SYSMENU | WS_POPUP | 
		//	WS_CLIPCHILDREN |  // If parent window should not be re-drawn where child windows area..
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
		MoveWindow(hWnd, previousPosition.x, previousPosition.y, previousSize.x, previousSize.y, true);
#elif defined LINUX
        XResizeWindow(display, window, Graphics.oldWidth, Graphics.oldHeight);
#endif
        isFullScreen = false;
	}
	else {
		/// Save away old sizes		
#ifdef WINDOWS
/*
		// Set device mode for full-screen?
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);       // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth    = width;            // Selected Screen Width
		dmScreenSettings.dmPelsHeight   = height;           // Selected Screen Height
		dmScreenSettings.dmBitsPerPel   = 32;             // Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		bool result = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
*/

		Vector2i newMin, newSize;

		// Extract old window-coordinates first?
		RECT rect;
		bool success = GetWindowRect(hWnd, &rect);
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
			newMin.x = lpmi.rcMonitor.left - 2;
			newMin.y = lpmi.rcMonitor.top - 1;
			newSize.x = lpmi.rcMonitor.right - lpmi.rcMonitor.left + 4;
			newSize.y = lpmi.rcMonitor.bottom - lpmi.rcMonitor.top + 2;
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
			WS_SYSMENU |
			WS_POPUP | 
		//	WS_CLIPCHILDREN | 
			WS_CLIPSIBLINGS | WS_VISIBLE);
		MoveWindow(hWnd, newMin.x, newMin.y, newSize.x, newSize.y, true);
#elif defined LINUX
        XResizeWindow(display, window, Graphics.ScreenWidth(), Graphics.ScreenHeight());
#endif
		isFullScreen = true;
	}
}


void Window::SetDefaults()
{
	/// Set window style
	windowStyle =
		WS_CAPTION |		// The window has a title bar (includes the WS_BORDER style).
		WS_MAXIMIZEBOX |	// The window has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
		WS_MINIMIZEBOX |	// The window has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
		WS_OVERLAPPED |		// The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style.
		WS_SIZEBOX |		// The window has a sizing border. Same as the WS_THICKFRAME style.
		WS_SYSMENU |		// The window has a window menu on its title bar. The WS_CAPTION style must also be specified.
		WS_THICKFRAME |		// The window has a sizing border. Same as the WS_SIZEBOX style.
//		WS_VISIBLE | // Have no idea..
//			WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
WS_CLIPSIBLINGS |
// WS_CLIPCHILDREN | 
		0;

	/// If not resizable, de-flag it
	if (!resizable)
		windowStyle &= ~WS_SIZEBOX;

	/// Set extended window styles
	dwExStyle = WS_EX_ACCEPTFILES /// Accept drag-and-drop files
		| WS_EX_APPWINDOW		// Forces a top-level window onto the taskbar when the window is visible.
		| WS_EX_STATICEDGE;
}

/// Create the actual window.
bool Window::Create()
{
	// Set defaults if not done already?
	if (windowStyle == 0)
	{
		std::cout<<"\nSetting default window styles.";
		SetDefaults();
	}

	// Convert strings..
	static TCHAR szWindowClass[50];
	static TCHAR szTitle[50];
	wcscpy(szTitle, title.wc_str());
	wcscpy(szWindowClass, WindowMan.defaultWcx.lpszClassName);
	
	HWND parent = NULL;
	if (!this->main && WindowMan.MainWindow())
	{
		std::cout<<"\nAdding as child to main window.";
		parent = WindowMan.MainWindow()->hWnd;
	}
	

	Vector2i size(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	if (requestedSize.x && requestedSize.y )
		size = requestedSize;
	Vector2i position(CW_USEDEFAULT,CW_USEDEFAULT);
	if (requestedRelativePosition.x || requestedRelativePosition.y )
	{	
		RECT parentRect;
		bool success = GetWindowRect(parent, &parentRect);
		position = Vector2i(requestedRelativePosition);
		position.x += parentRect.left;
		position.y += parentRect.top;
	}

	bool useParenting = false;
	if (!useParenting)
		parent = NULL;

	hWnd = CreateWindowExW(
		dwExStyle,
		szWindowClass, szTitle,
		windowStyle,
		position.x, position.y,
		size.x, size.y,
		parent, 
		NULL,
		Application::hInstance, NULL);
/*	hWnd = CreateWindow(				// The parameters to CreateWindow explained:
		szWindowClass,					// szWindowClass: the name of the application
		szTitle,						// szTitle: the text that appears in the title bar
		windowStyle,					// windowStyle: the type of window to create
		CW_USEDEFAULT, CW_USEDEFAULT,	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
		500, 100,						// 500, 100: initial size (width, length)
		NULL,							// NULL: the parent of this window
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
	Viewport * vp = new Viewport();
	vp->window = this;
	viewports.Add(vp);

	created = true;
	return true;
}

/// Must be called from the same thread that created it (on Windows).
bool Window::Destroy()
{
	bool result;
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms632682%28v=vs.85%29.aspx
	// If the specified window is a parent or owner window, DestroyWindow automatically destroys the associated 
	// child or owned windows when it destroys the parent or owner window. The function first destroys child or 
	// owned windows, and then it destroys the parent or owner window.
	// A thread cannot use DestroyWindow to destroy a window created by a different thread. 
	if (main)
	{
		result = DestroyWindow(hWnd);
		assert(result);
	}
#endif
	return result;
}

bool Window::IsVisible()
{
#ifdef WINDOWS
	return visible;
//	return IsWindowVisible(hWnd);
#endif
}


void Window::MoveToMonitor(int monitorIndex)
{
	List<Monitor> monitors = GetMonitors();
	if (monitors.Size() <= monitorIndex)
		return;
	Monitor monitor = monitors[monitorIndex];
	// Move so we are somewhere in the middle of the monitor?
	MoveCenterTo(monitor.center);
}

Vector2i Window::OSWindowSize()
{
#ifdef WINDOWS
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return Vector2i(rect.right - rect.left, rect.bottom - rect.top);
#endif
}

// o-o
bool Window::GetRayFromScreenCoordinates(int mouseX, int mouseY, Ray & ray)
{
	return GetRayFromScreenCoordinates(Vector2i(mouseX, mouseY), ray);
}

bool Window::GetRayFromScreenCoordinates(Vector2i screenCoords, Ray & ray)
{
	// Get viewport the screen-coordinate are inside.
	for (int i = 0; i < viewports.Size(); ++i)
	{
		Viewport * vp = viewports[i];
		// Check if inside
		if (screenCoords.x < vp->absMin.x ||
			screenCoords.x > vp->absMax.x)
			continue;
		if (screenCoords.y < vp->absMin.y ||
			screenCoords.y > vp->absMax.y)
			continue;
		// Is inside! o.o
		Vector2i viewportCoords = screenCoords - vp->absMin;
		return vp->GetRayFromViewportCoordinates(viewportCoords, ray);
	}
	// No good viewport :(
	return false;
}


/// Since some parts of the window may be used by the OS, this has to be taken into consideration when rendering.
Vector2i Window::ClientAreaSize()
{
#ifdef WINDOWS
	RECT rect;
	GetClientRect(hWnd, &rect);
	return Vector2i(rect.right - rect.left, rect.bottom - rect.top);
#endif
}

Vector2i Window::GetTopLeftCorner()
{
#ifdef WINDOWS
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return Vector2i(rect.left, rect.top);
#endif
}

/// Fetches right-edge X-position of the window.
int Window::GetRight()
{
#ifdef WINDOWS
	RECT rect;
	GetWindowRect(hWnd, &rect);
	return rect.right;
#endif
}


// Moving the window using the center of the window as guide.
void Window::MoveCenterTo(Vector2i position)
{
#ifdef WINDOWS
	Vector2i topLeftCorner = GetTopLeftCorner();
	Vector2i size = OSWindowSize();
	Vector2i halfSize = size * 0.5f;
	Vector2i newTopLeft = position - halfSize;
	MoveWindow(hWnd, newTopLeft.x, newTopLeft.y, size.x, size.y, true);
#endif
}


void Window::Show()
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

	if (!dad)
		dad = new DragAndDrop();
	/// Enable Drag-n-drop for the window for non-File types too.
	int result = RegisterDragDrop(hWnd, dad);
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
}

// Hides the window from user interaction! Called by default for non-main windows when closing them.
void Window::Hide()
{
#ifdef WINDOWS
	ShowWindow(hWnd, SW_HIDE);
#endif
}

void Window::BringToTop()
{
#ifdef WINDOWS
	BringWindowToTop(hWnd);
#endif
}

/// Fills contents of current frame into target texture. Exactly which frame which will be sampled depends on the render-thread.
void Window::GetFrameContents(Texture * intoTexture)
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


/// Fetches the (global) UI which is displayed on top of everything else, used for fade-effects etc. Is created dynamically if not set earlier.
UserInterface * Window::GetUI()
{
//	assert(ui && "Should have been created at start");
	return ui;
}

/// Fetches the global (system) UI.
UserInterface * Window::GetGlobalUI(bool fromRenderThread)
{
	if (!globalUI && fromRenderThread)
		CreateGlobalUI();
//	assert(globalUI && "Should have been created at start."); 
	// BS. Create it if it's really needed.
	return globalUI;	
}

UserInterface * Window::CreateGlobalUI()
{
	assert(!globalUI);
	globalUI = new UserInterface();
	globalUI->name = name + "WindowGlobalUI";
	globalUI->CreateRoot();
	return globalUI;
}

UserInterface * Window::CreateUI()
{
	assert(ui == NULL);
	ui = new UserInterface();
	ui->CreateRoot();
	ui->name = name + "WindowUI";
	return ui;
}

/// Requested size.
void Window::SetRequestedSize(Vector2i size)
{
	requestedSize = size;
}

/// Relative to parent window.
void Window::SetRequestedRelativePosition(Vector2i pos)
{
	requestedRelativePosition = pos;
}

int Window::MemLeakTest()
{
#define MEMLEAKTEST
#ifdef MEMLEAKTEST
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
		std::cout<<"\nhglrc, window gl context: "<<hGLContext;
		wglMakeCurrent(hDC, hGLContext);
	//	Sleep(50);
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
#endif
	return 0;
}

bool Window::CreateGLContext()
{
	if (!created)
		return false;
#ifdef WINDOWS
	assert(hdc == 0);
	/// Wait until we have a valid window to draw in.
	while(hWnd == NULL)
		Sleep(5);
	hdc = GetDC(hWnd);
	assert(hdc);
	bool result = SetupPixelFormat(hdc);
	assert(result);
	hglrc = wglCreateContext(hdc);		// Create rendering context
	assert(hglrc);
#endif
	return result;
}

bool Window::MakeGLContextCurrent()
{
	if (!created)
		return false;
	// If a new window..
	if (hglrc == 0)
	{
		Window * mainWindow = WindowMan.MainWindow();
		assert(mainWindow && "Should have created context manually..?");
		// Create a new context for it..
		CreateGLContext();
		// .. but link it to the primary window's gl-context too! o+
		bool share = true;
		if (share)
		{
			bool result = wglShareLists(mainWindow->hglrc, hglrc);
			assert(result);
		}
		std::cout<<"\nCreated new GL context and linked it to the main window.";
	}
#ifdef WINDOWS
	bool result = wglMakeCurrent(hdc, hglrc);	// Make it current
#endif
	return result;
}

bool Window::DeleteGLContext()
{
#ifdef WINDOWS
	// Deselect rendering context
	int result = wglMakeCurrent(hdc, NULL);		
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
#endif
	return result;
}

/// Disables all render-flags below
void Window::DisableAllRenders()
{
	renderViewports = 
		renderFPS = 
		renderState =
		renderScene =
		renderUI = false;
}

/// p-=p
Viewport * Window::MainViewport()
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
bool Window::SetupPixelFormat(HDC hDC)
{
    /*      Pixel format index
    */
    int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),          //size of structure
        1,                                      //default version
        PFD_DRAW_TO_WINDOW |                    //window drawing support
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
    bool result = SetPixelFormat(hDC, nPixelFormat, &pfd);
	return result;
}
#endif



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
