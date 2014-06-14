/// Emil Hedemalm
/// 2013-12-13
/// Game state for combination of runes and rune-templates! Menu-intensive with some cool visualizations hopefully ;)

#ifndef RUNE_STATE_H
#define RUNE_STATE_H

#include "../RRGameState.h"

/// Game state for combination of runes and rune-templates! Menu-intensive with some cool visualizations hopefully ;)
class RuneState : public RRGameState {
public:
	RuneState();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(GameState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(GameState * nextState);
	/// Creates the user interface for this state
	void CreateUserInterface();
	void ProcessMessage(Message * message);

private:

	void UpdateTemplateName();
	/// For rune-template creation
	int primary, secondary;
	
};

#endif