/// Emil Hedemalm
/// 2014-04-23
/// Some general scripts here.

#include "GeneralScripts.h"
#include "StateManager.h"

StateChanger::StateChanger(String line, Script * parent)
: Script(line, parent)
{
}
/// Regular state-machine mechanics for the events, since there might be several parralell events?
void StateChanger::OnBegin()
{
	String stateName = name.Tokenize("()")[1];
	// Queue the state.
	gs = StateMan.GetStateByName(stateName);
	StateMan.QueueState(gs);
}
void StateChanger::Process(float time)
{
	// Wait until the state has been entered too.
	GameState * activeState = StateMan.ActiveState();
	if (activeState == gs)
		state = Script::ENDING;
}

