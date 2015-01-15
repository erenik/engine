/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.

#include "WindowManager.h"
#include "Application/Application.h"
#include "Command/CommandLine.h"
#include "FilePath/FilePath.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include <cassert>

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

/// For handling Window-specific messages (instead of having everything the in the MessageManager)
void WindowManager::ProcessMessage(Message * message)
{
#define ASSERT_WINDOW {if (!window){ std::cout<<"\nERROR: No window by given name \'"<<windowName<<"\'"; return;}}
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg.Contains("HideWindow("))
			{
				String windowName = msg.Tokenize("()")[1];
				Window * window = GetWindowByName(windowName);
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
				Window * window = WindowMan.GetWindowByName(windowName);
				ASSERT_WINDOW;
				window->MoveToMonitor(monitor);
			}
			break;	
		}
	}
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
		
		// Allocates a unique device context for each window in the class. CS_OWNDC 0x0020 
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff729176%28v=vs.85%29.aspx
		CS_OWNDC | 
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

/// Print stuff
void WindowManager::ListWindows()
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		std::cout<<"\nWindow "<<i<<" name: "<<window->name<<" displayName: "<<window->displayName;
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
	mainWindow = new Window("MainWindow", Application::name);
	mainWindow->main = true;
	windows.Add(mainWindow);
	return mainWindow;
}

/// Creates a new window, returning a reference to it.
Window * WindowManager::NewWindow(String name, String displayName)
{

	Window * window = new Window(name, displayName);
	windows.Add(window);
	return window;
}

Window * WindowManager::GetWindow(int index)
{
	return windows[index];
}

Window * WindowManager::GetWindowByName(String name)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		if (window->name == name)
			return window;
	}
	return NULL;
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

bool WindowManager::InFocus()
{
	
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633505%28v=vs.85%29.aspx
	HWND activeWindow = GetForegroundWindow();
#endif WINDOWS
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
#ifdef WINDOWS
		if (window->hWnd == activeWindow)
		{
#endif
			lastActiveWindow = window;
			return window;
		}
	}
	return false;
}

/// To see if the application should quit?
void WindowManager::OnWindowHidden(Window * w)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		if (window->IsVisible())
		{
			return;
		}
	}
	/// Shutdown if no windows were visible.
	MesMan.QueueMessages("QuitApplication");
}



/// Using relative positions
void WindowManager::UpdateChildWindows()
{
	if (!lockChildWindowRelativePositions)
		return;
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		if (window->main)
			continue;
		window->UpdatePosition();
	}
}


void WindowManager::MoveAllChildWindows(Vector2i byThisAmount)
{
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		if (window->main)
			continue;
		window->Move(byThisAmount);
	}
}


