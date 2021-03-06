/// Emil Hedemalm
/// 2013-03-01

#include "InputManager.h"
#include "StateManager.h"
#include "Message/MessageManager.h"

#include "Message/Message.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/UIMessages.h"
#include "Graphics/Messages/UI/GMNavigateUI.h"
#include "Graphics/Messages/GMMouse.h"
#include "../UI/UserInterface.h"
#include "DefaultBindings.h"
#include <cstring>
#include "Window/AppWindowManager.h"
#include "Viewport.h"
#include "Entity/EntityProperty.h"
#include "InputState.h"
#include "Gamepad/Gamepad.h"

#include "File/LogFile.h"

//#include "../Managers.h"
#include "OS/OS.h"

#ifdef WINDOWS // For XBOX controller input
#include <Windows.h>
#ifdef _MSC_VER // MSVC++ specific header
#include <xinput.h>
#include <XInput.h>
#endif // _MSC_VER
#endif // WINDOWS

#include "Gamepad.h"

/// TODO: Implement gamepad bindings better?
// #include "SpaceRace/AppStates/Racing/Actions.h"

/// Global inputmanager
InputManager * InputManager::input = NULL;
int debug = 0;

/// Allocate
void InputManager::Allocate(){
	assert(input == NULL);
	input = new InputManager();
}
InputManager * InputManager::Instance(){
	assert(input);
	if (input == NULL)
	{
		std::cout<<"\nInputManager NULL!";
	}
	return input;
}

void InputManager::Deallocate()
{
	assert(input);
	delete(input);
	input = NULL;
}

InputManager::InputManager()
{

	gamepadManager = new GamepadManager();
	keyBindingsEnabled = true;
	printHoverElement = 0;
	/// Default mouse data
	prevMouseX = prevMouseY = startMouseX = startMouseY = 0;
	lButtonDown = rButtonDown = false;
	ignoreMouse = false;
	mouseCameraState = NULL_STATE;
	mouseLocked = false;
	selectedInputBuffer = 0;
//	activeUIInputElement = NULL;
//	hoverElement = NULL;
//	clickElement = NULL;

	// Default some inputs..!
	activePlayer = 0;
	/// While toggled, will force all input to primarily navigate the UI.
	navigateUI = false;
	forceNavigateUI = false;
	// Will be toggled often..
	cyclicY = true;

	/// Reset input device availability
	for (int i = 0; i < InputDevice::MAX_INPUT_DEVICES; ++i)
		inputDeviceAvailability[i] = false;

	/// Used to make user-experience more fluid.
	lastMouseMoveWindow = NULL;
}
InputManager::~InputManager(){
#ifdef WINDOWS
	delete gamepadManager;
#endif
}

void InputManager::DisableKeyBindings()
{
	keyBindingsEnabled = false;
}
void InputManager::EnableKeyBindings()
{
	keyBindingsEnabled = true;
}

/// Initializes the manager by loading mappings, generating to defaults if needed.
void InputManager::Initialize(){
	if (!this->LoadMappings())
		this->LoadDefaults();
	for (int i = 0; i < KEY::TOTAL_KEYS; ++i){
		this->keyPressed[i] = false;
		this->keyPressedThisFrame[i] = false;
	}
	this->isInTextEnteringMode = false;
	insertionMode = true;
	/// Clear past buffers
	for (int i = 0; i < INPUT_BUFFERS; ++i){
		// memset(inputBuffers[i], 0 , BUFFER_SIZE * sizeof(wchar_t));
	}
}

/// Clears flags for all input keys. Returns amount of keys that were in need of resetting.
int InputManager::ClearInputFlags()
{
	int inputsReset = 0;
	for (int i = 0; i < KEY::TOTAL_KEYS; ++i)
	{
		if (this->keyPressed[i])
		{
			++inputsReset;
			// Flag it as false
			this->keyPressed[i] = false;
			// Flag it as false
			this->keyPressedThisFrame[i] = false;
			// Execute anything needed when the key press state is lost too, though!
			this->KeyUp(MainWindow(), this->keyPressed[i]);
		}
	}
	LogMain("ClearInputFlags: Keys reset: " + String(inputsReset), INFO);
	return inputsReset;
}

/// Returns the state of the mouse lock
bool InputManager::MouseLocked() const {
	return mouseLocked;
}
void InputManager::SetMouseLock(bool enable){
	mouseLocked = enable;
}


/// Query if we've got any dialogue up that the player must/should respond to before continuing.
bool InputManager::DialogueActive(){
	UserInterface * ui = RelevantUI();
	/// Assume we only have to care about the UI of the state, not any specific viewports...
	UIElement * element = ui->GetElementByState(UIState::DIALOGUE);
	return element? true : false;
}

List<int> InputManager::ActiveModifierKeys()
{
	List<int> activeModifierKeys;
	List<int> modifierKeys;
	int modifierKeyArr[] = {
		KEY::CTRL, KEY::SHIFT, KEY::ALT, KEY::ALT_GR
	};
	modifierKeys.AddArray(4, modifierKeyArr);
	for (int i = 0; i < modifierKeys.Size(); ++i)
	{
		int key = modifierKeys[i];
		if (this->keyPressed[key])
			activeModifierKeys.Add(key);
	}
	return activeModifierKeys;
}

/// Sets input focus, which currently just flags a boolean with its properties (if available and set up correctly).
void InputManager::SetInputFocus(Entity* entity)
{
	/// Check if it can take it.
	bool canTakeInputFocus = false;
	if (entity)
	{
		for (int i = 0; i < entity->properties.Size(); ++i)
		{
			EntityProperty * ep = entity->properties[i];
			if (ep->inputFocusEnabled)
			{
				ep->inputFocus = true;
			}
		}
	}
	for (int i = 0; i < inputFocusEntities.Size(); ++i)
	{
		Entity* oldEntity = inputFocusEntities[i];
		for (int i = 0; i < oldEntity->properties.Size(); ++i)
		{
			EntityProperty * ep = oldEntity->properties[i];
			if (ep->inputFocusEnabled)
			{
				ep->inputFocus = false;
			}
		}
	}
	// clear old
	inputFocusEntities.Clear();
	// Add it.
	if (entity)
		inputFocusEntities.AddItem(entity);
	if (!canTakeInputFocus)
		return;
}


/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
	If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
*/
bool InputManager::HandleDADFiles(List<String> files)
{
	UIElement * element = HoverElement();
	if (!element)
		return false;
	bool handled = element->HandleDADFiles(files);
	return handled;
}


/// For manual control of assigning devices to players, each game can handle this as they choose, but the occupied variables here are pretty much global in scope, so 1 game at a time is the limit.
void InputManager::SetInputDeviceAvailability(int device, bool occupied)
{
	inputDeviceAvailability[device] = occupied;
}

int InputManager::GetNextAvailableInputDevice()
{
	for (int i = 0; i < InputDevice::MAX_INPUT_DEVICES; ++i)
		if (!inputDeviceAvailability[i])
			return i;
	return InputDevice::INVALID_DEVICE;
}

/// Fetches and updates the device states for all external controllers (if any)
void InputManager::UpdateDeviceStates(float timeInSeconds){
	// See Input/Gamepad/Gamepad.h
	gamepadManager->Update(timeInSeconds);
}

/// Clears mainly the keyPressedThisFrame array, used for checking newly pressed keys without specific bindings.
void InputManager::ClearPreviousFrameStats()
{
	for (int i = 0; i < KEY::TOTAL_KEYS; ++i)
	{
		keyPressedThisFrame[i] = false;
	}
}


/** DEPRECATING Enters the text-input mode, using a caret and everything to receive user input.
	Sends the actionToBeTakenUponCompletion down to the current state's processor
	for processing upon completion.
*/
void InputManager::EnterTextInputMode(String actionToBeTaken){
	/// Deprecated
	assert(false);
}

/// Sets the text of the input buffer.
void InputManager::SetInputBuffer(const char * text){
	assert(false);
	/*
	mbstowcs(inputBuffers[0], text, strlen(text)+1);
	caretPosition = strlen(text);
	*/
}

/// For continually updating it's text, etc.
void InputManager::SetActiveUIInputElement(UIElement * e){
	// Deprecate
	assert(false);
}

/// Woo
void InputManager::OnElementDeleted(UIElement * element){
	/// Pop it from stack if needed too.
	// assert(false && "Find other solution");
	//this->PopFromStack(element, true);
	std::cout<<"lol?";
	// RelevantUI()->OnElementDeleted(element);
}

/// Called from the GMSetUI message upon processing, at which point a new or old UI is displayed.
void InputManager::OnSetUI(UserInterface * ui){
	if (!ui)
		return;
	/*
	/// Fetch the active element (if there exists any), and set it to be our hover element!
	UIElement * stackTop = ui->GetStackTop();
	UIElement * uiHoverElement = stackTop->GetElementByState(UIState::HOVER);
	if (!uiHoverElement){
	//	hoverElement = NULL;
	}
	else {
	//	hoverElement = uiHoverElement;
	}
	*/
}

/// Loads mappings from pre-defined saved archive. Returns true upon success.
bool InputManager::LoadMappings(){
	// TODO: Implement loading and saving.
	std::cout<<"\nImplement InputManager::LoadMappings";
	return false;
};

/// Lists all available input combinations for the current game StateMan.
void InputManager::ListMappings(){
	assert(false);
	/*
	InputMapping * map = &inputMapping[StateMan.ActiveStateID()];
	std::cout<<"\n\nListing input mappings for current state: ";
	std::cout<<"\n-----------------------------------------";
	for (int i = 0; i < map->mappings; ++i){
		Binding * bind = map->bindings[i];
		std::cout<<"\n"<<bind->name;
	}*/
}

/// Loads default key-mappings.
void InputManager::LoadDefaults(){
#ifdef _DEBUG
	CreateDefaultDebuggingBindings();
#endif
	CreateDefaultGeneralBindings();
	CreateDefaultGameBindings();

	StateMan.CreateDefaultInputBindings();

};

/// To enable/disable ui from e.g. cutscene-scripts. This does not disable the system-global UI!.
void InputManager::DisableActiveUI()
{
	activeUIEnabled = false;
}
void InputManager::EnableActiveUI()
{
	activeUIEnabled = true;
}


//=======================================================//
/// Mouse handling ~
//=======================================================//
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void InputManager::MouseClick(AppWindow * appWindow, bool down, int x, int y)
{
	if (!inputState->acceptInput)
		return;
	if (ignoreMouse)
		return;
	/// If mouse is le locked, return
	if (mouseLocked)
		return;
    lButtonDown = down;
    LogMain("MouseClick x: "+String(x)+" y: "+String(y)+" down: "+down, EXTENSIVE_DEBUG);
	/// Move to graphics thread.
	QueueGraphics(new GMMouse(down? GMMouse::LDOWN : GMMouse::LUP, appWindow, Vector2i(x,y)));
	/// Inform the active state of the interaction
	StateMan.ActiveState()->MouseClick(appWindow, down, x, y, 0);
}
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void InputManager::MouseRightClick(AppWindow * appWindow, bool down, int x, int y)
{
	if (!inputState->acceptInput)
		return;
	if (ignoreMouse)
		return;
	/// If mouse is le locked, return
	if (mouseLocked)
		return;
    rButtonDown = down;
#ifdef MOUSE_COORD_DEBUG
	std::cout<<"\nMouseRightClick: "<<x<<" "<<y;
	if (down)
		std::cout<<" CLICK!";
	else
		std::cout<<" UP!";
#endif

	QueueGraphics(new GMMouse(down ? GMMouse::RDOWN : GMMouse::RUP, appWindow, Vector2i(x, y)));

	/// Inform the active state of the interaction
	StateMan.ActiveState()->MouseRightClick(appWindow, down, x, y, nullptr);

}

/// Interprets a mouse-move message to target position.
void InputManager::MouseMove(AppWindow * appWindow, Vector2i activeWindowAreaCoords)
{	
	int x = activeWindowAreaCoords[0];
	int y = activeWindowAreaCoords[1];
	if (!inputState->acceptInput)
		return;
	if (ignoreMouse)
		return;
	/// If mouse is le locked, return
	if (mouseLocked)
		return;
	lastMouseMoveWindow = appWindow;
	/// Save coordinates
	inputState->mousePosition = Vector2i(x,y);
	/// If we have a global UI (system ui), process it first.
	QueueGraphics(GMMouse::Move(appWindow, activeWindowAreaCoords));
	/// If no active gamestate, return...
	Graphics.QueryRender();
}

/** Handles mouse wheel input.
	Positive delta signifies scrolling upward or away from the user, negative being toward the user.
*/
void InputManager::MouseWheel(AppWindow * AppWindow, float delta)
{
	/// Use the AppWindow last passed to the MouseMove function, as the Mouse Wheel message (at least in windows) was not the same as the one hovering over.
#ifdef WINDOWS
	AppWindow = lastMouseMoveWindow;
#endif
	if (!inputState->acceptInput)
		return;
	if (ignoreMouse)
		return;
//	std::cout<<"\nMouseWheel: "<<delta;
	UserInterface * ui = GetRelevantUIForWindow(AppWindow);
	if (ui)
	{
		// Hover first.
		UIElement * element = ui->GetElementByPosition(inputState->mousePosition);
		if (element == nullptr)
			element = ui->GetHoverElement();
		if (element)
		{
			delta *= 2.f;
			if (KeyPressed(KEY::CTRL))
				delta *= 0.25f;
			QueueGraphics(new GMScrollUI(element->name, delta, ui));
			// Do a mouse hover/move too! - from the graphics thread, yeah.
		}
	}
	/// If no UI has been selected/animated, pass the message on to the stateManager
	StateMan.ActiveState()->MouseWheel(AppWindow, delta);
	
	/// Call to render if needed.
	Graphics.QueryRender();
}

Vector2i InputManager::GetMousePosition()
{
	return inputState->mousePosition;
}

//=======================================================//
/// Keyboard input
//=======================================================//
extern void generalInputProcessor(int action, int inputDevice = 0);
extern void debuggingInputProcessor(int action, int inputDevice = 0);

/// Evaluates if the active key generates any new events by looking at the relevant key bindings
void InputManager::EvaluateKeyPressed(int activeKeyCode, bool downBefore, UIElement * activeElement)
{
	/// Check if we have an active ui element.
	UserInterface * userInterface = RelevantUI();
	UIElement * hoverElement = NULL;
	if (userInterface)
		hoverElement = userInterface->GetHoverElement();

	/// Evaluate relevant key-bindings!
	Binding * binding;

	// First the specific one!
	bool fromGlobal = false;
	binding = StateMan.ActiveState()->inputMapping.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
	// The global one!
	if (!binding && StateMan.GlobalState())
	{
		binding = StateMan.GlobalState()->inputMapping.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
		fromGlobal = true;
	}
	if (binding && binding->exclusive)
		return;

	/// Evaluate debug inputs first of all.
#if defined(DEBUG_INPUT_ENABLED)
	// And at last the debug one, if need be
	binding = debug.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
	if (binding){
		return;
	}
#endif

	/// Let the general one process things first for certain events, yow.
	// Then the general one!
	binding = general.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
	if(binding)
	{
		goto endKeyPressed;
	}

	/// Check if we have an active ui element. If so, navigate it? -> Create actions for navigating UI and just handle them in a separate file?
	if (userInterface)
	{
		// UI-navigation if no element is active. Active elements have the responsibility to let go of their activity at the user's behest.
		if (navigateUI && !activeElement)
		{
		
			bool uiCommand = true;
			switch(activeKeyCode)
			{
			//	case KEY::BACKSPACE: - Not all too natural.
				case KEY::ESCAPE:
				{
					bool didSomething = UICancel();
					uiCommand = didSomething;
					// Nothing relevant?
					if (!didSomething)
					{
						AppWindow * activeWindow = ActiveWindow();
						// Are we in the main AppWindow? If not, make it active? (maybe should add a AppWindow-stack..? lol)
						if (activeWindow != MainWindow())
						{
							/// This should make it active.
							if (activeWindow->hideOnEsc)
								activeWindow->Hide();
							MainWindow()->BringToTop();
						}
					}
					/// Check navigate UI stuff after actually popping anything?
					if (!didSomething && !forceNavigateUI)
						navigateUI = false;
					break;
				}
				case KEY::UP:		
					QueueGraphics(new GMNavigateUI(NavigateDirection::Up));
					// UIUp();		
					break;
				case KEY::DOWN:
					QueueGraphics(new GMNavigateUI(NavigateDirection::Down));
					//UIDown();	
					break;
				case KEY::LEFT:
					QueueGraphics(new GMNavigateUI(NavigateDirection::Left));
					//UILeft();
					break;
				case KEY::RIGHT:
					QueueGraphics(new GMNavigateUI(NavigateDirection::Right));
					//UIRight();	
					break;
				case KEY::PG_UP:	UIPage(1.f); break;
				case KEY::PG_DOWN:	UIPage(-1.f); break;
				case KEY::TAB:
					if (keyPressed[KEY::SHIFT])
						UIPrevious();
					else
						UINext();
					break;
				case KEY::ENTER:
				{
					QueueGraphics(new GMProceedUI());
					// uiCommand = UIProceed();
					break;
				}
				default:
					uiCommand = false;
			}
			if (uiCommand){
				Graphics.QueryRender();
				return;
			}
		} // END: UI-navigation if focused.
	}
	
	// By default, don't process regular keybindings if there is an active ui element.
	if (activeElement)
		return;

endKeyPressed:

	// Callback state for handling if applicable?
	AppState * state = StateMan.ActiveState();
	if (state && state->keyPressedCallback)
		state->KeyPressed(activeKeyCode, downBefore);

	// Then general one! o-o
	Graphics.QueryRender();
}

/// Evaluates if the active key generates any new events by looking at the relevant key bindings
void InputManager::EvaluateKeyReleased(int activeKeyCode){
	if (!inputState->acceptInput)
		return;
	/// Evaluate relevant key-bindings!
	int action = 0;
	Binding * binding;
	// First the specific one!
//    std::cout<<"\nEvaluateKeyReleased called for key: "<<GetKeyString(activeKeyCode);

#if defined(DEBUG_INPUT_ENABLED)
	// And at last the debug one, if need be
	if (debug.EvaluateKeyRelease(activeKeyCode, this->keyPressed))
		return;
#endif
	// Then the general one!
	binding = general.EvaluateKeyRelease(activeKeyCode, this->keyPressed);
	if(binding)
		return;

	// The global one!
	AppState * global = StateMan.GlobalState();
	if (global){
		binding = StateMan.GlobalState()->inputMapping.EvaluateKeyRelease(activeKeyCode, this->keyPressed);
		if (binding)
			return;
	}

	// First the specific one!
	AppState * activeGameState = StateMan.ActiveState();
	if (activeGameState){
		binding = activeGameState->inputMapping.EvaluateKeyRelease(activeKeyCode, this->keyPressed);
	}
}

/// Processes char-code messages, primarily for writing, secondly for key-bindings.
void InputManager::Char(AppWindow * window, unsigned char asciiCode)
{
	if (!inputState->acceptInput)
		return;

	QueueGraphics(new GMChar(window, asciiCode));

	/*
	UserInterface * ui = RelevantUI();
	if (ui)
	{
		UIElement * inputFocusElement = ui->ActiveInputFocusElement();
		// Catch the codes there that don't get caught in WM_CHAR?
		if (inputFocusElement){
			/// Use the result somehow to determine if other actions can be triggered, too.
			int result = inputFocusElement->OnChar(asciiCode);
			return;
		}
	}*/
};

/// Processes key-presses of commanding-nature (CTRL, ALT, SHIFT, etc.)
void InputManager::KeyDown(AppWindow * window, int keyCode, bool downBefore)
{
	if (!inputState->acceptInput)
		return;
	if (!StateMan.ActiveState())
		return;

//	std::cout<<"\nKeydown: "<<keyCode;
	QueueGraphics(GMKey::Down(window, keyCode, downBefore));

	if (!keyBindingsEnabled)
		return;
	keyPressed[keyCode] = true;
	keyPressedThisFrame[keyCode] = true;
//	std::cout<<"\nKeyDown ^^";
	EvaluateKeyPressed(keyCode, downBefore, 0);
};

void InputManager::OnStopActiveInput()
{
	/// Will be moved to the UIInput class for internal handling.
	assert(false);
	this->isInTextEnteringMode = false;
	SetActiveUIInputElement(NULL);
	UIElement * input = StateMan.ActiveState()->GetUI()->GetActiveElement();
	if (input){
		input->state &= ~UIState::ACTIVE;
		input->text.OnInputStopped();
	}
}


/// Processes key-releases of commanding-nature (CTRL, ALT, SHIFT, etc.)
void InputManager::KeyUp(AppWindow * window, int keyCode)
{
	if (!inputState->acceptInput)
		return;
	keyPressed[keyCode] = false;
	if (!keyBindingsEnabled)
		return;
	EvaluateKeyReleased(keyCode);
};

/// Returns state of the selected key
bool InputManager::KeyPressed(int keyCode) 
{ 
#define CHECK_BAD_KEY if (keyCode > KEY::TOTAL_KEYS || keyCode < KEY::NULL_KEY) return false;
	CHECK_BAD_KEY;
	return keyPressed[keyCode]; 
}

/// Returns true if the given key was pressed down this frame.
bool InputManager::KeyPressedThisFrame(int keyCode)
{
	CHECK_BAD_KEY;
	return keyPressedThisFrame[keyCode];
}


/// Prints input to stdConsole
void InputManager::PrintBuffer(){
	assert(false);
	/*
	std::cout<<"\n";
	for (int i = 0; i < (int)wcslen(inputBuffers[selectedInputBuffer])+1; ++i){
		if (i == caretPosition)
			std::wcout<<"|";
		std::wcout<<inputBuffers[selectedInputBuffer][i];
	}
	*/
}

/// Deprecated input device functions!
// For adjusting local-player input-settings
/*
void InputManager::SetActivePlayer(int player) {
	assert(player >= 0);
	if (player >= MAX_PLAYERS)
		player = 0;
	activePlayer = player;
}
//void InputManager::GetActivePlayer() const { return activePlayer; }
// Adjusting active player input-device.
void InputManager::SetActivePlayerInputDevice(int device){
	assert(device > NULL_TYPE);
	if (device >= MAX_INPUT_DEVICES)
		device = NULL_TYPE;
	playerInputDevice[activePlayer] = device;
}
int InputManager::GetActivePlayerInputDevice() const {
	return playerInputDevice[activePlayer];
}
/// Returns the player(s) bound to the current input-device.
int InputManager::GetPlayerByActiveInputDevice(int deviceID){
	for (int i = 0; i < MAX_PLAYERS; ++i){
		if (playerInputDevice[i] == deviceID)
			return i;
	}
	return -1;
}
String InputManager::GetActivePlayerInputDeviceString() const {
	switch(playerInputDevice[activePlayer]){
		case NULL_TYPE:
			return "None/Default/Global";
		case KEYBOARD_1:
			return "Keyboard 1";
		case KEYBOARD_2:
			return "Keyboard 2";
		case GAME_PAD_1:
			return "Gamepad 1";
		case GAME_PAD_2:
			return "Gamepad 2";
		case GAME_PAD_3:
			return "Gamepad 3";
		case GAME_PAD_4:
			return "Gamepad 4";
		default:
			return "Unknown";
	}
}
*/

/// For handling text-input
void InputManager::OnBackspace(){
    /// Will be moved to the UIInput class for internal handling.
	assert(false);
	/*
	if (caretPosition > 0){
        --caretPosition;	// Move back caret
        // ...and move back the rest one step
        for (int i = caretPosition; i < (int)wcslen(inputBuffers[selectedInputBuffer]); ++i){
            inputBuffers[selectedInputBuffer][i] = inputBuffers[selectedInputBuffer][i+1];
        }
        OnTextInputUpdated();
    }*/
}

/// For updating UI
void InputManager::OnTextInputUpdated(){
	/// Will be moved to the UIInput class for internal handling.
	assert(false);
}

//====================================================================================
/// For interacting with UI, be it simulated or not.
//====================================================================================
List<UIElement*> InputManager::UIGetRelevantElements()
{
	List<UIElement*> elements;
	UserInterface * ui = RelevantUI();
	if (!ui)
		return elements;
	return ui->GetRelevantElements();
}

bool IsNavigatable(UIElement * element)
{
	if (!element)
		return false;
	if (!element->IsVisible())
		return false;
	if (!element->interaction.activateable)
		return false;
	return true;
}

void InputManager::UIUp()
{
//	std::cout<<"\nUIDown";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL)
	{
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->layout.posY > element->layout.posY)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			bool searchChildrenOnly = false;
			UIElement * desiredElement = hoverElement->GetUpNeighbour(nullptr, NULL, searchChildrenOnly);
			if (IsNavigatable(desiredElement))
			{
				element = desiredElement;
			}
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;

	QueueGraphics(new GMSetHoverUI(element->name));
}

void InputManager::UIDown()
{
//	std::cout<<"\nUIDown";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL){
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->layout.posY > element->layout.posY)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			bool searchChildrenOnly = false;
			UIElement * desiredElement = hoverElement->GetDownNeighbour(nullptr, NULL, searchChildrenOnly);
			if (IsNavigatable(desiredElement))
				element = desiredElement;
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;
	QueueGraphics(new GMSetHoverUI(element->name));
}
void InputManager::UILeft()
{
//	std::cout<<"\nUIUp";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
//	std::cout<<" Relevant elements: "<<relevantElements.Size();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL){
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->layout.posX > element->layout.posX)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			bool searchChildrenOnly = false;
			UIElement * desiredElement = hoverElement->GetLeftNeighbour(NULL, searchChildrenOnly);
			if (IsNavigatable(desiredElement))
				element = desiredElement;
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;
	QueueGraphics(new GMSetHoverUI(element->name));
}
void InputManager::UIRight()
{
//	std::cout<<"\nUIRight";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
//	std::cout<<" Relevant elements: "<<relevantElements.Size();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL){
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->layout.posX < element->layout.posX)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			bool searchChildrenOnly = false;
			UIElement * desiredElement = hoverElement->GetRightNeighbour(NULL, searchChildrenOnly);
			if (IsNavigatable(desiredElement))
				element = desiredElement;
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;
	QueueGraphics(new GMSetHoverUI(element->name));
}

// Returns true if it did anything.
void InputManager::UIPage(float amount)
{
	UserInterface * ui = RelevantUI();
	// TODO: Add a UIElement::UIPage function, which scrolls up or down a page (if inside a UIList), updating the active hover item.
	/// Then update the list position and stuff as is done with regular hover and UI-navigation functions!
	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement)
		return;
	// Get position of hoverelement before scroll
	QueueGraphics(new GMScrollUI(hoverElement->name, amount, ui));
}


/** Emulates Pressing a mouse-button or Enter-key in order to continue with whatever dialogue was up, using the selected or default option (if any)
	Returns true if it actually did something. False if e.g. no UI item was active or in hover state.
*/
bool InputManager::UIProceed()
{
	UserInterface * ui = RelevantUI();
	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement)
		return false;
	/// Set it as active if needed.
	QueueGraphics(new GMProceedUI());
	//hoverElement->AddState(UIState::ACTIVE);
	/// Activate it.
//	hoverElement->Activate(nullptr);
	/// Trigger active hover-element.
//	ui->Activate(hoverElement);
	return true;
}


/// Returns true if it actually did something. False if all menus etc. already have been closed.
bool InputManager::UICancel()
{
	Graphics.QueueMessage(new GMCancelUI());
	return true;
	/*
	std::cout<<"\nUICancel";
	UserInterface * ui = RelevantUI();
	/// Fetch active ui in stack.
	UIElement * element = ui->GetStackTop();
	if (element->interaction.exitable == false)
		return false;
	/// Queue a message to remove it!
	Graphics.QueueMessage(new GMPopUI(element->name, ui));
	// Post onExit only when exiting it via UICancel for example!
	MesMan.QueueMessages(element->interaction.onExit);
	return true;
	*/
}

/// Similar to GoToNextElement above^
void InputManager::UINext()
{
	std::cout << "\nMove to graphics thread.";
	assert(false && "re-implement this by sendinga message to graphics thread");
	return;
	/*
	UserInterface * ui = RelevantUI();
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	List<UIElement*> uiList;
	assert(ui);
	ui->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, uiList);
	if (hoverElement == NULL)
	{
		if (uiList.Size())
			element = uiList[0];
		hoverElement = ui->Hover(element->layout.posX, element->layout.posY);
		assert(hoverElement);
	}
	else {
		int index = uiList.GetIndexOf(hoverElement);
		++index;
		if (index >= uiList.Size())
			element = uiList[0];
		else
			element = uiList[index];
		hoverElement = ui->Hover(element->layout.posX, element->layout.posY);
	}
	*/
}
void InputManager::UIPrevious()
{
	// Move to graphics thread.
	std::cout<<"\nMove to graphics thread.";
	assert(false && "re-implement this by sendinga message to graphics thread");

	/*
	UserInterface * ui = RelevantUI();
	UIElement * element;
	UIElement * hoverElement = ui->GetHoverElement();
	List<UIElement*> uiList;
	assert(ui);
	ui->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, uiList);
	if (hoverElement == NULL){
		element = uiList[uiList.Size() -1 ];
		hoverElement = ui->Hover(element->layout.posX, element->layout.posY);
		assert(hoverElement);
	}
	else {
		int index = uiList.GetIndexOf(hoverElement);
		--index;
		if (index < 0)
			element = uiList[uiList.Size()-1];
		else
			element = uiList[index];
		hoverElement = ui->Hover(element->layout.posX, element->layout.posY);
	}*/
}

/// Returns the element that the cursor is currently hovering over.
UIElement * InputManager::HoverElement()
{
	UserInterface * ui = HoverUI();
	if (!ui)
		return NULL;
	UIElement * element = ui->GetHoverElement();
	return element;
}

bool InputManager::NavigateUI() {
	return navigateUI || forceNavigateUI;
}

/// When set, will make certain keys only navigate the UI, by default arrow-keys, ENTER and Escape for PC.
void InputManager::SetNavigateUI(bool mode)
{
	if (forceNavigateUI)
		return;
	navigateUI = mode;
}

/// When set, nothing will disable the Navigate UI mode until this function is called again, at which point it is cancelled.
void InputManager::SetForceNavigateUI(bool mode)
{
	navigateUI = forceNavigateUI = mode;
}

/// Fetches state of the NavigateUI tool. 0 = disabled, 1 = enabled, 2 = force enabled.
int InputManager::NavigateUIState()
{
	if (forceNavigateUI && navigateUI)
		return 2;
	else if (navigateUI)
		return 1;
	return 0;
}

/// Loads NavigateUI state using integer provided using NavigateUIState() function earlier.
void InputManager::LoadNavigateUIState(int state)
{
	switch(state){
		case 2: SetForceNavigateUI(true); break;
		case 1: SetNavigateUI(true); break;
		default: SetNavigateUI(false);
	}
}

/// Pops the top-most UI from stack, also automatically tries to locate the previous hover-element for further interaction.
UIElement * InputManager::PopTopmostUIFromStack(GraphicsState* graphicsState, UserInterface * ui)
{
	if (!ui)
		return NULL;
	UIElement * top = ui->GetStackTop();
	if (PopFromStack(graphicsState, top, ui)) {
		return top;
	}
	return NULL;
}
/// Pops target element from stack, and also automatically tries to locate the previous hover-element!
UIElement * InputManager::PopFromStack(GraphicsState* graphicsState, UIElement * element, UserInterface * ui, bool force /* = false*/)
{
	assert(false); 
	return nullptr;
	/*
	if (!ui || !element){
		std::cout<<"\nNull UI or element";
		return NULL;
	}
	// If trying to pop root, assume user is trying to cancel the UI-navigation mode?
	if (element->name == "root"){
		SetNavigateUI(false);
	}
	/// Pop it.
	bool success = ui->PopFromStack(graphicsState, element, force);
	if (!success){
		LogGraphics("Unable to pop UI from stack, might require force=true", FATAL);
		return NULL;
	}

	ui->PrintStack();

	// Set new navigation cyclicity.
	UIElement * stackTop = ui->GetStackTop();
	cyclicY = stackTop->interaction.cyclicY;

	// If element was previously active, remove it, keep only hover.
	stackTop->RemoveState(UIState::ACTIVE, true);

	UIElement * currentHover = stackTop->GetElementByState(UIState::HOVER);
	if (currentHover){
		ui->SetHoverElement(graphicsState, currentHover);
		return element;
	}
	UIElement * firstActivatable = stackTop->GetElementByFlag(UIFlag::HOVERABLE | UIFlag::ACTIVATABLE);
//	std::cout<<"\nHovering to element \""<<firstActivatable->name<<"\" with text \""<<firstActivatable->text<<"\"";
	ui->SetHoverElement(graphicsState, firstActivatable);

	/// If no activatable menu item is out, set navigate UI to false?
	/// No. Better embed this into the appropriate UI's onExit message!
	// InputMan.SetNavigateUI(true);
	return element;
	*/
}
