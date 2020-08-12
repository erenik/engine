// Emil Hedemalm
// 2020-08-01
// Message sent when gamepad state is updated.

#pragma once

#include "Message/Message.h"
#include "Gamepad.h"

class GamepadMessage : public Message {
public:
	GamepadMessage();
	int index; // 0 for first player, up to 1 (max 2) or 3 (max 4)
	Gamepad gamepadState, previousState;
	bool leftStickUpdated, rightStickUpdated,
		leftTriggerUpdated, rightTriggerUpdated,
		aButtonPressed, bButtonPressed,
		xButtonPressed, yButtonPressed,
		// The buttons above triggers
		leftButtonPressed, rightButtonPressed;

	Vector2f leftStickAccumulated;
};

