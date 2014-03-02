//#include "../GameState.h"
//#include "../../Selection.h"
//
//class AITest : public GameState{
//public:
//	AITest(){
//		id = GAME_STATE_AI_TEST;
//		playbackSpeed = 1.0f;
//	}
//	void OnEnter(GameState * previousState);
//	void Process(float time);
//	void OnExit(GameState * nextState);
//	/// Input functions for the various states
//	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
//	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
//	virtual void MouseMove(float x, float y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
//	void MouseWheel(float delta);
//	void CreateDefaultBindings();
//	void InputProcessor(int action, int inputDevice = 0);
//
//	/// Interpret selection queries for additional processing (updating UI for example).
//	void OnSelect(Selection &selection);
//
//	/// Increases playback speed and notifies relevant systems of the change
//	void IncreaseSpeed();
//	/// Decreases playback speed and notifies relevant systems of the change
//	void DecreaseSpeed();
//
//	float playbackSpeed;
//
//private:
//	Selection aiTestSelection;
//	Camera aiTestCamera;
//};
//
