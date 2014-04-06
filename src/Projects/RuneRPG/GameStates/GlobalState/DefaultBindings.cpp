// Awesome Author

#include "RuneGlobalState.h"
#include "Actions.h"
#include "Input/InputManager.h"
// Don't include all managers. Ever.

/// Creates bindings that are used for debugging purposes only
void RuneGlobalState::CreateDefaultBindings(){
	std::cout<<"\nGlobalState::CreateDefaultBindings() called";

	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	/// Get pointer to this mapping
	InputMapping * mapping = &this->inputMapping;
	/// Create default bindings

	/// C = Create, L = List

	mapping->CreateBinding(TOGGLE_FULL_SCREEN, KEY::ALT, KEY::ENTER);
	mapping->CreateBinding(RELOAD_BATTLERS, KEY::CTRL, KEY::R, KEY::B, "Reload battles");
	mapping->SetBlockingKeys(mapping->CreateBinding(OPEN_CONSOLE, KEY::CTRL, KEY::ENTER, "CTRL + ENTER : Open Console"), KEY::ALT);

	mapping->CreateBinding(TOGGLE_MOUSE_LOCK, KEY::CTRL, KEY::L, KEY::M, "Toggle mouse lock");

};

