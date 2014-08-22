// Emil Hedemalm
// 2013-06-17

#ifndef CUTSCENE_H
#define CUTSCENE_H

#include "../RRGameState.h"
#include "AppStates/AppStates.h"
#include "Game/GameConstants.h"
class Script;

class CutsceneState : public RRGameState{
public:
	CutsceneState();
	~CutsceneState();
	void OnEnter(AppState * previousState);
	void Process(int timeInMs);
	void OnExit(AppState * nextState);
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
	/// Paused?! o-o
	bool paused;
	List<Script*> scriptsPaused;

	// Pauses all running scripts.
	void Pause();
	// Resumes all paused scripts.
	void Resume();


	/// Called once, in order to set player name and maybe notify of updates, etc?
	void OnFirstEnter();
	bool enteredOnce;

	/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
	virtual void NetworkLog(String message);

private:
	int requestedPlayers;
};

#endif // MAIN_MENU_H

