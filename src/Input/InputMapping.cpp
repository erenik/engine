/// Emil Hedemalm
/// 2015-01-14 (although much older originally)

#include "InputMapping.h"
#include "../Globals.h"
#include <iostream>

/// Default keyMapping action processor
void defaultInputProcessor(int action){
	std::cout<<"\nWARNING: Default processor called to process input. ";
}

/// Sets the processor function pointer to a default function.
InputMapping::InputMapping(){
	activeBinding = NULL;
	this->mappings = 0;
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
	//	if (binding->action == 9 && activeKeyCode == KEY::E)
	//		int b = i;
        // Skip null-actions
		if (binding->action == 0)
			continue;
		// Skip if our active key is not the relevant one
		int inputRelevantKey = binding->inputCombinationArray[binding->inputs-1];
		if (binding->inputCombinationArray[binding->inputs-1] != activeKeyCode)
			continue;
//		this->binding->Print();
		// If it is our active key, check that all other pre-requisites are met
		bool allPrerequisitesMet = true;
		for (int j = 0; j < binding->inputs; ++j){
            int cKey = binding->inputCombinationArray[j];
      //      std::cout<<"\nKeyState for "<<GetKeyString(cKey)<<": "<<keyPressedState[cKey];
			if (!keyPressedState[binding->inputCombinationArray[j]]){
				allPrerequisitesMet = false;
		//		std::cout<<", "<<j+1<<" of "<<b->inputs<<" input-prerequisties met.";
				break;
			}
		}
		if (!allPrerequisitesMet)
			continue;
	//	std::cout<<"\nEvaluating blocks..";
		for (int j = 0; j < binding->blocks; ++j){
			if (keyPressedState[binding->blockingKeyArray[j]]){
				allPrerequisitesMet = false;
				break;
			}
		}
		if (!allPrerequisitesMet)
			continue;
		// Check that repeatability not is an issue.
		if (binding->activateOnRepeat == false && downBefore == true)
			continue;
//#define LOG_BINDINGS
#ifdef LOG_BINDINGS
		if (binding->name)
			std::cout<<"\nExecuting binding: "<<(b->name ? b->name : "Unnamed");
#endif
		/// Process our action!
		return binding;
		++triggeredBindings;
	}
	return 0;
}

Binding * InputMapping::EvaluateKeyRelease(int activeKeyCode, bool *keyPressedState){
	int triggeredBindings = 0;
	/// Assume the active key is the last one in the chain
	for (int i = 0; i < bindings.Size(); ++i){
		Binding * binding = bindings[i];
		// Skip null-actions
		if (binding->stopAction == 0)
			continue;
		// Check if the key belonged to any of the previously active states,
		// make sure that this was part of that combination to!
		int diff = 0;
		bool partOfThis = false;
		/// Required inputs - active ones.
		for (int j = 0; j < binding->inputs; ++j){
			if (!keyPressedState[binding->inputCombinationArray[j]]){
				++diff;
			}
			if (binding->inputCombinationArray[j] == activeKeyCode)
				partOfThis = true;
		}
		/// Check the key-blocks too..!
		for (int j = 0; j < binding->blocks; ++j){
			if (keyPressedState[binding->blockingKeyArray[j]]){
				partOfThis = false;
				break;
			}
		}
		/// If the not part of this or diff is not 1, continue.
		if (!partOfThis || diff != 1)
			continue;
		if (binding->name)
			;//std::cout<<"\nExecuting stopAction for binding: "<<binding->name;
		/// Process our action!
		/// Process our action!
		return binding;
		++triggeredBindings;
	}
	return NULL;
}


/// Creates a new binding, returning a pointer to it
Binding * InputMapping::CreateBinding(int action, int key1, const char *name){
	return CreateBinding(action, key1, 0, 0, name);
}
/// Creates a new binding, returning a pointer to it
Binding * InputMapping::CreateBinding(int action, int key1, int key2, const char *name){
	return CreateBinding(action, key1, key2, 0, name);
}

/// Creates a new binding, returning a pointer to it
Binding * InputMapping::CreateBinding(int action, int key1, int key2, int key3, const char *name){
	Binding * binding = new Binding();
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
Binding * InputMapping::CreateBinding(String action, int key1, int key2 /*= 0*/, int key3 /*= 0*/){
	Binding * binding = new Binding();
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
Binding * InputMapping::SetBlockingKeys(Binding * binding, int key1, int key2, int key3){
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


/// Sets the repeatability flag for the active binding (e.g. bolding down keys)
Binding * InputMapping::Repeatable(bool flag){
	if (activeBinding == NULL){
		std::cout<<"\nERROR: No active binding selected.";
		return NULL;
	}
	activeBinding->activateOnRepeat = flag;
	return activeBinding;
}

/// Sets the repeatability flag for the binding (e.g. holding down keys)
Binding * InputMapping::Repeatable(Binding * binding, bool flag){
	if (binding == NULL){
		std::cout<<"\nERROR: Binding NULL.";
		return NULL;
	}
	binding->activateOnRepeat = flag;
	activeBinding = binding;
	return binding;
}
