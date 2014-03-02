#ifndef GLOBALSTATE_ACTIONS_H
#define GLOBALSTATE_ACTIONS_H

enum GlobalStateActions{
	NULL_ACTION,

	/// Woo
	TOGGLE_FULL_SCREEN,
	RELOAD_BATTLERS,
	/// Opening the general console for multi-purpose commands of more difficult nature
	OPEN_CONSOLE,
	INTERPRET_CONSOLE_COMMAND,

	// To disable mouse input
	TOGGLE_MOUSE_LOCK,

};

#endif
