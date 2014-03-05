#ifndef GLOBALSTATE_ACTIONS_H
#define GLOBALSTATE_ACTIONS_H

enum GlobalStateActions{
	NULL_ACTION,

	START_HOSTING,			/// Start hosing a server on port 50333
	START_JOINING,			/// Tries to join server on port 50333
	START_JOINING_MY_WINDOWS_MACHINE,
	STOP_NETWORK,			/// Shutdowns the active network, whether hosting or joining
	// For sending some packets
	SEND_PACKET_1,
	// Makes a test FTP request
	MAKE_FTP_REQUEST,
	/// Opening the general console for multi-purpose commands of more difficult nature
	OPEN_CONSOLE,
	INTERPRET_CONSOLE_COMMAND,
	PREVIOUS_UI_ELEMENT,
	NEXT_UI_ELEMENT,
	ACTIVATE_UI_ELEMENT,
	TOGGLE_MOUSE_LOCK,
};

#endif
