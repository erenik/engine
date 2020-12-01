/// Emil Hedemalm
/// 2020-01-08
/// Controller input state code

#include "Gamepad.h"
#include "OS.h"
#include "Input/InputDevices.h"
#include "GamepadMessage.h"
#include "Message/MessageManager.h"
#include "Graphics/Messages/UI/GMNavigateUI.h"
#include "Graphics/Messages/UI/GMProceedUI.h"
#include "Input/InputManager.h"
#include "StateManager.h"

#ifdef WINDOWS
	#include <Xinput.h>
	#include <winerror.h>
#endif 

Gamepad::Gamepad()
	: durationLeftStick(0)
	, leftStickIterations(0)
{

}

using namespace InputDevice;

GamepadManager::GamepadManager() {

	secondsThresholdFirstNavigation = 0.25f;
	secondsBetweenNavigationIterations = 0.125f;
	accelerationFactor = 0.98f;

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

void GamepadManager::Update(float timeInSeconds) {
	/// Check for xbox controller input on windows and xbox o-o
#ifdef WINDOWS
	for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
		XINPUT_STATE tmpXInputState;
		DWORD result = XInputGetState(i, &tmpXInputState);

		// Skip if failed
		if (result != ERROR_SUCCESS)
			continue;

		static DWORD lastPacketNumber[4];
		Gamepad & gamepad = gamepadState[i];
		Gamepad & previousState = previousGamepadState[i];

		// Check if there's any new data avaiable at all
		if (tmpXInputState.dwPacketNumber == lastPacketNumber[i]) {
			OnNoUpdate(timeInSeconds, gamepad, previousGamepadState[i]);			
			continue;
		}
		lastPacketNumber[i] = tmpXInputState.dwPacketNumber;

		// Parse Xbox controller input data
		// http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference[0]input_gamepad%28v=vs.85%29.aspx
		XINPUT_GAMEPAD * input = &tmpXInputState.Gamepad;

		// Get float valuesl.....
#define XINPUT_THUMB_MAX	32768

		GamepadMessage * message = new GamepadMessage();
		message->index = i;

		gamepad.leftStick = Vector2f(((float)input->sThumbLX) / XINPUT_THUMB_MAX, ((float)input->sThumbLY) / XINPUT_THUMB_MAX);
		gamepad.rightStick = Vector2f(((float)input->sThumbRX) / XINPUT_THUMB_MAX, ((float)input->sThumbRY) / XINPUT_THUMB_MAX);

		gamepad.leftStickUp = gamepad.leftStick.y > 0.5f;
		gamepad.leftStickDown = gamepad.leftStick.y < -0.5f;
		gamepad.leftStickRight = gamepad.leftStick.x > 0.5f;
		gamepad.leftStickLeft = gamepad.leftStick.x < -0.5f;


		// Reset duration if needed.
		if (gamepad.leftStickUp != previousState.leftStickUp ||
			gamepad.leftStickDown != previousState.leftStickDown ||
			gamepad.leftStickLeft != previousState.leftStickLeft ||
			gamepad.leftStickRight != previousState.leftStickRight)
		{
			gamepad.durationLeftStick = 0;
			gamepad.leftStickIterations = 0;
		}


		gamepad.leftTrigger = input->bLeftTrigger;
		gamepad.rightTrigger = input->bRightTrigger;
		gamepad.menuButtonPressed = input->wButtons & XINPUT_GAMEPAD_START;
		gamepad.aButtonPressed = input->wButtons & XINPUT_GAMEPAD_A;
		gamepad.bButtonPressed = input->wButtons & XINPUT_GAMEPAD_B;
		gamepad.xButtonPressed = input->wButtons & XINPUT_GAMEPAD_X;
		gamepad.yButtonPressed = input->wButtons & XINPUT_GAMEPAD_Y;
		gamepad.leftButtonPressed = input->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
		gamepad.rightButtonPressed = input->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

		gamepad.dPadUpPressed = input->wButtons & XINPUT_GAMEPAD_DPAD_UP;
		gamepad.dPadDownPressed = input->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;

		message->previousState = previousState;

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

		message->dPadUpPressed = gamepad.dPadUpPressed && (previousState.dPadUpPressed != gamepad.dPadUpPressed);
		message->dPadDownPressed = gamepad.dPadDownPressed && (previousState.dPadDownPressed != gamepad.dPadDownPressed);

		MesMan.QueueMessage(message);

		// UI code if relevant
		PostUpdate(gamepad, previousState);

		// Update previous state to track changes.
		previousState = gamepad;

#endif
	};
}


void GamepadManager::OnNoUpdate(float timeInSeconds, Gamepad & state, Gamepad & previousState) {

	if (InputMan.NavigateUI()) {
		if (state.leftStickUp || state.leftStickDown || state.leftStickLeft || state.leftStickRight) {
			state.durationLeftStick += timeInSeconds;
			bool iterate = false;

//			if (state.leftStickIterations == 0) {
				//iterate = true;
	//			std::cout << "\nInitial state.durationLeftStick: " << state.durationLeftStick;
		//	}
			if (state.durationLeftStick > (secondsThresholdFirstNavigation +
				state.leftStickIterations * secondsBetweenNavigationIterations * pow(accelerationFactor, state.leftStickIterations))) {
				iterate = true;
				std::cout<<"\nRepeat state.durationLeftStick: "<< state.durationLeftStick;
			}
			
			if (iterate)
			{
				++state.leftStickIterations;
				QueueGraphics(new GMNavigateUI(state.leftStickUp?  NavigateDirection::Up : 
					state.leftStickDown? NavigateDirection::Down : 
					state.leftStickRight? NavigateDirection::Right : NavigateDirection::Left));
			}
		}
	}
}


void GamepadManager::PostUpdate(Gamepad & state, Gamepad & previousState) {
	if (InputMan.NavigateUI()) {

		if (state.dPadUpPressed) {
			QueueGraphics(new GMNavigateUI(NavigateDirection::Up));
		}
		if (state.dPadDownPressed) {
			QueueGraphics(new GMNavigateUI(NavigateDirection::Down));
		}

		if (state.leftStickUp && !previousState.leftStickUp) {
			QueueGraphics(new GMNavigateUI(NavigateDirection::Up));
			std::cout << "\nNavigate up post update";
		}
		else if (state.leftStickDown && !previousState.leftStickDown) {
			QueueGraphics(new GMNavigateUI(NavigateDirection::Down));
			std::cout << "\nNavigate down post update";
		}
		else if (state.leftStickRight && !previousState.leftStickRight)
			QueueGraphics(new GMNavigateUI(NavigateDirection::Right));
		else if (state.leftStickLeft && !previousState.leftStickLeft)
			QueueGraphics(new GMNavigateUI(NavigateDirection::Left));

		if (state.aButtonPressed && !previousState.aButtonPressed)
			QueueGraphics(new GMProceedUI());
		if (state.bButtonPressed && !previousState.bButtonPressed)
			QueueGraphics(new GMCancelUI());
	}
}


