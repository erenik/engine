// Emil Hedemalm
// 2013-07-07

#include "Binding.h"
#include "Keys.h"
#include "InputDevices.h"

int Binding::defaultInputDevice = InputDevice::KEYBOARD_1;

Binding::Binding(){
    inputCombinationArray = NULL;
    blockingKeyArray = NULL;
    inputs = blocks = 0;
    action = -1;
    stopAction = -1;
    activateOnRepeat = false;
	inputDevice = defaultInputDevice;
}
Binding::~Binding(){
    if (inputCombinationArray)
        delete[] inputCombinationArray;
    inputCombinationArray = NULL;
    if (blockingKeyArray)
        delete[] blockingKeyArray;
    blockingKeyArray = NULL;
}
/// Sets name for this binding's input-combination
void Binding::SetName(String name){
    this->name = name;
};
void Binding::Print(){
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
}
