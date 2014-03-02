//
//#ifndef PATHFINDING_STATE_H
//#define PATHFINDING_STATE_H
//
//#include "../GameState.h"
//
//class Pathfinding : public GameState{
//public:
//	Pathfinding(){
//		id = GAME_STATE_PATHFINDING;
//	}
//	void OnEnter(GameState * previousState);
//	void Process(float time);
//	void OnExit(GameState * nextState);
//	/// Input functions for the various states
//	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
//	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
//	virtual void MouseMove(float x, float y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
//	void MouseWheel(float delta);
//
//	///
//	void CreateDefaultBindings();
//	void InputProcessor(int action, int inputDevice = 0);
//
//	/// Interpret selection queries for additional processing (updating UI for example).
//	void OnSelect(Selection &selection);
//};
//
//#endif
