
#ifndef STATES_H
#define STATES_H

#include "GameStates.h"
#include "../Globals.h"
#include "../Selection.h"
#include "../UI/UIElement.h"
#include "../UI/UserInterface.h"
#include "Selection.h"
#include "Input/InputMapping.h"

#include <ctime>

class Packet;
struct Message;
struct GraphicsState;
struct ChatMessage;

void EmptyFunction();

class GameState {
	friend class StateManager;
	friend class InputManager;
public:
	GameState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~GameState();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(GameState * previousState) = 0;
	/// Main processing function, using provided time since last frame.
	virtual void Process(float time) = 0;
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(GameState * nextState) = 0;

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessPacket(Packet * packet);
	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

    /// Chat :3 Means that it has already been posted to the ChatManager.
	virtual void OnChatMessageReceived(ChatMessage * cm);

	/// Input functions for the various states
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	virtual void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	virtual void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	/// Interprets a mouse-move message to target position.
	virtual void MouseMove(int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	/** Handles mouse wheel input.
		Positive delta signifies scrolling upward or away from the user, negative being toward the user.
	*/
	virtual void MouseWheel(float delta);
	/// Callback from the Input-manager, query it for additional information as needed.
	virtual void KeyPressed(int keyCode, bool downBefore);

	/** Function to handle custom actions defined per state.
		This function is called by the various bindings that the state defines.
	*/
	virtual void InputProcessor(int action, int inputDevice = 0);
	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();
	/// Creates the user interface for this state
	virtual void CreateUserInterface();
	/** Attempts to free the resources used by the user interface before deleting it.
		Aborts and returns false if any errors occur along the way.
	*/
	bool DeallocateUserInterface();

	/// For handling drag-and-drop files.
	virtual void HandleDADFiles(List<String> & files);

	/// What happens.. when we rendar?!
	virtual void Render();

	/// Getter functions
	UserInterface * GetUI(){ return ui; };
	int GetID(){return id;};

	/// Boolean, set to true if you want the KeyPressed function to be called.
	bool keyPressedCallback;

protected:

	/// For further identification
	String name;
	/// Unique ID
	int id;
	// UI fluff
	UserInterface * ui;
	InputMapping inputMapping;
	/// Last update time in ms
	int64 lastTime;
private:

};

class Initialization : public GameState{
public:
	Initialization();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
};

class Exit : public GameState{
public:
	Exit();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState){
		// shouldn't be entered ever
	};
};




#endif
