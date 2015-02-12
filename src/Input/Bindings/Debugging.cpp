/// Emil Hedemalm
/// 2015-01-17 (older originally)
/// Debugging key-bindings for things usually not wanted in final product, such as re-building UI or shaders in runtime.

#include <iostream>
#include "Graphics/Messages/GraphicsMessages.h"

#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Input/Action.h"

void debuggingInputProcessor(int action, int inputDevice){
}

/// Creates bindings that are used for debugging purposes only
void CreateDefaultDebuggingBindings()
{
	List<Binding*> & bindings = Input.debug.bindings;
#define BIND(a,b) bindings.Add(new Binding(a,b))

	BIND(Action::FromEnum(RELOAD_UI), List<int>(KEY::CTRL, KEY::R, KEY::U));
	BIND(Action::FromEnum(RECOMPILE_SHADERS), List<int>(KEY::CTRL, KEY::R, KEY::S));
	BIND(Action::FromString("InputMan.printHoverElement"), List<int>(KEY::P, KEY::H, KEY::E));
	BIND(Action::FromEnum(PRINT_SHADOW_MAPS), List<int>(KEY::P, KEY::S, KEY::M));
};

