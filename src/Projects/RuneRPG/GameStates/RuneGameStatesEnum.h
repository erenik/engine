/// Emil Hedemalm
/// 2013-12-13
/// Just to keep track of it all myself...

#ifndef RUNE_GAME_STATE_ENUM
#define RUNE_GAME_STATE_ENUM 

#include "GameStates/GameStates.h"

enum RuneGameStatesEnum {
	RUNE_GAME_STATE_MAIN_MENU = GAME_STATE_MAIN_MENU,
	RUNE_GAME_STATE_EDITOR	= GAME_STATE_EDITOR,
	/// All specific states below
	RUNE_GAME_STATE_MAP = GAME_STATE_GAME_SPECIFIC_STATES_FIRST,
	RUNE_GAME_STATE_BATTLE_STATE,
	RUNE_GAME_STATE_RUNE_STATE,

};

#endif