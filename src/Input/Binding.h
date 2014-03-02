#ifndef BINDING_H
#define BINDING_H

#include "../Globals.h"
#include "String/AEString.h"

/// Structure for a single binding, binding an arbitrary amount of inputs to a single action
struct Binding {
	Binding();
	~Binding();

	/// Used in the Bindings constructor, initially set to KEYBOARD_1.
	static int defaultInputDevice;
	/// Sets name for this binding's input-combination
	void SetName(String name);
	void Print();

	/// Array of inputs that have to be active in order for the binding to execute
	int * inputCombinationArray;
	/// Array of keys that will make the command not process in order make more bindings.
	int * blockingKeyArray;
	int inputs;			// Length of the input-array.
	int blocks;			// Number of blocking keys
	String name;		// Name of the binding input-combination
	/// Identifier for the action to perform once the required combination is met. If -1, use stringAction instead!
	int action;
	/// String action. Uses the same philosophy as the rest of the messaging system. Might obsolete the integral system in due time.
	String stringAction;
	/// String action to take when the key-binding is released. Similar to stopAction but uses the regular messaging system.
	String stringStopAction;
	/** Identifier for the action to perform once one or more of the
		inputCombination keys have been released. */
	int stopAction;
	/// Defaults if the bindings activate on repeated inputs sent from the OS after the initial one. (holding keys down, for example)
	bool activateOnRepeat;
	/// By default, keyboard 1.
	int inputDevice;
};


#endif
