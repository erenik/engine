// Emil Hedemalm
// 2013-06-15

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "InputMapping.h"
#include "../Globals.h"
#include "AppStates/AppState.h"

#include "Entity/Entity.h"

#include <ctime>
#include <Util.h>

#include "Keys.h"
#include "OS/OS.h"
#include "Game/GameConstants.h"

#include "Input/InputDevices.h"

class Gamepad;
class GamepadManager;

#define InputMan (*InputManager::Instance())

/// Disable this for disabling debug commands like reloading UI/shaders
#ifdef DEBUGGER_ENABLED
	#define DEBUG_INPUT_ENABLED
#endif 

/// Binds all keys to actions (i.e. messages) depending on the current game state and key configuration
class InputManager {
private:
	InputManager();
	static InputManager * input;
public:
	static void Allocate();
	static InputManager * Instance();
	static void Deallocate();
	~InputManager();

#ifdef DEBUGGER_ENABLED
	/// Specific debug-mapping for testing purposes
	InputMapping debug;
#endif
	// A general mapping for world-wide commands (bringing up menus and UI navigation, etc)
	InputMapping general;

	/// For debugging.
	bool printHoverElement;

	void DisableKeyBindings();
	void EnableKeyBindings();

	/// Initializes the manager by loading mappings, generating to defaults if needed.
	void Initialize();
	/// Clears flags for all input keys. Returns amount of keys that were in need of resetting.
	int ClearInputFlags();

	/// Query if we've got any dialogue up that the player must/should respond to before continuing.
	bool DialogueActive();
	
	/// Returns list of all modifier keys (CTRL, SHIFT, etc.) currently being active/pressed.
	List<int> ActiveModifierKeys();

	/// Sets input focus, which currently just flags a boolean with its properties (if available and set up correctly).
	void SetInputFocus(EntitySharedPtr entity);

	/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
		If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
	*/
	bool HandleDADFiles(List<String> files);

	/// For manual control of assigning devices to players, each game can handle this as they choose, but the occupied variables here are pretty much global in scope, so 1 game at a time is the limit.
	void SetInputDeviceAvailability(int device, bool occupied);
	int GetNextAvailableInputDevice();

	/// Fetches and updates the device states for all external controllers (if any)
	void UpdateDeviceStates(float timeInSeconds);
	/// Clears mainly the keyPressedThisFrame array, used for checking newly pressed keys without specific bindings.
	void ClearPreviousFrameStats();

	/** Returns the flag value for it we're currenty entering text or not. */
	bool IsInTextEnteringMode() { return this->isInTextEnteringMode; };
	/** Enters the text-input mode, using a caret and everything to receive user input.
		Sends the actionToBeTakenUponCompletion down to the current state's processor
		for processing upon completion.
	*/
	void EnterTextInputMode(String actionToBeTakenUponCompletion);
	/// Sets the text of the input buffer.
	void SetInputBuffer(const char * text);
	/// For continually updating it's text, etc.
	void SetActiveUIInputElement(UIElement * e);
	/// Woo
	void OnElementDeleted(UIElement * element);
	/// Called from the GMSetUI message upon processing, at which point a new or old UI is displayed.
	void OnSetUI(UserInterface * ui);

	/** Returns the specified input buffer.
		Index 0 refers to the new/most recent/active input buffer.
		Indices 1 to MAX_BUFFERS-1 refer to previously entered buffers.
		Index -1 will return the current buffer which may be an older buffer.
	*/
	String GetInputBuffer(int index = -1);
	/// Prints input to stdConsole
	void PrintBuffer();

	/** Attempts to parse a single integer from target string. If no string is provided the system
		will parse from the inputBuffers[0]. */
	bool ParseInt(int &integer, wchar_t * string = NULL);
	/** Attempts to parse a single integer from target string */
	bool ParseInt(int &integer, const char * string);
	/** Attempts to parse select amount of floats from target string and places them into the provided array.
		Default floats is 3 for the general coordinate system x|y|z.
		Returns number of successfully parsed floats. */
	int ParseFloats(float * floats, int amount, wchar_t * string);
	int ParseFloats(float * floats, int amount, char * string);
	/** Attempts to parse select amount of floats from the inputBuffers[0].
		Default floats is 3 for the general coordinate system x|y|z.
		Returns number of successfully parsed floats. */
	int ParseFloats(float * floats, int amount = 3);


	/// Saves key bindings to file. Returns true upon success.
	bool SaveMappings();
	/// Loads mappings from pre-defined saved archive. Returns true upon success.
	bool LoadMappings();
	/// Lists all available input combinations for the current game StateMan.
	void ListMappings();

	/// Loads default key-mappings.
	void LoadDefaults();

	/// To enable/disable ui from e.g. cutscene-scripts. This does not disable the system-global UI!.
	void DisableActiveUI();
	void EnableActiveUI();

	//=======================================================//
	/// Mouse handling ~
	//=======================================================//
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	void MouseClick(AppWindow * AppWindow, bool down, int x, int y);
	/** Handles a mouse click.
		Argument true indicate that the button was pressed, while false indicates that it was just released.
		Default arguments for x and y indicate that they should not be refreshed.
	*/
	void MouseRightClick(AppWindow * AppWindow, bool down, int x, int y);
	/// Interprets a mouse-move message to target position.
	void MouseMove(AppWindow * AppWindow, Vector2i activeWindowAreaCoords);
	/** Handles mouse wheel input.
		Positive delta signifies scrolling upward or away from the user, negative being toward the user.
	*/
	void MouseWheel(AppWindow * AppWindow, float delta);

	Vector2i GetMousePosition();

	//=======================================================//
	/// Keyboard input
	//=======================================================//
	/// Processes char-code messages, primarily for writing!
	void Char(AppWindow * window, unsigned char asciiCode);
	/** Processes key-presses of commanding-nature (CTRL, ALT, SHIFT, etc.)
		Parameters:
			- keyCode		The engine defined key code, e.g: KEY::W, KEY::CONTROL, KEY::SHIFT
			- downBefore	Specifies if the key was down before, only relevant on key presses.
	*/
	void KeyDown(AppWindow * window, int keyCode, bool downBefore);
	/// Processes key-releases of commanding-nature (CTRL, ALT, SHIFT, etc.)
	void KeyUp(AppWindow * window, int keyCode);

	/// Returns state of the selected key
	bool KeyPressed(int keyCode);
	/// Returns true if the given key was pressed down this frame.
	bool KeyPressedThisFrame(int keyCode);

	/// Amount of previous input buffers stored for future reference/eased reusability.
	static const int INPUT_BUFFERS = 20;

	///================================================================///
	/// Rapid access variables for determining input in states later.
	/// These are all refreshed every input update.
	///================================================================///
	/// Mouse states
	bool lButtonDown;
	bool rButtonDown;
	/// Starting positions for when dragging mouse for selection, etc.
	int startMouseX, startMouseY;
	/// Previous mouse position
	int prevMouseX, prevMouseY;

	enum mouseCameraStates {
		NULL_STATE,
		ROTATING,
		PANNING
	};
	int mouseCameraState;
	/// Default false. Toggleable with CTRL+I+M. When true, all mouse input is disabled.
	bool ignoreMouse;
	
	/// If true, we are in a menu and should interpret events differently.
	bool NavigateUI();
	/// When set, will make certain keys only navigate the UI, by default arrow-keys, ENTER and Escape for PC.
	void SetNavigateUI(bool mode);
	/// When set, nothing will disable the Navigate UI mode until this function is called again, at which point it is cancelled.
	void SetForceNavigateUI(bool mode);
	/// Fetches state of the NavigateUI tool. 0 = disabled, 1 = enabled, 2 = force enabled.
	int NavigateUIState();
	/// Loads NavigateUI state using integer provided using NavigateUIState() function earlier.
	void LoadNavigateUIState(int state);
	
	
	/// Will push to stack target element in the active UI and also automatically try and hover on the primary/first element hoverable element within.
	void PushToStack(GraphicsState* graphicsState, UIElement * element, UserInterface * ui);
	/// Pops the top-most UI from stack, also automatically tries to locate the previous hover-element for further interaction, returning the element removed upon success.
	UIElement * PopTopmostUIFromStack(GraphicsState* graphicsState, UserInterface * ui);
	/// Pops target element from stack, and also automatically tries to locate the previous hover-element!
	UIElement * PopFromStack(GraphicsState* graphicsState, UIElement * element, UserInterface * ui, bool force = false);

	/// Returns the state of the mouse lock
	bool MouseLocked() const;
	void SetMouseLock(bool enable);

	///================================================================///
	/// For interacting with UI, be it simulated or not.
	///================================================================///
	List<UIElement*> UIGetRelevantElements();
	void UIUp();
	void UIDown();
	void UILeft();
	void UIRight();
	// For Page up/down, returns true if anything was done.
	void UIPage(float amount);
	/** Emulates Pressing a mouse-button or Enter-key in order to continue with whatever dialogue was up, using the selected or default option (if any)
		Returns true if it actually did something. False if e.g. no UI item was active or in hover state.
	*/
	bool UIProceed();
	/// Returns true if it actually did something. False if all menus etc. already have been closed.
	bool UICancel();
	/// Similar to GoToNextElement above^
	void UINext();
	void UIPrevious();
	
	/// Cyclic property for UI interaction. Can be nice at times, annoying at times.
	bool cyclicY;

	/// Returns the element that the cursor is currently hovering over.
	UIElement * HoverElement(); //{return hoverElement;};

private:
	List< std::shared_ptr<Entity> > inputFocusEntities;
	// To disable the current ui from e.g. cutscene-scripts.
	bool activeUIEnabled;

	/// Input device user-set availability. TODO: Add another array of booleans that check if the device is supported or connected currently?
	bool inputDeviceAvailability[InputDevice::MAX_INPUT_DEVICES];

	/// While toggled, will force all input to primarily navigate the UI.
	bool navigateUI;
	/// When toggled will force the UI to be on until disabled.
	bool forceNavigateUI;

	/// Default true. sets to false when an GUI input is currently active (may cause unwanted behaviour otherwise).
	bool keyBindingsEnabled;
	/// For disabling/enabling mouse-input!
	bool mouseLocked;

#define EvaluateButtonPressed EvaluateKeyPressed
#define EvaluateButtonReleased EvaluateKeyReleased

    /// Evaluates if the active key generates any new events by looking at the relevant key bindings. 
	void EvaluateKeyPressed(int activeKeyCode, bool downBefore, UIElement * activeElement);
	/// Evaluates if the active key generates any new events by looking at the relevant key bindings
	void EvaluateKeyReleased(int activeKeyCode);
	/// For handling text-input
	void OnBackspace();
	/// For updating UI
	void OnTextInputUpdated();

	/// Make it null and stuff
	void OnStopActiveInput();

	/// Input device
	int activePlayer; // Default the active player to first one.

	/// Flags for if the keys are pressed down currently
	bool keyPressed[KEY::TOTAL_KEYS];
	bool keyPressedThisFrame[KEY::TOTAL_KEYS];

	/// Currently selected previous input buffer
	int selectedInputBuffer;

	/// Flag for text-entering mode
	bool isInTextEnteringMode;
	/// Time we entered text input mode.
	clock_t textInputStartTime;
	/// Toggles the "Insert" button if text should be inserted or replaced. True is default
	bool insertionMode;
	/// Caret position for editing text properly.
	int caretPosition;
	/// Specifies the action to be sent down to the input-binding upon pressing Enter when in text-edit mode.
	String actionToBeTakenUponCompletion;

	/// Used to make user-experience more fluid.
	AppWindow * lastMouseMoveWindow;

	GamepadManager * gamepadManager;
};


#endif
