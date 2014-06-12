/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.

#include "WindowManager.h"
#include "Application/Application.h"
#include "Command/CommandLine.h"
#include "FilePath/FilePath.h"

#include <cassert>

WindowManager * WindowManager::windowManager = 0;

WindowManager::WindowManager()
{
	inFocus = false;
	mainWindow = NULL;
	lastActiveWindow = NULL;
}
WindowManager::~WindowManager()
{
	windows.ClearAndDelete();
}

void WindowManager::Allocate()
{
	assert(windowManager == 0);
	windowManager = new WindowManager();
}

void WindowManager::Deallocate()
{
	assert(windowManager);
	delete windowManager;
	windowManager = 0;
}

WindowManager * WindowManager::Instance()
{
	assert(windowManager);
	return windowManager;
}

/// Used to create the window class to be used when creating windows?
bool WindowManager::CreateDefaultWindowClass()
{
	// Check registry for Resizability
	HINSTANCE iconHInst = NULL;
	// Use default icon, Consider looking up design for a real icon
//	HICON icon = LoadIconW(iconHInst, (WORD)MAKEINTRESOURCEW(IDI_APPLICATION));
//	icon = (HICON) LoadImage(hInstance, L"Game.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	/// Class styles
	UINT windowClassStyle =
		CS_HREDRAW |			// Redraws the entire window if a movement or size adjustment changes the width of the client area.
		CS_VREDRAW |			// Redraws the entire window if a movement or size adjustment changes the height of the client area.
		CS_DBLCLKS |			// Sends a double-click message to the window procedure when the user double-clicks the mouse while the cursor is within a window belonging to the class.
	//	CS_NOCLOSE |			// Disables Close on the window menu.
	//	CS_BYTEALIGNCLIENT |	// Aligns the window's client area on a byte boundary (in the x direction).
	//	CS_BYTEALIGNWINDOW |	// Aligns the window on a byte boundary (in the x direction).
		0;

	// Define window class structure
	WNDCLASSEXW & wcex = defaultWcx;
	static WCHAR szWindowClass[] = L"win32app";
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style          = windowClassStyle;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = Application::hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
	wcex.lpszClassName = szWindowClass;
//    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = 0;
	// Register the window class structure
	if (!RegisterClassExW(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Win32 Guided Tour"),
            NULL);

        return 1;
    }
}

/// Removes and deletes all windows.
void WindowManager::DeleteWindows()
{
	windows.ClearAndDelete();
}

/// For creating the first window, sets certain properties as needed.
Window * WindowManager::CreateMainWindow()
{
	mainWindow = new Window(Application::name);
	mainWindow->main = true;
	windows.Add(mainWindow);
	return mainWindow;
}

/// Creates a new window, returning a reference to it.
Window * WindowManager::NewWindow(String name)
{

	Window * window = new Window(name);
	windows.Add(window);
	return window;
}

Window * WindowManager::GetWindow(int index)
{
	return windows[index];
}

#ifdef WINDOWS
Window * WindowManager::GetWindowByHWND(HWND hWnd)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		if (window->hWnd == hWnd)
			return window;
	}
	return NULL;
}
#endif

Window * WindowManager::GetCurrentlyActiveWindow()
{
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633505%28v=vs.85%29.aspx
	HWND activeWindow = GetForegroundWindow();
	if (!activeWindow)
		return NULL;
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		if (window->hWnd == activeWindow)
		{
			lastActiveWindow = window;
			return window;
		}
	}
	// Not the current one?
	// Check the last one that we recorded user-interaction on then?
	return lastActiveWindow;
#endif
	return NULL;
}
	

const int WindowManager::NumWindows() const
{
	return windows.Size();
} 

/// Sets if any of our windows has focus or not.
void WindowManager::SetFocus(int focus){
	inFocus = focus;
}

bool WindowManager::InFocus(){
	return inFocus;
}