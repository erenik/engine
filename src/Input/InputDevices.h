// Emil Hedemalm
// 2013-07-11

// Input device enumeration, for binding keys and buttons to specific actions.

#ifndef INPUT_DEVICES_H
#define INPUT_DEVICES_H

#include "String/AEString.h"

/// List of input-devices o-o, used for handling multiple inputs (multiple local players)
/// These are added to the message number of player-actions in-game to differentiate which controller gave which response.
namespace InputDevice{
enum inputDevices {
	INVALID_DEVICE = -1,
    /// -1 is bad device.
	KEYBOARD_1 = 0,	// WSAD
	KEYBOARD_2,	// Arrows
	GAME_PAD_1,	// Xbox/PSX/etc.-controllers
	GAME_PAD_2,
	GAME_PAD_3,
	GAME_PAD_4,
	MAX_INPUT_DEVICES // Maximhum o-o
};};

/// Re-define this if compiling for another platform! :)
const int FIRST_VALID_INPUT_DEVICE = InputDevice::KEYBOARD_1;

String GetInputDeviceName(int deviceEnum);

#endif
