/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "AppStates/AppState.h"

class MORPG : public AppState 
{
public:
	MORPG();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(AppState * nextState);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

private:
	/// World map... 
	Entity * worldMapEntity;

};


