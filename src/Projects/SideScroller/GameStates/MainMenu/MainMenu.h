// Emil Hedemalm
// 2013-06-17

#include "Game/GameType.h"

#ifdef RUNE_RPG

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

	/// For generating ui.
	void CreateUserInterface();
	/// For key-bindings.
	void CreateDefaultBindings();
	/// For key-bindings.
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

#endif // RUNE_RPG