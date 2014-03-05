// Awesome Author

#include "Game/GameType.h"

#ifdef RUNE_RPG

#include "GameStates/GameState.h"
#include "Selection.h"

#define TEST_BATTLE_PATH "data/RuneRPG/testBattle.txt"
#define BATTLERS_DIRECTORY "data/RuneRPG/Battlers/"
#define BATTLES_DIRECTORY	"data/RuneRPG/Battles/"
#define ACTIONS_DIRECTORY   "data/RuneRPG/BattleActions/"

class ScrollerGlobalState : public GameState{
public:
	ScrollerGlobalState();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);

	void ProcessPacket(Packet * packet);
	void ProcessMessage(Message * message);

	/// For key-bindings.
	void CreateDefaultBindings();
	/// For key-bindings.
	void InputProcessor(int action, int inputDevice = 0);

	void CreateUserInterface();
	
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

#endif
