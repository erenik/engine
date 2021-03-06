// Emil Hedemalm
// 2013-07-03 Linuxifying!

#include "AppState.h"
#include "../Managers.h"
extern UserInterface * ui[GameStateID::MAX_GAME_STATES];

#include "OS/OS.h"
#include "OS/Sleep.h"

#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GraphicsMessages.h"

#ifdef WINDOWS
#include <Windows.h>
#endif

Initialization::Initialization(){
    id = GameStateID::GAME_STATE_INITIALIZATION;
    name = "Initialization state";
}

void Initialization::OnEnter(AppState * previousState){
	std::cout<<"\nEntering Initialization StateMan.";
#ifdef WINDOWS
	// Get last AppWindow position
	// Update AppWindow size
	//	SetWindowPos(hWnd, HWND_TOP, 0, 0, 800, 600, SWP_NOMOVE/*SWP_HIDEWINDOW*/);
//	MoveWindow(hWnd, 0, 0, 800, 600, true);
#endif
    AudioMan.QueueMessage(new AMPlaySFX("AeonicEngine.ogg", 0.2f));
};

void Initialization::Process(int timeInMs)
{ // DO stuff?
	SleepThread(20);
	Graphics.renderQueried = true;
}

void Initialization::OnExit(AppState * nextState){
	std::cout<<"\nLeaving Initialization StateMan.";
	// Sets full-screen style if specified
	if (Graphics.shouldFullScreen){
#ifdef WINDOWS
		
		//SetWindowLongPtr(hWnd, GWL_STYLE,
		//	/*WS_SYSMENU |*/ WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
		//MoveWindow(hWnd, 0, 0, Graphics.ScreenWidth(), Graphics.ScreenHeight(), true);
#endif
	}
	/// Use loaded user settings here...
	int width = 800;
	int height = 600;
#ifdef WINDOWS
//	MoveWindow(hWnd, 0, 0, width, height, true);
#endif

}

