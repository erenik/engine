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

	String name;		// Name of the binding input-combination
	
	/// If true, will only trigger when no entity is focused in camera.
	static bool requireNoCameraFocusEntity;
	/// If true, no other binding may be triggered at the same time, even if it contains similar key-strokes. Default false, enabling multiple messages to be generated from one key-stroke.
	bool exclusive;
	/** Trigger action. To replace stringAction, stringStopAction, activateOnRepeat, etc. See Action.h for details.
		If non-NULL, will override all said variables used below to hopefully clean-up code used elsewhere.
	*/
	Action * action;

	/// By default, keyboard 1.
	int inputDevice;
};


#endif
