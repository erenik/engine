
#ifndef STATES_H
#define STATES_H

#include "AppStates.h"
#include "../Globals.h"
#include "Entity/Entities.h"
#include "../UI/UIElement.h"
#include "../UI/UserInterface.h"
#include "Entity/Entities.h"
#include "Input/InputMapping.h"

#include <ctime>

class Packet;
class Message;
class GraphicsState;
class ChatMessage;
class Window;

void EmptyFunction();

class AppState {
	friend class StateManager;
	friend class InputManager;
public:
	AppState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~AppState();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState) = 0;
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs) = 0;
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState) = 0;

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
	virtual void MouseClick(Window * window, bool down, int x, int y, UIElement * elementClicked);
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	virtual void MouseRightClick(Window * window, bool down, int x, int y, UIElement * elementClicked);
	/// Interprets a mouse-move message to target position.
	virtual void MouseMove(Window * window, int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	/** Handles mouse wheel input.
		Positive delta signifies scrolling upward or away from the user, negative being toward the user.
	*/
	virtual void MouseWheel(Window * window, float delta);
	/// Callback from the Input-manager, query it for additional information as needed.
	virtual void KeyPressed(int keyCode, bool downBefore);

	/** Function to handle custom actions defined per state.
		This function is called by the various bindings that the state defines.
	*/
	virtual void InputProcessor(int action, int inputDevice = 0);
	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();
	/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
	virtual void CreateUserInterface();
	/** Attempts to free the resources used by the user interface before deleting it.
		Aborts and returns false if any errors occur along the way.
	*/
	bool DeallocateUserInterface();

	/// For handling drag-and-drop files.
	virtual void HandleDADFiles(List<String> & files);

	/// Called from the render-thread for every viewport/window, after the main rendering-pipeline has done its job.
	virtual void Render(GraphicsState * graphicsState);

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

class Initialization : public AppState{
public:
	Initialization();
	void OnEnter(AppState * previousState);
	void Process(int timeInMs);
	void OnExit(AppState * nextState);
};

class Exit : public AppState{
public:
	Exit();
	void OnEnter(AppState * previousState);
	void Process(int timeInMs);
	void OnExit(AppState * nextState){
		// shouldn't be entered ever
	};
};




#endif
