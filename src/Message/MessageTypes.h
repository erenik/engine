// Emil Hedemalm
// 2013-06-15

// Just an enum..

#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

namespace MessageType {
enum messageTypes{
	NULL_TYPE,
	STRING,
	WINDOW_MESSAGE, // OS-based window messages, sent either from WndProc or XProc, to update UI, etc.
	DRAG_AND_DROP,
	MOUSE_MESSAGE,
	PASTE, // Copy-paste.
	CONSOLE_COMMAND, // Supposed to be entered from any input or maybe console for more complex usage.
	GAMEPAD_MESSAGE, // See Input/Gamepad/GamepadMessage.h
	/// Messages sent from the UI system
	ON_UI_ELEMENT_HOVER,
	/// Message type that is based on a string for what action to perform, but includes a list of 1 or more files or directories to act upon too. See Message/FileEvent.h
	FILE_EVENT,
	TEXTURE_MESSAGE,
	SET_STRING,			// For setting strings, usually via dedicated ui.
	BOOL_MESSAGE,		// For setting booleans. Sent from UICheckBox.
	FLOAT_MESSAGE,		// For setting floats, simple as that.
	INTEGER_MESSAGE,	// For setting Integers.
	VECTOR_MESSAGE,		// For setting vectors, simple as that.
	DATA_MESSAGE, // For DataMessage
	DATA, // Probably obsolete.
	RAYCAST, // Defined in Physics/Messages/PhysicsMessage.h sent as a reply to PMRaycast. By default handled by respective entity's properties.
	COLLISSION_CALLBACK, // Sent by the physics system if certain stuff are met.
	EDITOR_MESSAGE,
	CURL, // HTTP and similar stuff, sent by the CURLManager
	SIP, // Session Initiation Protocol, sent from the NetworkManager when certain SIP packets are received.
	SET_PATH_DESTINATION_MESSAGE, // PathableProperty.
	PATHFINDING_MESSAGE, // For Pathfinding/Navmesh system. Sent to PathableProperty, PathManager and entities.
	MOVE_TO_MESSAGE, /// Sent by PathableProperty. Interpret in other properties and apply accelerations accordingly!

	// Consider creating packets straight away for these networking features instead of using the message system?
	/// Make room for 100 different network packets (no more than like 10 should be needed, but eh..)
	FIRST_NETWORK_MESSAGE = 100,
	LAST_NETWORK_MESSAGE = FIRST_NETWORK_MESSAGE + 100,

	// Game-specific message numbers, enumerate as pleased. Advised to store name in the message as well.
	CUSTOM,
	BREAKOUT_POWERUP, // For breakout.
	LAST_CUSTOM_MESSAGE_TYPE = CUSTOM + 100,
};
};

#endif
