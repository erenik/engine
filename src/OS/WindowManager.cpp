/// Emil Hedemalm
/// 2014-01-27
/// Manager for creation and deletion of OS-based windows. 
/// For now will serve its purpose by storing focus-related data to determine how much processor consumption the processes should use.

#include "WindowManager.h"

#include <cassert>

WindowManager * WindowManager::windowManager = 0;

WindowManager::WindowManager(){
	inFocus = false;
}
WindowManager::~WindowManager(){

}

void WindowManager::Allocate(){
	assert(windowManager == 0);
	windowManager = new WindowManager();
}

void WindowManager::Deallocate(){
	assert(windowManager);
	delete windowManager;
	windowManager = 0;
}

WindowManager * WindowManager::Instance(){
	assert(windowManager);
	return windowManager;
}

/// Sets if any of our windows has focus or not.
void WindowManager::SetFocus(int focus){
	inFocus = focus;
}

bool WindowManager::InFocus(){
	return inFocus;
}