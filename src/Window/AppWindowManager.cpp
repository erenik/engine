/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.

#include "AppWindowManager.h"
#include "Application/Application.h"
#include "Command/CommandLine.h"
#include "FilePath/FilePath.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include "WindowSystem.h"
#include <cassert>

#include "Window/WindowSystem.h"
#include "StateManager.h"

#ifdef USE_X11
	#undef Time
	#include "XWindowSystem.h"
	#include <X11/Xlib.h>
	#include "Window/XProc.h"
	extern Display * xDisplay; // connection to X server
	extern XEvent event;
#endif

WindowManager * WindowManager::windowManager = 0;

WindowManager::WindowManager()
{
	inFocus = false;
	mainWindow = NULL;
	lastActiveWindow = NULL;
	lockChildWindowRelativePositions = false;

	hoverWindow = NULL;
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

// Initialize AppWindow-manager, Linux requires some specifics
void WindowManager::Initialize()
{
#ifdef USE_X11
    assert(XWindowSystem::Initialize());
    XWindowSystem::SetupDefaultWindowProperties();
//    XWindowSystem::CreateDefaultWindow();
 //   XWindowSystem::SetupProtocols();
#endif
}

/// For handling AppWindow-specific messages (instead of having everything the in the MessageManager)
void WindowManager::ProcessMessage(Message * message)
{
#define ASSERT_WINDOW {if (!window){ std::cout<<"\nERROR: No AppWindow by given name \'"<<windowName<<"\'"; return;}}
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg.Contains("HideWindow("))
			{
				String windowName = msg.Tokenize("()")[1];
				AppWindow * window = GetWindowByName(windowName);
				ASSERT_WINDOW;
				window->Hide();
			}
			else if (msg.Contains("MoveWindowToMonitor("))
			{
				List<String> args = msg.Tokenize("(,)");
				if (args.Size() < 3)
					return;
				String windowName = args[1];
				int monitor = args[2].ParseInt();
				AppWindow * window = WindowMan.GetWindowByName(windowName);
				ASSERT_WINDOW;
				window->MoveToMonitor(monitor);
			}
			break;	
		}
	}
}

/// Fetches incoming messages from the OS.
void WindowManager::ProcessMessages()
{
	// Main message loop for all extra created windows, since they are dependent on the thread they were created in...
#ifdef WINDOWS
	// Get messages and dispatch them to WndProc
	MSG msg;
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644943%28v=vs.85%29.aspx
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
#elif defined USE_X11
	// TODO: Add linux version in an elif for more created windows?
	int events = 0;
	while(events = XPending(xDisplay) && StateMan.ActiveStateID() != GameStateID::GAME_STATE_EXITING)
	{	
        // XNextEvent may block until an event appears, which might not be wanted, to check beforehand how many events are available!
        XNextEvent(xDisplay, &event);
        if (XProc() != NULL)
            break;
    }
#endif // OS-specific message processing.
}


/// Used to create the AppWindow class to be used when creating windows?
bool WindowManager::CreateDefaultWindowClass()
{
#ifdef WINDOWS
	// Check registry for Resizability
	HINSTANCE iconHInst = NULL;
	// Use default icon, Consider looking up design for a real icon
//	HICON icon = LoadIconW(iconHInst, (WORD)MAKEINTRESOURCEW(IDI_APPLICATION));
//	icon = (HICON) LoadImage(hInstance, L"Game.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	/// Class styles
	UINT windowClassStyle =
		CS_HREDRAW |			// Redraws the entire AppWindow if a movement or size adjustment changes the width of the client area.
		CS_VREDRAW |			// Redraws the entire AppWindow if a movement or size adjustment changes the height of the client area.
		CS_DBLCLKS |			// Sends a double-click message to the AppWindow procedure when the user double-clicks the mouse while the cursor is within a AppWindow belonging to the class.
	//	CS_NOCLOSE |			// Disables Close on the AppWindow menu.
	//	CS_BYTEALIGNCLIENT |	// Aligns the AppWindow's client area on a byte boundary (in the x direction).
	//	CS_BYTEALIGNWINDOW |	// Aligns the AppWindow on a byte boundary (in the x direction).
		
		// Allocates a unique device context for each AppWindow in the class. CS_OWNDC 0x0020 
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff729176%28v=vs.85%29.aspx
		CS_OWNDC | 
		0;

	// Define AppWindow class structure
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
	// Register the AppWindow class structure
	if (!RegisterClassExW(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Win32 Guided Tour"),
            NULL);

        return 1;
    }
#endif
}

/// Print stuff
void WindowManager::ListWindows()
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		std::cout<<"\nWindow "<<i<<" name: "<<window->name<<" displayName: "<<window->displayName;
	}
}


/// Removes and deletes all windows.
void WindowManager::DeleteWindows()
{
	windows.ClearAndDelete();
}

/// For creating the first AppWindow, sets certain properties as needed.
AppWindow * WindowManager::CreateMainWindow()
{
#ifdef USE_SSE
//	mainWindow = NewAligned<AppWindow>(AppWindow("MainWindow", Application::name));
	mainWindow = NewAS(AppWindow, AppWindow("MainWindow", Application::name));
	/* Macro testing.
	mainWindow = AllocAligned(AppWindow);
	mainWindow = Construct(mainWindow, AppWindow("MainWindow", Application::name));
	mainWindow = NewAligned(AppWindow, AppWindow("MainWindow", Application::name));
*/
//	mainWindow = NewAligned(AppWindow, AppWindow("MainWindow", Application::name));
	// Working.
//	mainWindow = (AppWindow*) _aligned_malloc(1 * sizeof(AppWindow), 16);
//	mainWindow = new((void*)mainWindow) AppWindow("MainWindow", Application::name);
#else
	mainWindow = new AppWindow("MainWindow", Application::name);
#endif
	mainWindow->main = true;
	windows.Add(mainWindow);
	return mainWindow;
}

/// Creates a new AppWindow, returning a reference to it.
AppWindow * WindowManager::NewWindow(String name, String displayName)
{

	AppWindow * window = new AppWindow(name, displayName);
	windows.AddItem(window);
	return window;
}

AppWindow * WindowManager::GetWindow(int index)
{
	return windows[index];
}

AppWindow * WindowManager::GetWindowByName(String name)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		if (window->name == name)
			return window;
	}
	return NULL;
}

#ifdef WINDOWS
AppWindow * WindowManager::GetWindowByHWND(HWND hWnd)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		if (window->hWnd == hWnd)
			return window;
	}
	return NULL;
}
#endif

AppWindow * WindowManager::GetCurrentlyActiveWindow()
{
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633505%28v=vs.85%29.aspx
	HWND activeWindow = GetForegroundWindow();
	if (!activeWindow)
		return NULL;
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
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

bool WindowManager::InFocus()
{
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633505%28v=vs.85%29.aspx
	HWND activeWindow = GetForegroundWindow();
#elif defined USE_X11
	assert(xDisplay);
	Window focused;
	int revert_to;
	XGetInputFocus(xDisplay, &focused, &revert_to);
#endif // OS-specific preparation

	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		
#ifdef WINDOWS
		if (window->hWnd == activeWindow)
#elif defined USE_X11
		if (window->xWindowHandle == focused)
#endif
		{
			lastActiveWindow = window;
			return true;
		}
	}
	return false;
}

/// To see if the application should quit?
void WindowManager::OnWindowHidden(AppWindow * w)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		if (window->IsVisible())
		{
			return;
		}
	}
	/// Shutdown if no windows were visible... eh.
	if (Application::quitOnHide)
		MesMan.QueueMessages("QuitApplication");
}



/// Using relative positions
void WindowManager::UpdateChildWindows()
{
	if (!lockChildWindowRelativePositions)
		return;
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		if (window->main)
			continue;
		window->UpdatePosition();
	}
}


void WindowManager::MoveAllChildWindows(Vector2i byThisAmount)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		AppWindow * window = windows[i];
		if (window->main)
			continue;
		window->Move(byThisAmount);
	}
}


