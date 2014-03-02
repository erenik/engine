// Awesome Author

#include "Game/GameType.h"

//#ifdef DEMO_PROJECT

#include "GameStates/GameState.h"
#include "Selection.h"

class DemoProjectGlobalState : public GameState{
public:
	DemoProjectGlobalState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~DemoProjectGlobalState();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);

	void ProcessPacket(Packet * packet);
	void ProcessMessage(Message * message);

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
};
