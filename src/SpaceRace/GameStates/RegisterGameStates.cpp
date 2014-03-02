// Emil Hedemalm
// 2013-06-28

#include "StateManager.h"
#include "GameStates.h"

// Registers all states that will be used to the state manager upon startup
void RegisterStates()
{
	StateMan.RegisterState(new GlobalState());
	StateMan.RegisterState(new MainMenu());
	StateMan.RegisterState(new EditorState());
	StateMan.RegisterState(new Racing());
	StateMan.RegisterState(new LobbyState());
	
};