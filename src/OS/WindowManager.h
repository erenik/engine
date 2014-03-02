/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#define WindowMan (*WindowManager::Instance())

class WindowManager {
	WindowManager();
	~WindowManager();
	static WindowManager * windowManager;
public:
	static void Allocate();
	static void Deallocate();
	static WindowManager * Instance();

	/// Sets if any of our windows has focus or not.
	void SetFocus(int focus);
	bool InFocus();
private:
	/// 
	bool inFocus;
};

#endif