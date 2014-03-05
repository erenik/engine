// Emil Hedemalm
// 2013-06-28

#include "MainMenu.h"
#include "Actions.h"
#include "Input/InputManager.h"
// Don't include all managers. Ever.

/// Creates bindings that are used for debugging purposes only
void MainMenu::CreateDefaultBindings(){
	std::cout<<"\nMainMenu::CreateDefaultBindings() called";
	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	InputMapping * mapping = &inputMapping;
	mapping->CreateBinding(GO_TO_AI_TEST, KEY::CTRL, KEY::G, KEY::A);
	mapping->CreateBinding(GO_TO_RACING_STATE, KEY::CTRL, KEY::G, KEY::R);
	mapping->CreateBinding(GO_TO_NETWORK_TEST, KEY::CTRL, KEY::G, KEY::N);
	mapping->CreateBinding(GO_TO_BLUEPRINT_EDITOR, KEY::CTRL, KEY::G, KEY::S);

	/// Menu navigation, yush!
	mapping->CreateBinding(PREVIOUS_UI_ELEMENT, KEY::SHIFT, KEY::TAB);
	mapping->CreateBinding(NEXT_UI_ELEMENT, KEY::TAB);
	mapping->CreateBinding(ACTIVATE_UI_ELEMENT, KEY::ENTER);

	mapping->CreateBinding(TEST_AUDIO, KEY::T);
};

/// Creates the user interface for this state
void MainMenu::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MainMenu.gui");
}
