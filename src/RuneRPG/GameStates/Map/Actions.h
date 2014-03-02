// Emil Hedemalm
// 2013-06-17

#ifndef MAP_ACTIONS_H
#define MAP_ACTIONS_H

#include "Input/InputManager.h"

enum MapActions{
	NULL_ACTION,
	GO_TO_MAIN_MENU,

	/// Walking!
	FIRST_PLAYER_ACTION,
	BEGIN_WALK_LEFT, STOP_WALK_LEFT,
	BEGIN_WALK_RIGHT, STOP_WALK_RIGHT,
	BEGIN_WALK_UP, STOP_WALK_UP,
	BEGIN_WALK_DOWN, STOP_WALK_DOWN,
	INTERACT, // Talk, stuff, stuff :D Defaulted to le ENTER-key.
	LAST_PLAYER_ACTION,

	/// Menu navigation, yush!
	NEXT_UI_ELEMENT, // Tab
	PREVIOUS_UI_ELEMENT, // Shift-tab! Favorites :B
	ACTIVATE_UI_ELEMENT, // Enter-key, yaow!

	/// Camera
	CHANGE_CAMERA,			/// Switch active camera

	/// Opening the general console for multi-purpose commands of more difficult nature
	OPEN_CONSOLE,
	INTERPRET_CONSOLE_COMMAND,

	// Debugging
	PRINT_FRAME_TIME,

	/// Map save/loading
	SAVE_MAP_PROMPT,		/// Begins a prompt to select file-name to save the map as
	SAVE_MAP,				/// Save current contents as a map
	LOAD_MAP_PROMPT,		/// Begins a prompt to select map-file to load
	LOAD_MAP,				/// Clears all active entities and loads target map
	LOAD_MODEL,
	/// Listing functions
	LIST_MODELS,
	LIST_TEXTURES,
	LIST_ENTITIES,			/// Lists entities relevant to the active (editor) map
	LIST_SELECTION,			/// Lists currently selected entities
	LIST_ACTIONS,			/// Lists all available editor actions ^^
	LIST_DATA,				/// Lists entity data for all selected entities.
	/// Selecting functions
	SELECT_ALL,				/// Selects all entities
	SELECT_NEXT,			/// Selects next visible entity
	SELECT_PREVIOUS,		/// Selects previous visible entity
	SELECT_ENTITY_PROMPT,	/// Begins prompt to select target entity/entities
	SELECT_ENTITY,			/// Attempts to select specified entities
	ADD_TO_SELECTION_PROMPT,/// Begins prompt to select more entities without deselecting the previous ones.
	ADD_TO_SELECTION,
	REMOVE_FROM_SELECTION_PROMPT,	///
	REMOVE_FROM_SELECTION,	/// Attempts to remove from selection by parsing input
	CLEAR_SELECTION, DESELECT = CLEAR_SELECTION,
	/// Entity creation
	CREATE_ENTITY_PROMPT,	/// Begins a prompt to create an entity using object and maybe texture
	CREATE_ENTITY,			/// Attempts to create the entity using the specified parameters in the input-string
	/// Deletion
	DELETE_ENTITY_PROMPT,	/// Opens prompt to delete selected entity/entities
	DELETE_ENTITY,
	/// Entity manipulation
	TRANSLATE_ENTITY_PROMPT,/// Begins prompt for entity translation
	TRANSLATE_ENTITY,		/// Translates active entity/entities
	RESET_ENTITY_SCALE,		/// Resets scale to 1.0 on all lengths
	SCALE_ENTITY_PROMPT,	/// Begins prompt for entity scaling
	SCALE_ENTITY,			/// Scales active entity/entities
	ROTATE_ENTITY_PROMPT,	/// Begins prompt for entity rotation
	ROTATE_ENTITY,			/// Rotates active entity/entities
	SET_TEXTURE_PROMPT,		/// Begins a prompt to set texture of the active entity
	SET_TEXTURE,			/// Sets texture to target entity by parsing input-buffer
	SET_MODEL_PROMPT,		/// Begins a prompt to set model of the active entity
	SET_MODEL,				/// Sets model to target entity by parsing input-buffer
	SET_ENTITY_NAME_PROMPT,	/// Begins a prompt to set active entity's name.
	SET_ENTITY_NAME,		/// Sets name to target entity by parsing input-buffer
	/// Physics
	PAUSE_SIMULATIONS,
	TOGGLE_PHYSICS_SHAPES,	/// Toggles rendering of the physics shapes!

	/// Camera
	STOP,	/// Stops all movement :D
	FORWARD,	FORWARD_S,	/// Navigation
	BACKWARD,	BACKWARD_S,
	LEFT,		LEFT_S,
	RIGHT,		RIGHT_S,
	UP,			UP_S,
	DOWN,		DOWN_S,
	TURN_LEFT,	TURN_LEFT_S,	/// Rotation
	TURN_RIGHT,	TURN_RIGHT_S,
	TURN_UP,	TURN_UP_S,
	TURN_DOWN,	TURN_DOWN_S,
	COME_CLOSER,	/// Camera Distance
	BACK_AWAY,
	ZOOM_IN,		/// Zoom
	ZOOM_OUT,
	INCREASE_SPEED,
	DECREASE_SPEED,
	RESET_CAMERA,
};

#endif
