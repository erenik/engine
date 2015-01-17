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
	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	Input.debug.bindings.Add(2,
		new Binding(Action::FromEnum(RELOAD_UI), List<int>(3, KEY::CTRL, KEY::R, KEY::U)),
		new Binding(Action::FromEnum(RECOMPILE_SHADERS), List<int>(3, KEY::CTRL, KEY::R, KEY::S))
	);
};

