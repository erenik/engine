/// Emil Hedemalm
/// 2015-01-16
/// An action, or response of any kind, based on input from the user. 
/// Added in order to make the messaging system more flexible when it 
/// comes to triggering Key-bindings where some data other than text 
/// might need transport, or an action wants to be coded into a simple 
/// integer-based value for quicker processing.
///
/// It should also alleviate from re-coding similar tasks for all 
/// projects as the actions include processing code for when they 
/// activate to some extent. E.g. taking a screenshot.

#ifndef ACTION_H
#define ACTION_H

#include "String/AEString.h"

enum {
	NONE,
	QUEUE_STRING_MESSAGE, /// Equivalent to the defualt of just queuing up a String in the MessageManager to be processed by the AppState, etc.
	QUEUE_STRING_START_STOP, /// Similar to the regular message, but posts a different message based on if the action starts or stops (key-binding starts and stops).
							 /// By default all actions are presumed to be executed only on the Starting phase.
	
	/// Application state
	QUIT_APPLICATION,

	// For testing graphics
	LIGHTEN_BACKGROUND,
	DARKEN_BACKGROUND,

	// Reloading stuff.
	RELOAD_UI,
	RECOMPILE_SHADERS,
	RELOAD_TEXTURES,

	// Debugging stuff.
	PRINT_SHADOW_MAPS,
	DEBUG_NEXT, /// Cycles debugging integer for displaying texts or w/e.
	DEBUG_PREVIOUS,

	/// Display options
	TOGGLE_FULL_SCREEN,

	// UIX
	COPY,
	PASTE,	

	// Debug-renders.
	TOGGLE_RENDER_LIGHTS,
	TOGGLE_RENDER_PHYSICS,
	TOGGLE_RENDER_WAYPOINTS,

	CYCLE_RENDER_PIPELINE,
	CYCLE_RENDER_PIPELINE_BACK,

	// Cameras
	CYCLE_ACTIVE_CAMERA,

	// Printing current data.
	RECORD_VIDEO,
	PRINT_SCREENSHOT,

	RELOAD_MODELS, // Reloads all models.
	GO_TO_EDITOR,
	GO_TO_MAIN_MENU,

    PRINT_FRAME_TIME,
	PRINT_PLAYER_INPUT_DEVICES,
	LIST_TEXTURES,
	LIST_MODELS,
	LIST_CAMERAS,
	PRINT_UI_TREE,

	PRINT_TO_FILE, // Prints active video screen or entire game screen-shot as PNG to file.

	OPEN_LIGHTING_EDITOR, // Common for the rendering pipeline in general.
	CLOSE_WINDOW,	// When pressing CTRL+W, like when closing tabs. Closes external windows or queries shutdown.
	INTERPRET_CONSOLE_COMMAND,

};

// Settings.
enum 
{
	ACTIVATE_ON_REPEAT = 1,
};

class Action 
{
public:
	/// Empty.
	Action();
	/// Default of queueing the message into the Message-manager.
	Action(const String & msg);
	/// Sets standard values.
	void Nullify();

	/** Creates an action featuring both a start- and stop-trigger.
		The start- and stop-actions will feature a prefix "Start" and "Stop" respectively, followed by the action's actual name. E.g. StartMoveCameraRight, StopMoveCameraRight
	*/
	static Action * CreateStartStopAction(String action);
	/// See enum at the top of Action.h. Pre-programmed engine functionality, mostly.
	static Action * FromEnum(int id);
	/// Default action based on a single start trigger string.
	static Action * FromString(String str, int flags = 0);

	/// Called when the action is to be triggered.
	virtual void TriggerStart();
	/// Called when the action is to be triggered again. (e.g. OS calls of keys being held down for a pro-longed period of time).
	virtual void TriggerRepeat();
	/// Called when the action is to be triggered.
	virtual void TriggerStop();

	/// Default false. If true, TriggerRepeat will call TriggerStart to perform iterative procedures.
	bool activateOnRepeat;
	/// If cursor is over a UI.
	bool activateOverUI;
protected:
	/// See enum above.
	int type;
	String startAction;
	String stopAction;
};

#endif
