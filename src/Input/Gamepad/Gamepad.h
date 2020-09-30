/// Emil Hedemalm
/// 2015-01-28
/// Controller input state code

#pragma once

#include "OS/OS.h"
#include "MathLib.h"

// A Gamepad struct for trying to encapsulate a "general"-type gamepad (Xbox, Playstation, etc.)
class Gamepad {
public:

	Gamepad();

	float leftTrigger;
	float rightTrigger;

	Vector2f leftStick, rightStick;
	
	bool leftStickUp, leftStickDown, leftStickRight, leftStickLeft;
	float durationLeftStick;
	int leftStickIterations;

	bool aButtonPressed, bButtonPressed, xButtonPressed, yButtonPressed;
	bool leftButtonPressed, rightButtonPressed;

	bool menuButtonPressed;
};


class GamepadManager {
public:
	GamepadManager();
	~GamepadManager();

	void Update(float timeInSeconds);

	// Default time for the first navigation iteration, to prevent double navigation it is higher than the repeat one.
	float secondsThresholdFirstNavigation;
	// Default time between iterations when browsing menus with analog stick.
	float secondsBetweenNavigationIterations;
	// Default 0.98, increase in navigation speed for each previous iteration, i.e. +5% scroll speed per tick if 0.95
	float accelerationFactor;

private:

	void OnNoUpdate(float timeInSeconds, Gamepad & gamepadState, Gamepad & previousState);
	void PostUpdate(Gamepad & gamepadState, Gamepad & previousState);

	// Xbox Controller structures
#ifdef WINDOWS
	Gamepad * gamepadState;
	Gamepad * previousGamepadState;
#endif

};


