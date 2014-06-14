// Emil Hedemalm
// 2013-06-17

#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "../RRGameState.h"
#include "GameStates/GameStates.h"
#include "Game/GameConstants.h"

class MainMenu : public RRGameState{
public:
	MainMenu();
	~MainMenu();
	void OnEnter(GameState * previousState);
	void Process(int timeInMs);
	void OnExit(GameState * nextState);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	void ProcessMessage(Message * message);

	// For stuff
	void IncreaseRequestedPlayers();
	void DecreaseRequestedPlayers();

	// Stuff! o.o
	virtual void OnChatMessageReceived(ChatMessage * cm);

protected:
	// For updating lobby-gui.
	void OnPlayersUpdated();
	void OnPlayerReadyStateUpdated();

	/// Called once, in order to set player name and maybe notify of updates, etc?
	void OnFirstEnter();
	bool enteredOnce;

	/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
	virtual void NetworkLog(String message);

private:
	int requestedPlayers;
};

#endif // MAIN_MENU_H

