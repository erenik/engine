// Emil Hedemalm
// 2013-07-10

#include "../SpaceRaceGameState.h"

class SRPlayer;

class LobbyState : public SpaceRaceGameState {
public:
	LobbyState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~LobbyState();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	void ProcessMessage(Message * message);
    /// Chat :3 Means that it has already been posted to the ChatManager.
	virtual void OnChatMessageReceived(ChatMessage * cm);

private:

	void OnGameSettingsLocked(bool locked);
	/// Host? Set on enter.
	bool isHost;

	void DisableButton(String elementName);
	void EnableButton(String elementName);

	// Update UI n such
	void OnPlayersUpdated();
	// Grab file-data
	void OnEnterLevelSelector();
	// Grab file-data
	void OnEnterShipSelector();

	int requestedPlayers;
	/// Before actually selecting it.
	String previewLevel;
	String selectedLevel;
	String previewShip;

	/// Index, set for selecting ship since pointers are dangerous!
	int shipSelector;
	// For when selecting ship, kicking, et al.
	SRPlayer * activePlayer;
};

