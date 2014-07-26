/// Emil Hedemalm
/// 2014-07-26
/// Simple flying aircraft game/simulator to test physics and possibly rendering 


#include "GameStates/GameState.h"

class AircraftState : public GameState 
{
public:
	AircraftState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~AircraftState();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(GameState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(GameState * nextState);

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
	virtual void MouseClick(Window * window, bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	virtual void MouseRightClick(Window * window, bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
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
	/// Creates the user interface for this state
	virtual void CreateUserInterface();
	/** Attempts to free the resources used by the user interface before deleting it.
		Aborts and returns false if any errors occur along the way.
	*/
	bool DeallocateUserInterface();

	/// For handling drag-and-drop files.
	virtual void HandleDADFiles(List<String> & files);

	/// What happens.. when we rendar?!
	virtual void Render(GraphicsState * graphicsState);

private:

	// 
	void CreateStars();

	void UpdateAcceleration();
	void UpdateLocalRotationX();
	void UpdateLocalRotationY();
	void UpdateLocalRotationZ();
	void UpdateLocalRotation();

	void NextCamera();

	// Our camaras. Good to test stuff.
	List<Camera*> cameras;

	Camera * firstPersonCamera;
	Camera * thirdPersonCamera;
	Camera * topDownCamera;
	Camera * freeFlyCamera;


	// For rotating the ship. Relative to the turning radii of the entity/ship in question and the distance that it travels.
	Vector3f relativeRot;

	List<Entity *> stars;
	Entity * ship;
};
