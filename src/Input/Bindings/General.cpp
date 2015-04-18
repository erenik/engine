// Emil Hedemalm
// 2013-07-03 Linuxifying!

#include "Input/InputManager.h"
#include "Input/Action.h"

/*
#include "OS/Sleep.h"
#include "Player/PlayerManager.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/FrameStatistics.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Multimedia/MultimediaManager.h"
#include "Application/Application.h"
#include "Window/AppWindowManager.h"
#include "Viewport.h"

#include "Message/FileEvent.h"
#include "Message/MessageManager.h"
#include "Window/DragAndDrop.h"
*/
enum generalActions
{
	NULL_ACTION,


};


/// Linux
/*
#if defined LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <X11/Xatom.h>
extern AppWindow AppWindow;
extern Display * display;
#endif
*/

/// Creates bindings that are used for debugging purposes only
void CreateDefaultGeneralBindings()
{

	InputMapping * mapping = &Input.general;

	
	int ctrl = KEY::CTRL;

#define Bind1(a,b) mapping->bindings.Add(new Binding(a,b));
#define Bind2(a,b,c) mapping->bindings.Add(new Binding(a,b,c));
#define Bind3(a,b,c,d) mapping->bindings.Add(new Binding(a,b,c,d));

	// Always nice to be able to pause anywhere..
	Bind1(new Action("Pause/Break"), KEY::PAUSE_BREAK);
	// For toggling mouse input.
	Bind3(Action::FromString("IgnoreMouseInput"), KEY::CTRL, KEY::I, KEY::M);
	Bind3(Action::FromString("List cameras"), KEY::CTRL, KEY::L, KEY::C);
	Bind2(Action::FromEnum(TOGGLE_FULL_SCREEN), KEY::ALT, KEY::ENTER);
	Bind3(Action::FromEnum(RECORD_VIDEO), KEY::CTRL, KEY::R, KEY::V);
	Bind1(Action::FromEnum(PRINT_SCREENSHOT), KEY::PRINT_SCREEN);
	Bind2(Action::FromEnum(QUIT_APPLICATION), KEY::ALT, KEY::F4);
	Bind2(Action::FromEnum(PRINT_FRAME_TIME), KEY::CTRL, KEY::T);
	Bind3(Action::FromEnum(CYCLE_RENDER_PIPELINE), ctrl, KEY::R, KEY::PLUS);
	Bind3(Action::FromEnum(CYCLE_RENDER_PIPELINE_BACK), ctrl, KEY::R, KEY::MINUS);
	
	List<Binding*> & bindings = mapping->bindings;
#define BIND(a,b) bindings.Add(new Binding(a,b))

	BIND(Action::FromEnum(RELOAD_UI), List<int>(KEY::CTRL, KEY::R, KEY::U));
	BIND(Action::FromEnum(RECOMPILE_SHADERS), List<int>(KEY::CTRL, KEY::R, KEY::S));
	BIND(Action::FromString("InputMan.printHoverElement"), List<int>(KEY::P, KEY::H, KEY::E));
	BIND(Action::FromEnum(PRINT_SHADOW_MAPS), List<int>(KEY::P, KEY::S, KEY::M));
	BIND(Action::FromEnum(DEBUG_NEXT), KEY::F5);
	BIND(Action::FromEnum(DEBUG_PREVIOUS), KEY::F6);


	/*
	mapping->CreateBinding(CLOSE_WINDOW, ctrl, KEY::W);
	mapping->CreateBinding(OPEN_LIGHTING_EDITOR, ctrl, KEY::O, KEY::L);
	mapping->CreateBinding(TOGGLE_RENDER_PHYSICS, ctrl, KEY::R, KEY::P);
	mapping->CreateBinding(TOGGLE_RENDER_LIGHTS, ctrl, KEY::R, KEY::L);
	mapping->CreateBinding(TOGGLE_RENDER_WAYPOINTS, ctrl, KEY::R, KEY::W);
	mapping->CreateBinding(CYCLE_RENDER_PIPELINE, ctrl, KEY::R, KEY::PLUS);
	mapping->CreateBinding(CYCLE_RENDER_PIPELINE_BACK, ctrl, KEY::R, KEY::MINUS);

	Input.general.CreateBinding(RECORD_VIDEO, KEY::CTRL, KEY::R, KEY::V);
	Input.general.CreateBinding("PrintScreenshot", KEY::PRINT_SCREEN);
    Input.general.CreateBinding(PRINT_FRAME_TIME, KEY::CTRL, KEY::T);

	Input.general.CreateBinding(QUIT_APPLICATION, KEY::ALT, KEY::F4);
	//Input.general.CreateBinding(QUIT_APPLICATION, KEY::ALT, KEY::F4);
	Input.general.CreateBinding(PASTE, KEY::CTRL, KEY::V);
	Input.general.CreateBinding(COPY, KEY::CTRL, KEY::C);
	Input.general.CreateBinding(GO_TO_MAIN_MENU, KEY::CTRL, KEY::G, KEY::M);
	
	Input.general.CreateBinding(PRINT_TO_FILE, KEY::CTRL, KEY::P, KEY::F);

	Input.general.CreateBinding(RELOAD_MODELS, KEY::CTRL, KEY::R, KEY::M, "CTRL+R+M : Reload models");
	Input.general.CreateBinding(GO_TO_EDITOR, KEY::CTRL, KEY::G, KEY::E);

	Input.general.CreateBinding(PRINT_PLAYER_INPUT_DEVICES, KEY::L, KEY::I);
    Input.general.CreateBinding(LIST_TEXTURES, KEY::L, KEY::T);
    Input.general.CreateBinding(LIST_CAMERAS, KEY::L, KEY::C);
    Binding * binding = Input.general.CreateBinding(LIST_MODELS, KEY::L, KEY::M);
	Input.general.SetBlockingKeys(binding, KEY::CTRL);
    Input.general.CreateBinding(PRINT_UI_TREE, KEY::L, KEY::U);
	*/
};
