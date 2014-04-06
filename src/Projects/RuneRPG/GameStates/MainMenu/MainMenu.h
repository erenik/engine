// Emil Hedemalm
// 2013-06-17

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "GameStates/GameState.h"
#include "GameStates/GameStates.h"
#include "Game/GameConstants.h"

class MainMenu : public GameState{
public:
	MainMenu();
	~MainMenu();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	void ProcessMessage(Message * message);

	// For stuff
	void IncreaseRequestedPlayers();
	void DecreaseRequestedPlayers();
private:
	int requestedPlayers;
};

#endif // MAIN_MENU_H

