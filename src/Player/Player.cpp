// Emil Hedemalm
// 2013-07-10
// A meta-structure for keeping track of a player and all it's external relations.

#include "Player.h"
#include "Input/InputDevices.h"
#include "Network/Peer.h"

int Player::idEnumerator = 0;

Player::Player(String name)
: name(name), type("DefaultPlayerType")
{
	id = idEnumerator++;
	entity = NULL;
	isLocal = true;
	isAI = false;
	color = Vector4f(1,1,1,1);
	/// Default no input device, must be assigned explicitly!
	inputDevice = InputDevice::KEYBOARD_1;
	viewport = NULL;
	owner = NULL;
}

Player::~Player()
{
}