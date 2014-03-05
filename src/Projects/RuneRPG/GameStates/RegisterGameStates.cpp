// Emil Hedemalm
// 2013-06-28

#include "StateManager.h"
#include "GameStates.h"

// Registers all states that will be used to the state manager upon startup
void RegisterStates(){
	StateMan.RegisterState(new RuneGlobalState());
	StateMan.RegisterState(new MainMenu());
	StateMan.RegisterState(new RuneBattleState());
	StateMan.RegisterState(new MapState());
	StateMan.RegisterState(new RuneEditor());
	StateMan.RegisterState(new RuneState());
};