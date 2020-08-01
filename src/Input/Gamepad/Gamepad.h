/// Emil Hedemalm
/// 2015-01-28
/// Controller input state code

#pragma once

#include "OS/OS.h"
#include "MathLib.h"

// A Gamepad struct for trying to encapsulate a "general"-type gamepad (Xbox, Playstation, etc.)
struct Gamepad {

	float leftTrigger;
	float rightTrigger;

	Vector2f leftStick, rightStick;
	
	bool aButtonPressed, bButtonPressed, xButtonPressed, yButtonPressed;
	bool leftButtonPressed, rightButtonPressed;
};


class GamepadManager {
public:
	GamepadManager();
	~GamepadManager();

	void Update();

private:
	// Xbox Controller structures
#ifdef WINDOWS
	Gamepad * gamepadState;
	Gamepad * previousGamepadState;
#endif

};


