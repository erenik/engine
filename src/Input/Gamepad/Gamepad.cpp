/// Emil Hedemalm
/// 2020-01-08
/// Controller input state code

#include "Gamepad.h"
#include "OS.h"
#include "Input/InputDevices.h"
#include "GamepadMessage.h"
#include "Message/MessageManager.h"

#ifdef WINDOWS
	#include <Xinput.h>
	#include <winerror.h>
#endif 

using namespace InputDevice;

GamepadManager::GamepadManager() {
#ifdef WINDOWS
	gamepadState = NULL;
	previousGamepadState = NULL;
#endif

#ifdef WINDOWS
	/// Enabled
	XInputEnable(true);
	gamepadState = new Gamepad[4];
	previousGamepadState = new Gamepad[4];
#endif
}

GamepadManager::~GamepadManager(){
#ifdef WINDOWS
	if (gamepadState)
		delete[] gamepadState;
	gamepadState = NULL;
	if (previousGamepadState)
		delete[] previousGamepadState;
	previousGamepadState = NULL;
#endif
}

void GamepadManager::Update() {
	/// Check for xbox controller input on windows and xbox o-o
#ifdef WINDOWS
	for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
		XINPUT_STATE tmpXInputState;
		DWORD result = XInputGetState(i, &tmpXInputState);

		// Skip if failed
		if (result != ERROR_SUCCESS)
			continue;
		//	std::cout<<"\nXbox Controller "<<i+1<<" responding as intended? >:3";

		static DWORD lastPacketNumber[4];

		// Check if there's any new data avaiable at all
		if (tmpXInputState.dwPacketNumber == lastPacketNumber[i]) {
			//		std::cout<<"\nOld data, skipping";
			continue;
		}
		lastPacketNumber[i] = tmpXInputState.dwPacketNumber;

		// Parse Xbox controller input data
		// http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference[0]input_gamepad%28v=vs.85%29.aspx
		XINPUT_GAMEPAD * input = &tmpXInputState.Gamepad;

		// Get float valuesl.....
#define XINPUT_THUMB_MAX	32768

		Gamepad & gamepad = gamepadState[i];
		GamepadMessage * message = new GamepadMessage();
		message->index = i;

		gamepad.leftStick = Vector2f(((float)input->sThumbLX) / XINPUT_THUMB_MAX, ((float)input->sThumbLY) / XINPUT_THUMB_MAX);
		gamepad.rightStick = Vector2f(((float)input->sThumbRX) / XINPUT_THUMB_MAX, ((float)input->sThumbRY) / XINPUT_THUMB_MAX);
		gamepad.leftTrigger = input->bLeftTrigger;
		gamepad.rightTrigger = input->bRightTrigger;
		gamepad.aButtonPressed = input->wButtons & XINPUT_GAMEPAD_A;
		gamepad.bButtonPressed = input->wButtons & XINPUT_GAMEPAD_B;
		gamepad.xButtonPressed = input->wButtons & XINPUT_GAMEPAD_X;
		gamepad.yButtonPressed = input->wButtons & XINPUT_GAMEPAD_Y;
		gamepad.leftButtonPressed = input->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
		gamepad.rightButtonPressed = input->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

		Gamepad & previousState = previousGamepadState[i];

		message->leftStickUpdated = previousState.leftStick != gamepad.leftStick;
		message->rightStickUpdated = previousState.rightStick != gamepad.leftStick;
		message->leftTriggerUpdated = previousState.leftTrigger != gamepad.leftTrigger;
		message->rightTriggerUpdated = previousState.rightTrigger != gamepad.rightTrigger;

		message->gamepadState = gamepad;

		message->aButtonPressed = gamepad.aButtonPressed && (previousState.aButtonPressed != gamepad.aButtonPressed);
		message->bButtonPressed = gamepad.bButtonPressed && (previousState.bButtonPressed != gamepad.bButtonPressed);
		message->xButtonPressed = gamepad.xButtonPressed && (previousState.xButtonPressed != gamepad.xButtonPressed);
		message->yButtonPressed = gamepad.yButtonPressed && (previousState.yButtonPressed != gamepad.yButtonPressed);
		message->leftButtonPressed = gamepad.leftButtonPressed && (previousState.leftButtonPressed != gamepad.leftButtonPressed);
		message->rightButtonPressed = gamepad.rightButtonPressed && (previousState.rightButtonPressed != gamepad.rightButtonPressed);

		// Update previous state to track changes.
		previousState = gamepad;

		MesMan.QueueMessage(message);
#endif
	};
}


