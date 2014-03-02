// Emil Hedemalm
// 2013-08-08

#include "InputDevices.h"


String GetInputDeviceName(int deviceEnum){
    switch(deviceEnum){
		case InputDevice::INVALID_DEVICE:
			return "Invalid device id";
		case InputDevice::KEYBOARD_1:
            return "Keyboard 1";
        case InputDevice::KEYBOARD_2:
            return "Keyboard 2";
        case InputDevice::GAME_PAD_1:
            return "Gamepad 1";
        case InputDevice::GAME_PAD_2:
            return "Gamepad 2";
        case InputDevice::GAME_PAD_3:
            return "Gamepad 3";
        case InputDevice::GAME_PAD_4:
            return "Gamepad 4";
        default:
            return "Undefined index "+String::ToString(deviceEnum);
    }
}
