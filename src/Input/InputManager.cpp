/// Emil Hedemalm
/// 2013-03-01

#include "InputManager.h"
#include "StateManager.h"
#include "Message/MessageManager.h"

#include "Message/Message.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "../UI/UserInterface.h"
#include "DefaultBindings.h"
#include <cstring>
#include "Window/WindowManager.h"
#include "Viewport.h"

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
// #include "SpaceRace/GameStates/Racing/Actions.h"

/// Global inputmanager
InputManager * InputManager::input = NULL;

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
void InputManager::Deallocate(){
	assert(input);
	delete(input);
	input = NULL;
}

InputManager::InputManager(){
	/// Default mouse data
	prevMouseX = prevMouseY = mouseX = mouseY = startMouseX = startMouseY = 0;
	lButtonDown = rButtonDown = false;
	mouseCameraState = NULL_STATE;
	acceptInput = false;
	mouseLocked = false;
	selectedInputBuffer = 0;
//	activeUIInputElement = NULL;
//	hoverElement = NULL;
//	clickElement = NULL;

#ifdef WINDOWS
	gamepadState = NULL;
	previousGamepadState = NULL;
#endif

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
}
InputManager::~InputManager(){
#ifdef WINDOWS
	if (gamepadState)
		delete[] gamepadState;
	gamepadState = NULL;
	if (previousGamepadState)
		delete[] previousGamepadState;
	previousGamepadState = NULL;
#endif
}

/// Initializes the manager by loading mappings, generating to defaults if needed.
void InputManager::Initialize(){
	if (!this->LoadMappings())
		this->LoadDefaults();
	for (int i = 0; i < KEY::TOTAL_KEYS; ++i){
		this->keyPressed[i] = false;
	}
	this->isInTextEnteringMode = false;
	insertionMode = true;
	/// Clear past buffers
	for (int i = 0; i < INPUT_BUFFERS; ++i){
		// memset(inputBuffers[i], 0 , BUFFER_SIZE * sizeof(wchar_t));
	}

#ifdef WINDOWS
	/// Enabled
//	XInputEnable(true);
	gamepadState = new Gamepad[4];
	previousGamepadState = new Gamepad[4];
#endif
}

/// Clears flags for all input keys. Returns amount of keys that were in need of resetting.
int InputManager::ClearInputFlags(){
	int inputsReset = 0;
	for (int i = 0; i < KEY::TOTAL_KEYS; ++i){
		if (this->keyPressed[i]){
			++inputsReset;
			// Flag it as false
			this->keyPressed[i] = false;
			// Execute anything needed when the key press state is lost too, though!
			this->KeyUp(this->keyPressed[i]);
		}
	}
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


/*
void InputManager::ActivateElement(){
	UserInterface * ui = RelevantUI();
	assert(ui);
	if (hoverElement == NULL){
		std::cout<<"\nINFO: Not hovering over any valid element! Aborting InputManager::ActivateElement()";
		return;
	}
	assert(hoverElement);
	ui->Click(hoverElement->posX, hoverElement->posY);
	ui->Activate();
}
*/

/// Fetches and updates the device states for all external controllers (if any)
void InputManager::UpdateDeviceStates(){
	/*
	/// Check for xbox controller input on windows and xbox o-o
#ifdef WINDOWS
	for (int i = 0; i < 4; ++i){
		XINPUT_STATE tmpXInputState;
		DWORD result = XInputGetState(i, &tmpXInputState);

		// Skip if failed
		if (result != ERROR_SUCCESS)
			continue;
	//	std::cout<<"\nXbox Controller "<<i+1<<" responding as intended? >:3";

		static DWORD lastPacketNumber[4];

		// Check if there's any new data avaiable at all
		if (tmpXInputState.dwPacketNumber == lastPacketNumber[i]){
	//		std::cout<<"\nOld data, skipping";
			continue;
		}
		lastPacketNumber[i] = tmpXInputState.dwPacketNumber;

		// Parse Xbox controller input data
		// http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinput_gamepad%28v=vs.85%29.aspx
		XINPUT_GAMEPAD * input = &tmpXInputState.Gamepad;

		// Get float valuesl.....
#define XINPUT_THUMB_MAX	32768

		gamepadState[i].leftStickX = ((float)input->sThumbLX) / XINPUT_THUMB_MAX;
		gamepadState[i].leftStickY = ((float)input->sThumbLX) / XINPUT_THUMB_MAX;
/*
		Gamepad * gamepad = &gamepadState[i];

		int inputDeviceIndex = GAME_PAD_1 + i;

		// Interpret Game sticks
		// First left Game stick!
		if (gamepad->leftStickX < -0.5f){
			StateMan.ActiveState()->InputProcessor(BEGIN_TURNING_LEFT, inputDeviceIndex);
		}
		else if (gamepad->leftStickX > 0.5f){
			StateMan.ActiveState()->InputProcessor(BEGIN_TURNING_RIGHT, inputDeviceIndex);
		}
		else {
			StateMan.ActiveState()->InputProcessor(STOP_TURNING_LEFT, inputDeviceIndex);
			StateMan.ActiveState()->InputProcessor(STOP_TURNING_RIGHT, inputDeviceIndex);
		}

		// Triggers (left, right)
		// Left trigger
		if (input->bLeftTrigger > 0){
			StateMan.ActiveState()->InputProcessor(BEGIN_BREAKING, inputDeviceIndex);
		}
		// Right trigger
		else if (input->bRightTrigger > 0){
			StateMan.ActiveState()->InputProcessor(BEGIN_ACCELERATION, inputDeviceIndex);
		}
		else
			StateMan.ActiveState()->InputProcessor(STOP_ACCELERATION, inputDeviceIndex);

		/// Button for le boostlur.
		if (input->wButtons & XINPUT_GAMEPAD_A)
			StateMan.ActiveState()->InputProcessor(BEGIN_BOOST, inputDeviceIndex);
		else
			StateMan.ActiveState()->InputProcessor(STOP_BOOST, inputDeviceIndex);

		// Reset position!
		if (input->wButtons & XINPUT_GAMEPAD_Y)
			StateMan.ActiveState()->InputProcessor(RESET_POSITION, inputDeviceIndex);

			*/
	/*
	}
#endif
	*/
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

/** Returns the specified input buffer.
	Index 0 refers to the new/most recent/active input buffer.
	Indices 1 to MAX_BUFFERS-1 refer to previously entered buffers.
*/
String InputManager::GetInputBuffer(int index){
	assert(false);
	/*
	assert(index >= -1 && index < INPUT_BUFFERS);
	if (index == -1)
		return inputBuffers[selectedInputBuffer];
	return inputBuffers[index];
	*/
	return "";
}


/** Attempts to parse a single integer from target string. If no string is provided the system
	will parse from the inputBuffers[0]. */
bool InputManager::ParseInt(int &integer, wchar_t * string){
	assert(false);
	/*
	char buffer[BUFFER_SIZE];
	if (string == NULL){
		/// Copy from standard input buffer
		wcstombs(buffer, inputBuffers[selectedInputBuffer], BUFFER_SIZE);
	}
	else
		wcstombs(buffer, string, BUFFER_SIZE);
	return ParseInt(integer, buffer);
	*/
	return false;
}

/** Attempts to parse a single integer from target string */
bool InputManager::ParseInt(int &integer, const char * string){
	assert(false);
	return false;
	/*
	try {
		char buf[InputManager::BUFFER_SIZE];
		strcpy(buf, string);
		char * cStrp;
		cStrp = strtok(buf, " ");
		if (cStrp){
			integer = atoi(buf);
		}
		return true;
	} catch (...) {
		std::cout<<"\nError reading input buffer";
	}
	return false;
	*/
}



/** Attempts to parse select amount of floats from the inputBuffer.
	Default floats is 3 for the general coordinate system x|y|z.
	Returns number of successfully parsed floats. */
int InputManager::ParseFloats(float * floats, int amount){
	assert(false);
	/*
	char cString[InputManager::BUFFER_SIZE];
	wchar_t * string = inputBuffers[selectedInputBuffer];
	wcstombs(cString, string, InputManager::BUFFER_SIZE);
	return ParseFloats(floats, amount, cString);
	*/
	return 0;
}

/** Attempts to parse select amount of floats from target string and places them into the provided array.
	If no string is provided the system	will parse from the inputBuffers[0].
	Default floats is 3 for the general coordinate system x|y|z. */
int InputManager::ParseFloats(float * floats, int amount, wchar_t * string){
	assert(false);
	/*
	// Copy over the buffer
	char cString[InputManager::BUFFER_SIZE];
	wcstombs(cString, string, InputManager::BUFFER_SIZE);
	return ParseFloats(floats, amount, cString);
	*/
	return 0;
}
/** Attempts to parse select amount of floats from target string and places them into the provided array.
	If no string is provided the system	will parse from the inputBuffers[0].
	Default floats is 3 for the general coordinate system x|y|z. */
int InputManager::ParseFloats(float * floats, int amount, char * string){
	int floatsRead = 0;
	try {
		// Parse until null-sign reached
		char * ptr = string;
		int length = strlen(string)+1;
		char floatBuffer[20];
		char * fBufP = floatBuffer;
		memset(floatBuffer, 0, 20);
		while (ptr < string+length){
			if (*ptr >= '0' && *ptr <= '9'){
				*fBufP = *ptr;
				++fBufP;
			}
			else if (*ptr == '.' || *ptr == ','){
				/// Check for existing decimal/comma
				for (char * s = floatBuffer; s < fBufP; ++s){
					if (*s == '.' || *s == ','){
						std::cout<<"ERROR: Two decimals or commas in single float.";
						return floatsRead;
					}
				}
				*fBufP = '.';		// Always make it a decimal though!
				++fBufP;
			}
			else if (*ptr == '-'){
				/// Check for numeral after the minus!
				if (*(ptr+1) >= '0' && *(ptr+1) <= '9'){
					*fBufP = *ptr;
					++fBufP;
				}
			}
			// If something else and the buffer has something in it: try buffer
			else if (fBufP > floatBuffer){
				floats[floatsRead] = (float)atof(floatBuffer);
				++floatsRead;
				memset(floatBuffer, 0, 20);		// Clear buffer and move back pointer again ^_^
				fBufP = floatBuffer;
				/// Return true if we found all values!
				if (floatsRead == amount)
					return floatsRead;
			}
			/// Break loop after we encounter a null-sign!
			if (*ptr == '\0')
				break;
			++ptr;
		}
		/// We have read values, but not all needed!
		return floatsRead;
	} catch (...) {
		std::cout<<"\nError reading input buffer";
	}
	return floatsRead;
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
// #define MOUSE_COORD_DEBUG			/// Activate to print mouse coordinates from all functions.
// #define UIELEMENT_TARGET_DEBUG		/// Activate to print active UI
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void InputManager::MouseClick(Window * window, bool down, int x, int y, UIElement * elementClicked){
	if (!acceptInput)
		return;
	/// If mouse is le locked, return
	if (mouseLocked)
		return;
    lButtonDown = down;
#ifdef MOUSE_COORD_DEBUG
	std::cout<<"\nMouseClick: "<<x<<" "<<y;
	if (down)
		std::cout<<" CLICK!";
	else
		std::cout<<" UP!";
#endif


	UIElement * element = NULL;
	UserInterface * userInterface = RelevantUI();	
	if (userInterface)
	{
		element = userInterface->Hover(x, y, true);
		userInterface->SetHoverElement(element);
	//	std::cout<<"InputMAnager:: OnMouse down: "<<down;
		if (userInterface && element){
			if (down){
				element = userInterface->Click(x, y, true);
		//		clickElement = element;
			}
			else if (!down)
			{
				/// Why click again?
				element = userInterface->Click(x, y, true);
				if (element){
					element->Activate();	
					if (element->activationMessage.Length() != 0){
						if (element->activationMessage.Type() == String::WIDE_CHAR)
							element->activationMessage.ConvertToChar();
						MesMan.QueueMessages(element->activationMessage, element);
					}
				}
				element = userInterface->Hover(x,y, true);
				userInterface->SetHoverElement(element);
			}
			if (element){
	//			std::cout<<"\nElement: "<<element->name<<" o-o";
				if (element->text)
					std::cout<<" "<<element->text;
			}
		}
		/// Don't process state stuff if we're inside an interactable element.
		if (element)
			return;
	}
	/// Inform the active state of the interaction
	StateMan.ActiveState()->MouseClick(window, down, x, y, element);
}
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void InputManager::MouseRightClick(Window * window, bool down, int x, int y, UIElement * elementClicked){
	if (!acceptInput)
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

	/// Inform the active state of the interaction
	StateMan.ActiveState()->MouseRightClick(window, down, x, y);

	// If navigating UI, interpret right-click as cancel/exit?
	if (this->navigateUI && !down){
		/// Only activate the cancel function if we are currently within a UI?
		if (HoverElement())
			this->UICancel();
		return;
	}

	UIElement * element = NULL;
	UserInterface * userInterface = StateMan.ActiveState()->GetUI();
	/*
	if (userInterface){
		if (down)
			element = RelevantUI()->Click((float)x,(float)y);
		else if (!down)
			element = RelevantUI()->Activate();
		if (element){
			std::cout<<"\nElement: "<<element->name<<" o-o";
		}
	}
	*/
}

/// Interprets a mouse-move message to target position.
void InputManager::MouseMove(Window * window, int x, int y)
{	
	if (!acceptInput)
		return;
	/// If mouse is le locked, return
	if (mouseLocked)
		return;

	/// Save coordinates
	this->mouseX = x;
	this->mouseY = y;

	/// If we have a global UI (system ui), process it first.
	UserInterface * userInterface = GetRelevantUIForWindow(window);
	UIElement * element = NULL;
	if (userInterface)
	{
		// Save old hover element...? wat
		UIElement * hoverElement = userInterface->GetHoverElement();
		element = userInterface->Hover(x, y, true);
		userInterface->SetHoverElement(element);		
		// This should fix so that the mouse cannot move the cursor if the underlying UI cannot later be activated.. ish.
//		if ((element && !element->highlightOnHover) || !element)
//			element = hoverElement;
	}

	/// If no active gamestate, return...
	GameState * currentState = StateMan.ActiveState();
	if (currentState)
	{
		currentState->MouseMove(window, x, y, lButtonDown, rButtonDown, element);
	}
	Graphics.QueryRender();
}
/** Handles mouse wheel input.
	Positive delta signifies scrolling upward or away from the user, negative being toward the user.
*/
void InputManager::MouseWheel(Window * window, float delta){
	if (!acceptInput)
		return;
	std::cout<<"\nMouseWheel: "<<delta;
	UserInterface * ui = GetRelevantUIForWindow(window);
	if (ui)
	{
		UIElement * element = ui->GetElementByPosition(mouseX, mouseY);
		if (element){
			delta *= 0.5f;
			if (KeyPressed(KEY::CTRL))
				delta *= 5;
	//		std::cout<<"\nWheeled over element: "<<element->name;
			bool scrolled = element->OnScroll(delta);
			if (scrolled)
				// Mark some variable... to pass to the MouseWheel of the state.
				;
		}
	}
	/// If no UI has been selected/animated, pass the message on to the stateManager
	StateMan.ActiveState()->MouseWheel(window, delta);
	/// Call to render if needed.
	Graphics.QueryRender();
}

//=======================================================//
/// Keyboard input
//=======================================================//
extern void generalInputProcessor(int action, int inputDevice = 0);
extern void debuggingInputProcessor(int action, int inputDevice = 0);

/// Evaluates if the active key generates any new events by looking at the relevant key bindings
void InputManager::EvaluateKeyPressed(int activeKeyCode, bool downBefore, UIElement * activeElement)
{
	/// Evaluate relevant key-bindings!
	Binding * binding;

//    std::cout<<"\nEvaluateKeyPressed called for key: "<<GetKeyString(activeKeyCode);

	/// Evaluate debug inputs first of all.
#if defined(DEBUG_INPUT_ENABLED)
	// And at last the debug one, if need be
	binding = debug.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
	if (binding){
		debuggingInputProcessor(binding->action, binding->inputDevice);
		return;
	}
#endif

	/// Let the general one process things first for certain events, yow.
	// Then the general one!
	binding = general.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
	if(binding){
		if (binding->action != -1)
			generalInputProcessor(binding->action, binding->inputDevice);
		else if (binding->stringAction.Length())
			MesMan.QueueMessages(binding->stringAction);
		return;
	}

	/// Check if we have an active ui element. If so don't fucking do anything.
	UserInterface * userInterface = RelevantUI();
	if (userInterface)
	{
		UIElement * hoverElement = userInterface->GetHoverElement();
		// UI-navigation if no element is active. Active elements have the responsibility to let go of their activity at the user's behest.
		if (navigateUI && !activeElement)
		{
			bool uiCommand = true;
			switch(activeKeyCode)
			{
				case KEY::BACKSPACE:
				case KEY::ESCAPE:
				{
					bool didSomething = UICancel();
					uiCommand = didSomething;
					/// Check navigate UI stuff after actually popping anything?
					if (!didSomething && !forceNavigateUI)
						navigateUI = false;
					break;
				}
				case KEY::UP:		UIUp();		break;
				case KEY::DOWN:		UIDown();	break;
				case KEY::LEFT:		UILeft();	break;
				case KEY::RIGHT:	UIRight();	break;
				case KEY::PG_UP: 
					uiCommand = UIPage(1.f); break;
				case KEY::PG_DOWN: 
					uiCommand = UIPage(-1.f); break;
				case KEY::TAB:
					if (keyPressed[KEY::SHIFT])
						UIPrevious();
					else
						UINext();
					break;
				case KEY::ENTER:
				{
					bool didSomething = UIProceed();
					uiCommand = didSomething;
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

	// Callback state for handling if applicable?
	GameState * state = StateMan.ActiveState();
	if (state && state->keyPressedCallback)
		state->KeyPressed(activeKeyCode, downBefore);

	// Then general one! o-o

	// The global one!
	if (StateMan.GlobalState()){
		binding = StateMan.GlobalState()->inputMapping.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
		if (binding){
			if (binding->action != -1)
				StateMan.GlobalState()->InputProcessor(binding->action, binding->inputDevice);
			else if (binding->stringAction.Length())
				MesMan.QueueMessages(binding->stringAction);
		//	return;
		}
	}

	// First the specific one!
	binding = StateMan.ActiveState()->inputMapping.EvaluateInput(activeKeyCode, this->keyPressed, downBefore);
	if (binding){
	//	std::cout<<"\nFound binding in activeState: "<<binding->name;
		if (binding->action != -1)
			StateMan.ActiveState()->InputProcessor(binding->action, binding->inputDevice);
		else if (binding->stringAction.Length())
		{
			std::cout<<"\nQueueing action: "<<binding->stringAction;
			MesMan.QueueMessages(binding->stringAction);
		}
	}
	Graphics.QueryRender();
}

/// Evaluates if the active key generates any new events by looking at the relevant key bindings
void InputManager::EvaluateKeyReleased(int activeKeyCode){
	if (!acceptInput)
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
	if(binding){
		if (binding->stopAction != -1)
			generalInputProcessor(binding->stopAction, binding->inputDevice);
		else if (binding->stringStopAction.Length())
			MesMan.QueueMessages(binding->stringStopAction);
		return;
	}

	// The global one!
	GameState * global = StateMan.GlobalState();
	if (global){
		binding = StateMan.GlobalState()->inputMapping.EvaluateKeyRelease(activeKeyCode, this->keyPressed);
		if (binding){
			if (binding->stopAction != -1)
				StateMan.GlobalState()->InputProcessor(binding->stopAction, binding->inputDevice);
			else if (binding->stringStopAction.Length())
				MesMan.QueueMessages(binding->stringStopAction);
			//	return;
		}
	}

	// First the specific one!
	GameState * activeGameState = StateMan.ActiveState();
	if (activeGameState){
		binding = activeGameState->inputMapping.EvaluateKeyRelease(activeKeyCode, this->keyPressed);
		if (binding){
			if (binding->stopAction != -1)
				StateMan.ActiveState()->InputProcessor(binding->stopAction, binding->inputDevice);
			else if (binding->stringStopAction.Length())
				MesMan.QueueMessages(binding->stringStopAction);
			return;
		}
	}
}

/// Processes char-code messages, primarily for writing, secondly for key-bindings.
void InputManager::Char(unsigned char asciiCode){
	if (!acceptInput)
		return;

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
	}
};

/// Processes key-presses of commanding-nature (CTRL, ALT, SHIFT, etc.)
void InputManager::KeyDown(int keyCode, bool downBefore){
	if (!acceptInput)
		return;
	if (!StateMan.ActiveState())
		return;

	keyPressed[keyCode] = true;
	UserInterface * ui = RelevantUI();
	UIElement * activeElement = NULL;
	if (ui)
	{
		UIElement * inputFocusElement = RelevantUI()->ActiveInputFocusElement();
		// Catch the codes there that don't get caught in WM_CHAR?
		if (inputFocusElement)
		{
			activeElement = inputFocusElement;
			/// Use the result somehow to determine if other actions can be triggered, too.
			int result = inputFocusElement->OnKeyDown(keyCode, downBefore);
			Graphics.QueryRender();
		}
	}

	keyPressed[keyCode] = true;
//	std::cout<<"\nKeyDown ^^";
	EvaluateKeyPressed(keyCode, downBefore, activeElement);
};

void InputManager::OnStopActiveInput(){
	/// Will be moved to the UIInput class for internal handling.
	assert(false);

	this->isInTextEnteringMode = false;
	SetActiveUIInputElement(NULL);
	UIElement * input = StateMan.ActiveState()->GetUI()->GetActiveElement();
	if (input){
		input->state &= ~UIState::ACTIVE;
		input->text.caretPosition = -1;
	}
}


/// Processes key-releases of commanding-nature (CTRL, ALT, SHIFT, etc.)
void InputManager::KeyUp(int keyCode){
	if (!acceptInput)
		return;
	keyPressed[keyCode] = false;
	EvaluateKeyReleased(keyCode);
};

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
//	UIElement * element;
	List<UIElement*> uiList;
	assert(ui);
	ui->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, uiList);
	/// Perform filtering by some other kind too. Like a priority variable?
	elements = uiList;
//	std::cout<<"\nUIGetRelevantElements: "<<elements.Size();
	return elements;
}

void InputManager::UIUp()
{
	std::cout<<"\nUIUp";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
	std::cout<<" Relevant elements: "<<relevantElements.Size();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL){
	//	std::cout<<"\nHoverElement null";
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->posY < element->posY)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			UIElement * desiredElement = hoverElement->GetUpNeighbour(NULL);
			if (desiredElement)
				element = desiredElement;
		}
		/*
		// If no preferred could be found, grab the closest UI within a 90 degree arc updwars relative to the active ne?
		if (!element){
			float minDist = 100000000;
			for (int i = 0; i < relevantElements.Size(); ++i){
				UIElement * e = relevantElements[i];
				Vector3f distVec = e->position - hoverElement->position;
			//	std::cout<<"\nDistVec: "<<distVec;
				Vector3f eToHoverNormalized = distVec.NormalizedCopy();
				float dotProduct = eToHoverNormalized.DotProduct(Vector3f(0,1,0));
				if (dotProduct < 0.5f)
					continue;
				float dist = distVec.Length();
			//	std::cout<<"\n element["<<i<<"], posY: "<<e->posY<<" dist: "<<dist<<" dotProduct: "<<dotProduct;
				if (dist > 0 &&  dist < minDist){
					element = e;
					minDist = dist;
				}
			}
			if (!element){
				if (cyclicY){
					// Remove the hover flag from the active element and re-do.
					hoverElement->RemoveState(UIState::HOVER);
					UIUp();
				}
				return;
			}
		}
		*/
	}
	/// Make sure the element is hoverable too.
	if (element && !element->hoverable){
		/// If not hoverable, check if the element has any children that are hoverable.
		std::cout<<"\nElement "<<element->name<<" lacking hoverable property, checking for any valid children.";
		List<UIElement*> hoverables;
        element->GetElementsByFlags(UIFlag::HOVERABLE, hoverables);
        /// Check sibling elements to the left
        if (hoverables.Size()){
            element = hoverables[0];
            /// Continue until a valid element is found. If none exist, perhaps check with the parent?
            while(element && !element->hoverable){
                if (element)
					element = ui->GetElementByName(element->upNeighbourName);
            }
            if (!element)
                return;
        }
        if (!element || !element->hoverable){
            std::cout<<"\nUnable to find any valid children.";
            return;
        }
	}
	ui->SetHoverElement(element);
//	hoverElement = ui->Hover(element->posX, element->posY);
}

void InputManager::UIDown()
{
	std::cout<<"\nUIDown";
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
			if (e->posY > element->posY)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			UIElement * desiredElement = hoverElement->GetDownNeighbour(NULL);
			if (desiredElement)
				element = desiredElement;
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;
	ui->SetHoverElement(element);
}
void InputManager::UILeft(){
	std::cout<<"\nUIUp";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
	std::cout<<" Relevant elements: "<<relevantElements.Size();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL){
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->posX > element->posX)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			UIElement * desiredElement = hoverElement->GetLeftNeighbour(NULL);
			if (desiredElement)
				element = desiredElement;
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;
	ui->SetHoverElement(element);
}
void InputManager::UIRight()
{
	std::cout<<"\nUIRight";
	UserInterface * ui = RelevantUI();
	List<UIElement*> relevantElements = UIGetRelevantElements();
	std::cout<<" Relevant elements: "<<relevantElements.Size();
	if (!relevantElements.Size())
		return;
	UIElement * element = NULL;
	UIElement * hoverElement = ui->GetHoverElement();
	if (hoverElement == NULL){
		element = relevantElements[0];
		/// Grab element furtherst down
		for (int i = 1; i < relevantElements.Size(); ++i){
			UIElement * e = relevantElements[i];
			if (e->posX < element->posX)
				element = e;
		}
	}
	// If we got a valid hover-element.
	else {
		// If not, or if the element couldn't find the named one, try and let it figure out for itself which one it would suggest for us!
		if (!element){
			UIElement * desiredElement = hoverElement->GetRightNeighbour(NULL);
			if (desiredElement)
				element = desiredElement;
		}
	}
	/// Skip if no valid elements were returned, keep the old one.
	if (!element)
		return;
	ui->SetHoverElement(element);
}

// Returns true if it did anything.
bool InputManager::UIPage(float amount)
{
	UserInterface * ui = RelevantUI();
	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement)
		return false;
	return hoverElement->OnScroll(amount);
}


/** Emulates Pressing a mouse-button or Enter-key in order to continue with whatever dialogue was up, using the selected or default option (if any)
	Returns true if it actually did something. False if e.g. no UI item was active or in hover state.
*/
bool InputManager::UIProceed(){
	UserInterface * ui = RelevantUI();
	UIElement * hoverElement = ui->GetHoverElement();
	if (!hoverElement)
		return false;
	/// Trigger active hover-element.
	ui->Activate(hoverElement);
	return true;
}


/// Returns true if it actually did something. False if all menus etc. already have been closed.
bool InputManager::UICancel()
{
	std::cout<<"\nUICancel";
	UserInterface * ui = RelevantUI();
	/// Fetch active ui in stack.
	UIElement * element = ui->GetStackTop();
	if (element->exitable == false)
		return false;
	/// Queue a message to remove it!
	Graphics.QueueMessage(new GMPopUI(element->name, ui));
	// Post onExit only when exiting it via UICancel for example!
	MesMan.QueueMessages(element->onExit);
	return true;
}

/// Similar to GoToNextElement above^
void InputManager::UINext()
{
	UserInterface * ui = RelevantUI();
	UIElement * element;
	UIElement * hoverElement = ui->GetHoverElement();
	List<UIElement*> uiList;
	assert(ui);
	ui->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, uiList);
	if (hoverElement == NULL){
		element = uiList[0];
		hoverElement = ui->Hover(element->posX, element->posY);
		assert(hoverElement);
	}
	else {
		int index = uiList.GetIndexOf(hoverElement);
		++index;
		if (index >= uiList.Size())
			element = uiList[0];
		else
			element = uiList[index];
		hoverElement = ui->Hover(element->posX, element->posY);
	}
}
void InputManager::UIPrevious()
{
	UserInterface * ui = RelevantUI();
	UIElement * element;
	UIElement * hoverElement = ui->GetHoverElement();
	List<UIElement*> uiList;
	assert(ui);
	ui->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, uiList);
	if (hoverElement == NULL){
		element = uiList[uiList.Size() -1 ];
		hoverElement = ui->Hover(element->posX, element->posY);
		assert(hoverElement);
	}
	else {
		int index = uiList.GetIndexOf(hoverElement);
		--index;
		if (index < 0)
			element = uiList[uiList.Size()-1];
		else
			element = uiList[index];
		hoverElement = ui->Hover(element->posX, element->posY);
	}
}

/// Returns the element that the cursor is currently hovering over.
UIElement * InputManager::HoverElement()
{
	UserInterface * ui = RelevantUI();
	if (!ui)
		return NULL;
	UIElement * element = ui->GetHoverElement();
	return element;
}

/// When set, will make certain keys only navigate the UI, by default arrow-keys, ENTER and Escape for PC.
void InputManager::NavigateUI(bool mode)
{
	if (forceNavigateUI)
		return;
	navigateUI = mode;
}

/// When set, nothing will disable the Navigate UI mode until this function is called again, at which point it is cancelled.
void InputManager::ForceNavigateUI(bool mode)
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
		case 2: ForceNavigateUI(true); break;
		case 1: NavigateUI(true); break;
		default: NavigateUI(false);
	}
}

/// Will push to stack target element in the active UI and also automatically try and hover on the primary/first element hoverable element within.
void InputManager::PushToStack(UIElement * element, UserInterface * ui)
{
	if (!ui || !element)
		return;
	/// Push it.
	int result = ui->PushToStack(element);
	if (result == UserInterface::NULL_ELEMENT)
		return;
	UIElement * firstActivatable = element->GetElementByFlag(UIFlag::HOVERABLE | UIFlag::ACTIVATABLE);
	if (!firstActivatable){
		std::cout<<"\nERROR: No activatable UI in the one pushed to stack just now?";
		return;
	}
	std::cout<<"\nHovering to element \""<<firstActivatable->name<<"\" with text \""<<firstActivatable->text<<"\"";
	ui->SetHoverElement(firstActivatable);

	// Set navigation cyclicity.
	cyclicY = element->cyclicY;
	//hoverElement = ui->Hover(firstActivatable->posX, firstActivatable->posY);
}

/// Pops the top-most UI from stack, also automatically tries to locate the previous hover-element for further interaction.
UIElement * InputManager::PopTopmostUIFromStack(UserInterface * ui)
{
	if (!ui)
		return NULL;
	UIElement * top = ui->GetStackTop();
	if (PopFromStack(top, ui))
		return top;
	return NULL;
}
/// Pops target element from stack, and also automatically tries to locate the previous hover-element!
UIElement * InputManager::PopFromStack(UIElement * element, UserInterface * ui, bool force /* = false*/)
{
	if (!ui || !element){
		std::cout<<"\nNull UI or element";
		return NULL;
	}
	// If trying to pop root, assume user is trying to cancel the UI-navigation mode?
	if (element->name == "root"){
		NavigateUI(false);
	}
	/// Pop it.
	bool success = ui->PopFromStack(element, force);
	if (!success){
		std::cout<<"\nUnable to pop UI from stack, might require force=true";
		return NULL;
	}

	// Set new navigation cyclicity.
	cyclicY = ui->GetStackTop()->cyclicY;

	UIElement * currentHover = ui->GetStackTop()->GetElementByState(UIState::HOVER);
	if (currentHover){
		ui->SetHoverElement(currentHover);
		return element;
	}
	UIElement * firstActivatable = element->GetElementByFlag(UIFlag::HOVERABLE | UIFlag::ACTIVATABLE);
	if (!firstActivatable){
		std::cout<<"\nERROR: No activatable UI in the one pushed to stack just now?";
		return element;
	}
	std::cout<<"\nHovering to element \""<<firstActivatable->name<<"\" with text \""<<firstActivatable->text<<"\"";
	ui->SetHoverElement(firstActivatable);

	/// If no activatable menu item is out, set navigate UI to false?
	/// No. Better embed this into the appropriate UI's onExit message!
	// Input.NavigateUI(true);
	return element;
}
