// Emil Hedemalm
// 2013-07-07

#include "Binding.h"
#include "Keys.h"
#include "InputDevices.h"
#include "Action.h"

int Binding::defaultInputDevice = InputDevice::KEYBOARD_1;

Binding::Binding()
{
	/*
    inputCombinationArray = NULL;
    blockingKeyArray = NULL;
    inputs = blocks = 0;
    action = -1;
    stopAction = -1;
    activateOnRepeat = false;
	*/
	Nullify();
//	activateOverUI = true;
}

Binding::Binding(Action * action, int keys, int to, int trigger, int it)
: action(action)
{
	Nullify();
	keysToTriggerIt.Add(keys);
	if (to)
		keysToTriggerIt.Add(to);
	if (trigger)
		keysToTriggerIt.Add(trigger);
	if (it)
		keysToTriggerIt.Add(it);
}

/// Constructor which assigns message to be spend upon triggering it.
Binding::Binding(Action * action, List<int> keysToTriggerIt)
: action(action), keysToTriggerIt(keysToTriggerIt)
{
	Nullify();
}
void Binding::Nullify()
{
	inputDevice = defaultInputDevice;
	exclusive = false;
}

Binding::~Binding()
{
	SAFE_DELETE(action);
}

/// Returns self to chain settings.
Binding * Binding::SetActivateOnRepeat(bool repeat)
{
	action->activateOnRepeat = repeat;
	return this;
}

/** If true (default), will activate even while mouse/cursor is over an activatable/interactable UI element.
	Set false to make it be ignored when the user is hovering over some UI (to enable default UI-interaction bindings).
*/
Binding & Binding::SetActivateOverUI(bool ignoreUI)
{
	assert(action);
	action->activateOverUI = ignoreUI;
	return *this;
}


/// Sets name for this binding's input-combination
void Binding::SetName(String name){
    this->name = name;
};
void Binding::Print(){
		assert(false && "Deprecated");

	/*
    if (!action)
        return;
    std::cout<<"\n"<<(name ? name : "Unnamed ")<<", ";
    for (int i = 0; i < inputs; ++i){
        if (i > 0 && i < inputs)
            std::cout<<" + ";
        std::cout<<GetKeyString(inputCombinationArray[i]);
    }
    std::cout<<", Action: "<<action;
    if (stopAction)
        std::cout<<" StopAction: "<<stopAction;
		*/
}
