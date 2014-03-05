// Emil Hedemalm
// 2013-06-28

#include "Game/GameType.h"

#ifdef RUNE_RPG

#include "MainMenu.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Message/Message.h"
#include "StateManager.h"
#include "Input/InputManager.h"

void MainMenu::InputProcessor(int action, int inputDevice/* = 0*/){
	switch(action){
		case GO_TO_EDITOR_STATE:
			std::cout<<"\nInput>>GO_TO_EDITOR_STATE";
			StateMan.QueueState(GAME_STATE_EDITOR);
			break;
		case GO_TO_AI_TEST:
			std::cout<<"\nInput>>GO_TO_AI_TEST";
			StateMan.QueueState(GAME_STATE_AI_TEST);
			break;
		case GO_TO_RACING_STATE:
			std::cout<<"\nInput>>GO_TO_RACING_STATE";
			StateMan.GetState(GAME_STATE_RACING)->ProcessMessage(new Message("set players " + String::ToString(requestedPlayers)));
			StateMan.QueueState(GAME_STATE_RACING);
			break;
		case GO_TO_NETWORK_TEST:
			std::cout<<"\nInput>>GO_TO_NETWORK_TEST";
			StateMan.QueueState(GAME_STATE_NETWORK_TEST);
			break;
		case GO_TO_BLUEPRINT_EDITOR:
			std::cout<<"\nInput>>GO_TO_BLJUEPRINT_EDITOR";
			StateMan.QueueState(GAME_STATE_BLUEPRINT_EDITOR);
			break;
	}
}

#endif
