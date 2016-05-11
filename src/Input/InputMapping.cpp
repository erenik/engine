/// Emil Hedemalm
/// 2015-01-14 (although much older originally)

#include "InputMapping.h"
#include "../Globals.h"
#include <iostream>
#include "Input/Action.h"

/// Default keyMapping action processor
void defaultInputProcessor(int action){
	std::cout<<"\nWARNING: Default processor called to process input. ";
}

/// Sets the processor function pointer to a default function.
InputMapping::InputMapping()
{
	activeBinding = NULL;
}

InputMapping::~InputMapping()
{
	this->bindings.ClearAndDelete();
}

#include "InputManager.h"

/** Evaluates by looking at the bindings if an action should be taken or not.
	Returns:	 the action to process if applicable.
	Parameters:
		- activeKeyCode		The engine defined key code, e.g: KEY::W, KEY::CONTROL, KEY::SHIFT
		- keyPressedStates	A boolean array with pressed states of all known and bound keys.
		- downBefore		Specifies if the key was down before, only relevant on key presses.
*/
Binding * InputMapping::EvaluateInput(int activeKeyCode, bool * keyPressedState, bool downBefore)
{
	int triggeredBindings = 0;
	/// Assume the active key is the last one in the chain
	for (int i = 0; i < bindings.Size(); ++i)
	{
		/// DEBUG
		Binding * binding = bindings[i];
		// Skip if our active key is not the relevant one
		if (binding->keysToTriggerIt.Last() != activeKeyCode)
			continue;
		// If it is our active key, check that all other pre-requisites are met
		bool allPrerequisitesMet = true;
		for (int j = 0; j < binding->keysToTriggerIt.Size(); ++j)
		{
			int key = binding->keysToTriggerIt[j];
			if (keyPressedState[key] == false)
			{
				allPrerequisitesMet = false;
				break;
			}
		}
		if (!allPrerequisitesMet)
			continue;
		/// Check if ALT, CTRL or SHIFT are held, without them being included in the keys to trigger. If so, do not trigger it.
		List<int> activeModifierKeys = InputMan.ActiveModifierKeys();
		if (activeModifierKeys.Size())
		{
			/// If the active modifier keys do not exist in the binding, skip triggering it.
			if (!binding->keysToTriggerIt.Exists(activeModifierKeys))
				continue;
		}

		// Check that repeatability not is an issue.	
		if (downBefore)
			binding->action->TriggerRepeat();
		else 
			binding->action->TriggerStart();
//#define LOG_BINDINGS
#ifdef LOG_BINDINGS
		if (binding->name)
			std::cout<<"\nExecuting binding: "<<(b->name ? b->name : "Unnamed");
#endif
		/// Return the binding with was triggered? Only one should be triggered at a time, right?
		if (binding->exclusive)
			return binding;
		++triggeredBindings;
	}
	return 0;
}

Binding * InputMapping::EvaluateKeyRelease(int activeKeyCode, bool *keyPressedState)
{
	int triggeredBindings = 0;
	/// Assume the active key is the last one in the chain
	for (int i = 0; i < bindings.Size(); ++i)
	{
		Binding * binding = bindings[i];
		if (!binding->keysToTriggerIt.Exists(activeKeyCode))
			continue;
		/// Check if ALT, CTRL or SHIFT are held, without them being included in the keys to trigger. If so, do not trigger it.
		List<int> activeModifierKeys = InputMan.ActiveModifierKeys();
		if (activeModifierKeys.Size())
		{
			/// If the active modifier keys do not exist in the binding, skip triggering it.
			if (!binding->keysToTriggerIt.Exists(activeModifierKeys))
				continue;
		}
		/// If it is our active key, check that all other pre-requisites are met
		int keyDiff = binding->keysToTriggerIt.Size();
		for (int j = 0; j < binding->keysToTriggerIt.Size(); ++j)
		{
			int key = binding->keysToTriggerIt[j];
			if (keyPressedState[key] == true)
			{
				--keyDiff;
			}
		}
		/// If we're more than 1 key away, skip it?
		if (keyDiff != 1)
			continue;
		/// If just one key away, it means it was just triggered and should now end?
		binding->action->TriggerStop();
		/// Return the binding with was triggered? Only one should be triggered at a time, right?
		if (binding->exclusive)
			return binding;
		++triggeredBindings;
	}
	return NULL;
}


/*
/// Creates a new binding, returning a pointer to it
Binding * InputMapping::CreateBinding(int action, int key1, const char *name){
	assert(false && "Deprecated");
	return CreateBinding(action, key1, 0, 0, name);
}
/// Creates a new binding, returning a pointer to it
Binding * InputMapping::CreateBinding(int action, int key1, int key2, const char *name){
	assert(false && "Deprecated");
	return CreateBinding(action, key1, key2, 0, name);
}

/// Creates a new binding, returning a pointer to it
Binding * InputMapping::CreateBinding(int action, int key1, int key2, int key3, const char *name)
{
	assert(false && "Deprecated");
	Binding * binding = new Binding();
	/*
	binding->action = action;
	int inputs = 0;
	if (key3)
		inputs = 3;
	else if (key2)
		inputs = 2;
	else
		inputs = 1;

	binding->inputs = inputs;
	binding->inputCombinationArray = new int[inputs];
	binding->inputCombinationArray[0] = key1;
	if (key2)
		binding->inputCombinationArray[1] = key2;
	if (key3)
		binding->inputCombinationArray[2] = key3;
	/// Set name if provided
	if (name)
		binding->SetName(name);
	activeBinding = binding;
	++this->mappings;
	bindings.Add(binding);
	return binding;
}

/// Creates a new binding, returning a pointer to it and making it active in the InputMapping editor.
Binding * InputMapping::CreateBinding(String action, int key1, int key2, int key3)
{
	Binding * binding = new Binding();
	/*
	/// Set action to -1 to show that we want to use the string-type.
	binding->stringAction = action;
	binding->action = -1;
	int inputs = 0;
	if (key3)
		inputs = 3;
	else if (key2)
		inputs = 2;
	else
		inputs = 1;

	binding->inputs = inputs;
	binding->inputCombinationArray = new int[inputs];
	binding->inputCombinationArray[0] = key1;
	if (key2)
		binding->inputCombinationArray[1] = key2;
	if (key3)
		binding->inputCombinationArray[2] = key3;
	/// Set name if provided
	binding->SetName(action);
	activeBinding = binding;
	++this->mappings;
	bindings.Add(binding);
	return binding;
}
	

/// Sets blocking keys for the active binding.
Binding * InputMapping::SetBlockingKeys(int key1, int key2, int key3){
	return SetBlockingKeys(activeBinding, key1, key2, key3);
}
/// Sets blocking keys for the binding
Binding * InputMapping::SetBlockingKeys(Binding * binding, int key1, int key2, int key3)
{
	assert(false && "Deprecated");
	if (binding == NULL){
		std::cout<<"\nERROR: Binding NULL.";
		return NULL;
	}
	int blocks = 0;
	if (key3)
		blocks = 3;
	else if (key2)
		blocks = 2;
	else
		blocks = 1;

	binding->blocks = blocks;
	binding->blockingKeyArray = new int[blocks];
	binding->blockingKeyArray[0] = key1;
	if (key2)
		binding->blockingKeyArray[1] = key2;
	if (key3)
		binding->blockingKeyArray[2] = key3;
	activeBinding = binding;
	return binding;
}
*/

/// Sets the repeatability flag for the active binding (e.g. bolding down keys)
Binding * InputMapping::Repeatable(bool flag)
{
	activeBinding->SetActivateOnRepeat(flag);
	return activeBinding;
}

/// Sets the repeatability flag for the binding (e.g. holding down keys)
Binding * InputMapping::Repeatable(Binding * binding, bool flag)
{
	binding->SetActivateOnRepeat(flag);
	return binding;
}
