// Emil Hedemalm
// 2013-06-28

#include "StateManager.h"
#include "GameStates.h"

// Registers all states that will be used to the state manager upon startup
void RegisterStates(){
	StateMan.RegisterState(new DemoProjectGlobalState());
	StateMan.RegisterState(new MainMenu());
	StateMan.RegisterState(new PhysicsDemoState());
	StateMan.RegisterState(new StreamerTestState());
};
