/// Emil Hedemalm
/// 2015-01-16 (although much older originally)
/// A key-binding, for associating a given action with set of keys (a key-combination).
/// By default, the last key in the list must be triggered last for the binding to be triggered.

#ifndef BINDING_H
#define BINDING_H

#include "../Globals.h"
#include "String/AEString.h"

class Action;

/// Structure for a single binding, binding an arbitrary amount of inputs to a single action
struct Binding 
{
	Binding();
	Binding(Action * action, int keys, int to = 0, int trigger = 0, int it = 0);
	Binding(Action * action, List<int> keysToTriggerIt);
	void Nullify();
	~Binding();

	/// Returns self to chain settings.
	Binding * SetActivateOnRepeat(bool repeat);
	/** If true (default), will activate even while mouse/cursor is over an activatable/interactable UI element.
		Set false to make it be ignored when the user is hovering over some UI (to enable default UI-interaction bindings).
	*/
	Binding & SetActivateOverUI(bool ignoreUI);

	/// Used in the Bindings constructor, initially set to KEYBOARD_1.
	static int defaultInputDevice;
	/// Sets name for this binding's input-combination
	void SetName(String name);
	void Print();

	/// Includes all keys required to trigger this binding. Replaces the inputCombinationArray.
	List<int> keysToTriggerIt;

	/// Array of inputs that have to be active in order for the binding to execute
//	int * inputCombinationArray;
	/// Array of keys that will make the command not process in order make more bindings.
//	int * blockingKeyArray;
//	int inputs;			// Length of the input-array.
//	int blocks;			// Number of blocking keys
	String name;		// Name of the binding input-combination
	/// Identifier for the action to perform once the required combination is met. If -1, use stringAction instead!
//	int action;

	/// If true, no other binding may be triggered at the same time, even if it contains similar key-strokes. Default false.
	bool exclusive;
	/** Trigger action. To replace stringAction, stringStopAction, activateOnRepeat, etc. See Action.h for details.
		If non-NULL, will override all said variables used below to hopefully clean-up code used elsewhere.
	*/
	Action * action;
	/*
	enum {
		DEPRECATED_INTEGER_MESSAGE, // Old
		STRING_MESSAGE, // Posting messages straight to the message-manager.
		ACTION, // Executing the triggerAction straight away.
	};
	// See enum of 3 above.
	int actionType;

	/// String action. Uses the same philosophy as the rest of the messaging system. Might obsolete the integral system in due time.
	String stringAction;
	/// String action to take when the key-binding is released. Similar to stopAction but uses the regular messaging system.
	String stringStopAction;
	*/
	/** Identifier for the action to perform once one or more of the
		inputCombination keys have been released. */
	/*
	int stopAction;
	/// Defaults if the bindings activate on repeated inputs sent from the OS after the initial one. (holding keys down, for example)
	bool activateOnRepeat;
	/// If true, is activatable whenever. If false, will not be evaluated when any UI is currently being hovered over.
	bool activateOverUI;
	*/

	/// By default, keyboard 1.
	int inputDevice;
};


#endif
