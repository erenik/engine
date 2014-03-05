// Awesome Author

#include "../SpaceRaceGameState.h"
#include "Selection.h"

class SRSession;

class GlobalState : public SpaceRaceGameState {
public:
	GlobalState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~GlobalState();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);

	void ProcessPacket(Packet * packet);
	void ProcessMessage(Message * message);


	/// Hosts game, using provided variables in the state
    bool HostGame();
    /// Joins game, using provided variables in the state
	void JoinGame();


    /// Wosh.
//	void OnChatMessageReceived(ChatMessage * cm);

	void CreateDefaultBindings();
	virtual void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	virtual void MouseMove(float x, float y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	/// Interpret selection queries for additional processing (updating UI for example).
	void OnSelect(Selection &selection);

	/// Increases playback speed and notifies relevant systems of the change
	void IncreaseSpeed();
	/// Decreases playback speed and notifies relevant systems of the change
	void DecreaseSpeed();
private:

	void LoadPreferences();
	void SavePreferences();

	/// Updates relevant gui.
	void OnMasterVolumeUpdated();
	void OnAudioEnabledUpdated();

	/// Update the list
	void UpdatePeerUdpStatusUI();

	/// Updated as it goes.
	String networkStatus;
};

