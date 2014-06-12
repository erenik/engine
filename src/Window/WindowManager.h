/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "OS/OS.h"

#include "Window.h"
#include "List/List.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif


#define WindowMan (*WindowManager::Instance())

class WindowManager {
	WindowManager();
	~WindowManager();
	static WindowManager * windowManager;
public:
	static void Allocate();
	static void Deallocate();
	static WindowManager * Instance();

#ifdef WINDOWS
	WNDCLASSEXW defaultWcx;
#endif

#undef CreateWindow
	/// For creating the first window, sets certain properties as needed.
	Window * CreateMainWindow();
	/// Creates a new window, returning a reference to it.
	Window * NewWindow(String name);
	Window * GetWindow(int index);
#ifdef WINDOWS
	Window * GetWindowByHWND(HWND hWnd);
#endif
	Window * MainWindow() {return mainWindow;};
	List<Window*> GetWindows(){return windows;};
	/// If an application window is the top one, it is returned. If another app is currently the top one, the most recently interacted window of this app is returned.
	Window * GetCurrentlyActiveWindow();
	const int NumWindows() const;
	/// Used to create the window class to be used when creating windows?
	bool CreateDefaultWindowClass();

	/// Removes and deletes all windows.
	void DeleteWindows();

	/// Sets if any of our windows has focus or not.
	void SetFocus(int focus);
	bool InFocus();
private:
	// Main window. Stores initial GL context that is to be shared and also binds its destroy message to closing the application.
	Window * mainWindow;
	Window * lastActiveWindow;
	/// All windows 
	List<Window*> windows;
	/// 
	bool inFocus;
};

#endif