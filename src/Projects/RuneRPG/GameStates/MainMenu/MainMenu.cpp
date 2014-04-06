// Emil Hedemalm
// 2013-06-28

#include "OS/Sleep.h"
#include "MainMenu.h"
#include "GameStates/GeneralManagerIncludes.h"

#include "Message/Message.h"
#include "Actions.h"
#include "Graphics/Messages/GMUI.h"
#include "Maps/Grids/TileTypeManager.h"
#include "Maps/Grids/Tile.h"
#include "../Map/MapState.h"
#include "Graphics/Messages/GMSet.h"
extern UserInterface * ui[MAX_GAME_STATES];
#include "../RuneGameStatesEnum.h"


MainMenu::MainMenu(){
	id = GAME_STATE_MAIN_MENU;
	requestedPlayers = 1;

	// Enter some tile types into the manager
	TileTypeManager::Allocate();
	TileTypes.CreateDefaultTiles();
}

MainMenu::~MainMenu(){
	TileTypeManager::Deallocate();
}

void MainMenu::OnEnter(GameState * previousState){
	std::cout<<"\nEntering Main Menu state";

	Graphics.EnableAllDebugRenders(false);

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	// Pre-load the loadingData texture for future use!
	TexMan.LoadTexture("loadingData.png");
	TexMan.LoadTexture("deallocating.png");

	/// Update options menu labels
	Graphics.QueueMessage(new GMSetUIs("ActivePlayerButton", GMUI::TEXT, "Player 1"));
	Graphics.QueueMessage(new GMSetUIs("ActivePlayerInput", GMUI::TEXT, "Keyboard 1"));

	/// Notify that input-manager to use menu-navigation.
	Input.NavigateUI(true);
	// Push the menu to be hovered on straight away, yo.
	Graphics.QueueMessage(new GMPushUI("MainMenu"));
};

void MainMenu::OnExit(GameState *nextState){
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	std::cout<<"\nLeaving MainMenu state.";
	// Load initial texture and set it to render over everything else
	if (nextState->GetID() != GAME_STATE_EXITING)
		Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));
	else
		Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/deallocating.png")));
}

#include "../UI/UserInterface.h"
#include "../Graphics/Messages/GMUI.h"

void MainMenu::Process(float time)
{
	Sleep(50);
#ifdef USE_AUDIO
	AudioMan.Update();
#endif
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MainMenu::ProcessMessage(Message * message){
	std::cout<<"\nState::ProcessMessage called:";
	switch(message->type){
		case MessageType::STRING: {
			String s = message->msg;
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (s == "NEW_GAME"){
				MapState * state = (MapState*)StateMan.GetState(RUNE_GAME_STATE_MAP);
				assert(state);
				if (state){
					state->SetEnterMode(EnterMode::NEW_GAME);
					StateMan.QueueState(state->GetID());
				}
			//	InputProcessor(GO_TO_RACING_STATE);
			//	StateMan.QueueState(GAME_STATE_RACING);
			}
			else if (s == "go_to_main_menu")
				StateMan.QueueState(GAME_STATE_MAIN_MENU);
			else if (s == "go_to_editor")
				StateMan.QueueState(GAME_STATE_EDITOR);
			else if (s == "go_to_options")
				Graphics.QueueMessage(new GMSet(ACTIVE_USER_INTERFACE, (void*)NULL));
			else if (s == "exit")
				StateMan.QueueState(GAME_STATE_EXITING);
			else if (s == "begin_input(this)"){
				UserInterface * ui = StateMan.ActiveState()->GetUI();
				UIElement * element = ui->GetActiveElement();
				if (element != NULL){
					assert(element->onTrigger);
					Input.EnterTextInputMode(element->onTrigger);
					if (element->text)
						Input.SetInputBuffer(element->text.c_str());
				}
				else
					assert(false && "NULL-element :<");
			}
			else {
				std::cout<<"\nUndefined message received: "<<message->msg;
			}
			break;
		}
	}
}

/// Creates bindings that are used for debugging purposes only
void MainMenu::CreateDefaultBindings(){
	std::cout<<"\nMainMenu::CreateDefaultBindings() called";
	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	InputMapping * mapping = &this->inputMapping;
	mapping->CreateBinding(GO_TO_AI_TEST, KEY::CTRL, KEY::G, KEY::A);
	mapping->CreateBinding(GO_TO_RACING_STATE, KEY::CTRL, KEY::G, KEY::R);
	mapping->CreateBinding(GO_TO_NETWORK_TEST, KEY::CTRL, KEY::G, KEY::N);
	mapping->CreateBinding(GO_TO_BLUEPRINT_EDITOR, KEY::CTRL, KEY::G, KEY::S);

	/// Menu navigation, yush!
	mapping->CreateBinding(PREVIOUS_UI_ELEMENT, KEY::SHIFT, KEY::TAB);
	mapping->CreateBinding(NEXT_UI_ELEMENT, KEY::TAB);
	mapping->CreateBinding(ACTIVATE_UI_ELEMENT, KEY::ENTER);
};

/// Creates the user interface for this state
void MainMenu::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("MainMenu.gui");
}
