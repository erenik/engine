/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.
/// See AppWindow.h for a proper guide of what is required to do when setting up a new AppWindow.

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "OS/OS.h"

#include "AppWindow.h"
#include "List/List.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

class Message;

#define WindowMan (*WindowManager::Instance())

class WindowManager 
{
	friend class AppWindow;
	WindowManager();
	~WindowManager();
	static WindowManager * windowManager;
#ifdef WINDOWS
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif

public:
	static void Allocate();
	static void Deallocate();
	static WindowManager * Instance();
	/// To be called once from the main State-processing and AppWindow-managing thread.
	void Initialize();

	/// For handling AppWindow-specific messages (instead of having everything the in the MessageManager)
	void ProcessMessage(Message * message);

#ifdef WINDOWS
	WNDCLASSEXW defaultWcx;
#endif

	/// For creating the first AppWindow, sets certain properties as needed.
	AppWindow * CreateMainWindow();
	/// Creates a new AppWindow, returning a reference to it.
	AppWindow * NewWindow(String name, String displayName);
	AppWindow * GetWindow(int index);
	AppWindow * GetWindowByName(String name);
#ifdef WINDOWS
	AppWindow * GetWindowByHWND(HWND hWnd);
#endif
	AppWindow * MainWindow() {return mainWindow;};
	AppWindow * HoverWindow() { return hoverWindow;};
	List<AppWindow*> GetWindows(){return windows;};
	/// If an application AppWindow is the top one, it is returned. If another app is currently the top one, the most recently interacted AppWindow of this app is returned.
	AppWindow * GetCurrentlyActiveWindow();
	const int NumWindows() const;
	/// Used to create the AppWindow class to be used when creating windows?
	bool CreateDefaultWindowClass();

	/// Print stuff
	void ListWindows();

	/// Using relative positions
	void UpdateChildWindows();
	/// Not really working..
	void MoveAllChildWindows(Vector2i byThisAmount);

	/// Removes and deletes all windows.
	void DeleteWindows();

	/// Returns true if any of our windows is in focus.
	bool InFocus();

	/// If set, will make child windows move along the main AppWindow when it is moved. Useful for certain editors..?
	bool lockChildWindowRelativePositions;
private:
	/// To see if the application should quit?
	void OnWindowHidden(AppWindow * w);

	// Main AppWindow. Stores initial GL context that is to be shared and also binds its destroy message to closing the application.
	AppWindow * mainWindow;
	// AppWindow mouse is currently hovering over (as per the latest mouse-related message received)
	AppWindow * hoverWindow;
	AppWindow * lastActiveWindow;
	/// All windows 
	List<AppWindow*> windows;
	/// 
	bool inFocus;
};

#endif