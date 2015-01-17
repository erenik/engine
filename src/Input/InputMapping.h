/// Emil Hedemalm
/// 2015-01-14 (although much older originally)

#ifndef INPUT_MAPPING_H
#define INPUT_MAPPING_H

//#include <cstdio>
#include "Binding.h"
#include <List/List.h>

/// Class for mapping one or several inputs to a specific action
class InputMapping {
	friend class InputManager;
public:
	/// Sets the processor function pointer to a default function.
	InputMapping();
	virtual ~InputMapping();

	/// Sets the repeatability flag for the active binding (e.g. bolding down keys)
	Binding * Repeatable(bool flag = true);
	/// Sets the repeatability flag for the target binding (e.g. holding down keys)
	Binding * Repeatable(Binding * binding, bool flag = true);
	/// Releases all bindings with the specified action tied to it.
	void ReleaseBindings(int actionToBeReleased);
	/** Evaluates by looking at the bindings if an action should be taken or not.
		Returns the binding if it should be processed, and NULL if not.
		Parameters:
			- activeKeyCode		The engine defined key code, e.g: KEY::W, KEY::CONTROL, KEY::SHIFT
			- keyPressedStates	A boolean array with pressed states of all known and bound keys.
			- downBefore		Specifies if the key was down before, only relevant on key presses.
	*/
	Binding * EvaluateInput(int activeKeyCode, bool * keyPressedStates, bool downBefore);
	/** Evaluates key releases by looking at the bindings if a stop-action should be taken or not.
		Returns true if the mapping found and processed a stop-action mapped to the relevant active key.
		Returns the binding if it should be processed, and NULL if not.
	*/
	Binding * EvaluateKeyRelease(int activeKeyCode, bool * keyPressedStates);

	/// Pointer to last created/selected/edited binding for eased usage :)
	Binding * activeBinding;
	/// Array of bindings
	List<Binding*> bindings;
};

#endif
