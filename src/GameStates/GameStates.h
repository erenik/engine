#ifndef GAME_STATES_H
#define GAME_STATES_H

// Game states
enum game_states {
	/// Default states
	GAME_STATE_NULL, // For setting none
	GAME_STATE_NONE = GAME_STATE_NULL,
	GAME_STATE_INITIALIZATION,
	GAME_STATE_LOADING,
	GAME_STATE_GLOBAL,
	GAME_STATE_MAIN_MENU,
	GAME_STATE_NEW_GAME,  // Why is this a fucking state? Remove.
	GAME_STATE_LOAD_GAME,	// Why is this a fucking state? Remove?
	GAME_STATE_EDITOR,
	GAME_STATE_CREDITS,
	GAME_STATE_OPTIONS,
	GAME_STATE_QUIT, GAME_STATE_EXITING = GAME_STATE_QUIT, GAME_STATE_EXIT = GAME_STATE_QUIT,

	/// States added with SpaceRace
	GAME_STATE_BLUEPRINT_EDITOR,
	GAME_STATE_RACING,		/// Test racing game ^^
	GAME_STATE_LOBBY,

    /// For the DemoProject
    GAME_STATE_PHYSICS_TEST,
	GAME_STATE_STREAM_TEST,

	GAME_STATE_MODEL_EDITOR,

	// For the MusicPlayer project
	GAME_STATE_MUSIC_PLAYER,

	// Test states
	GAME_STATE_AI_TEST,		/// Includes example AI
	GAME_STATE_PATHFINDING,	/// Includes test of pathfinding algorithms
	GAME_STATE_NETWORK_TEST,/// Awesome Network

	/// Add proper thingy for game-specific states.
	GAME_STATE_GAME_SPECIFIC_STATES_FIRST,
	/// Assume max 100 states, but increase it as needed,
	GAME_STATE_GAME_SPECIFIC_STATES_LAST = GAME_STATE_GAME_SPECIFIC_STATES_FIRST + 100,

	GAME_STATES,
	MAX_GAME_STATES,
};

#endif
