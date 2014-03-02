// Emil Hedemalm
// 2013-06-10

#ifndef GAMEPAD_H
#define GAMEPAD_H

// A Gamepad struct for trying to encapsulate a "general"-type gamepad (Xbox, Playstation, etc.)
struct Gamepad {
	
	float leftTrigger;
	float rightTrigger;

	float leftStickX;
	float leftStickY;

	float rightStickX;
	float rightStickY;

};

#endif
