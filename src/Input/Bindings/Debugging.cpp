
#include <iostream>
#include "Graphics/Messages/GraphicsMessages.h"

#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"


enum DebuggingActions{
	NULL_ACTION,
	RELOAD_UI,
	RECOMPILE_SHADERS,
	GO_TO_EDITOR,
	GO_TO_MAIN_MENU,
	GO_TO_AI_TEST,
};

void debuggingInputProcessor(int action, int inputDevice){
	switch(action){
		case RELOAD_UI:
			std::cout<<"\nInput>>RELOAD_UI";
			Graphics.QueueMessage(new GraphicsMessage(GM_RELOAD_UI));
			// Notify people of zis
			break;
		case RECOMPILE_SHADERS:
			std::cout<<"\nInput>>RECOMPILE_SHADERS";
			Graphics.QueueMessage(new GraphicsMessage(GM_RECOMPILE_SHADERS));
			break;
		case GO_TO_EDITOR:
			std::cout<<"\nInput>>GO_TO_EDITOR";
			StateMan.QueueState(StateMan.GetStateByID(GAME_STATE_EDITOR));
			break;
		case GO_TO_AI_TEST:
			std::cout<<"\nInput>>GO_TO_AI_TEST";
			StateMan.QueueState(StateMan.GetStateByID(GAME_STATE_AI_TEST));
			break;
	}
}

/// Creates bindings that are used for debugging purposes only
void CreateDefaultDebuggingBindings(){
	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	Input.debug.CreateBinding(RELOAD_UI, KEY::CTRL, KEY::R, KEY::U);
	Input.debug.CreateBinding(RECOMPILE_SHADERS, KEY::CTRL, KEY::R, KEY::S);
	Input.debug.CreateBinding(GO_TO_EDITOR, KEY::CTRL, KEY::G, KEY::E);
};
