
#ifndef MAINMENU_ACTIONS_H
#define MAINMENU_ACTIONS_H

enum MenuActions{
	NULL_ACTION,
	GO_TO_EDITOR_STATE,
	GO_TO_MAIN_MENU,
	GO_TO_AI_TEST,
	GO_TO_RACING_STATE,
	GO_TO_NETWORK_TEST,
	GO_TO_BLUEPRINT_EDITOR,

	INCREASE_PLAYERS,
	DECREASE_PLAYERS,

	/// Menu navigation, yush!
	NEXT_UI_ELEMENT, // Tab
	PREVIOUS_UI_ELEMENT, // Shift-tab! Favorites :B
	ACTIVATE_UI_ELEMENT, // Enter-key, yaow!
};

#endif