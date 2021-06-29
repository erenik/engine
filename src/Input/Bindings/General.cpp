// Emil Hedemalm
// 2013-07-03 Linuxifying!

#include "Input/InputManager.h"
#include "Input/Action.h"

enum generalActions
{
	NULL_ACTION,
};

void Bind2Ex(Action * a, int b, int c) {
	InputMapping* mapping = &InputMan.general;
	Binding* binding = new Binding(a, b, c);
	binding->exclusive = true;
	mapping->bindings.Add(binding);
}


/// Creates bindings that are used for debugging purposes only
void CreateDefaultGeneralBindings()
{
	InputMapping * mapping = &InputMan.general;
	
	int ctrl = KEY::CTRL;

#define Bind1(a,b) mapping->bindings.Add(new Binding(a,b));
#define Bind2(a,b,c) mapping->bindings.Add(new Binding(a,b,c));
#define Bind3(a,b,c,d) mapping->bindings.Add(new Binding(a,b,c,d));

	// Always nice to be able to pause anywhere..
	Bind1(new Action("Pause/Break"), KEY::PAUSE_BREAK);
	// For toggling mouse input.
	Bind3(Action::FromString("IgnoreMouseInput"), KEY::CTRL, KEY::I, KEY::M);
	Bind3(Action::FromString("List cameras"), KEY::CTRL, KEY::L, KEY::C);
	Bind2Ex(Action::FromEnum(TOGGLE_FULL_SCREEN), KEY::ALT, KEY::ENTER);
	Bind3(Action::FromEnum(RECORD_VIDEO), KEY::CTRL, KEY::R, KEY::V);
	Bind1(Action::FromEnum(PRINT_SCREENSHOT), KEY::PRINT_SCREEN);
	Bind2(Action::FromEnum(QUIT_APPLICATION), KEY::ALT, KEY::F4);
	Bind2(Action::FromEnum(PRINT_FRAME_TIME), KEY::CTRL, KEY::T);
	Bind3(Action::FromEnum(CYCLE_RENDER_PIPELINE), ctrl, KEY::R, KEY::PLUS);
	Bind3(Action::FromEnum(CYCLE_RENDER_PIPELINE_BACK), ctrl, KEY::R, KEY::MINUS);
	
	Bind2(Action::FromEnum(PASTE), ctrl, KEY::V);
	Bind2(Action::FromEnum(COPY), ctrl, KEY::C);

	List<Binding*> & bindings = mapping->bindings;
#define BIND(a,b) bindings.Add(new Binding(a,b))

	BIND(Action::FromEnum(RELOAD_UI), List<int>(KEY::CTRL, KEY::R, KEY::U));
	BIND(Action::FromEnum(RECOMPILE_SHADERS), List<int>(KEY::CTRL, KEY::R, KEY::S));
	BIND(Action::FromEnum(RELOAD_TEXTURES), List<int>(KEY::CTRL, KEY::ALT, KEY::T));
	BIND(Action::FromString("InputMan.printHoverElement"), List<int>(KEY::P, KEY::H, KEY::E));
	BIND(Action::FromEnum(PRINT_SHADOW_MAPS), List<int>(KEY::P, KEY::S, KEY::M));
	BIND(Action::FromEnum(DEBUG_NEXT), KEY::F5);
	BIND(Action::FromEnum(DEBUG_PREVIOUS), KEY::F6);

	BIND(Action::FromEnum(LIGHTEN_BACKGROUND), List<int>(KEY::L, KEY::B));
	BIND(Action::FromEnum(DARKEN_BACKGROUND), List<int>(KEY::D, KEY::B));

};
