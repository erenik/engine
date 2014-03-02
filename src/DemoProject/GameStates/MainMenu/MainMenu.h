// Emil Hedemalm
// 2013-06-17

#include "Game/GameType.h"

#ifdef DEMO_PROJECT

#include "GameStates/GameState.h"
#include "GameStates/GameStates.h"
#include "Game/GameConstants.h"

class MainMenu : public GameState{
public:
	MainMenu();
	/// Virtual destructor to discard everything appropriately.
	virtual ~MainMenu();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	void ProcessMessage(Message * message);

    /// Hosts game, using provided variables in the state
    void HostGame();
    /// Joins game, using provided variables in the state
	void JoinGame();

    /// Chat :3 Means that it has already been posted to the ChatManager.
    virtual void OnChatMessageReceived(ChatMessage * chatMessage);

	// For stuff
	void IncreaseRequestedPlayers();
	void DecreaseRequestedPlayers();
private:

    String hostPort, joinPort, joinIP, hostIP, hostPassword, joinPassword;

	int requestedPlayers;
};

#endif
