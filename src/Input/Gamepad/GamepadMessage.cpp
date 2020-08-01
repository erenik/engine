// Emil Hedemalm
// 2020-08-01
// Message sent when gamepad state is updated.

#include "GamepadMessage.h"

GamepadMessage::GamepadMessage() : Message(MessageType::GAMEPAD_MESSAGE) 
, index(-1)
, leftStickUpdated(false)
, rightStickUpdated(false)
, aButtonPressed(false)
, bButtonPressed(false)
, xButtonPressed(false)
, yButtonPressed(false)
, leftTriggerUpdated(false)
, rightTriggerUpdated(false)
{

}
