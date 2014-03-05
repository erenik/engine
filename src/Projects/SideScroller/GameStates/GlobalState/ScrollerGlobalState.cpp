// Awesome Author

#include "Game/GameType.h"

#ifdef RUNE_RPG

#include "OS/Sleep.h"
#include "ScrollerGlobalState.h"
#include "UI/UserInterface.h"
#include "Battle/BattleManager.h"

String applicationName = "RuneRPG";

extern UserInterface * ui[MAX_GAME_STATES];
#define UI ui[StateMan.currentGameState];

#include "Graphics/Messages/GMUI.h"
#include "UI/UIButtons.h"
#include <iomanip>
#include "Message/Message.h"
#include "Player/PlayerManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Battle/BattleAction.h"


#include "Graphics/GraphicsManager.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"
#include "Input/InputManager.h"

#include "Game/GameVariables.h"
#include "File/File.h"
#include "../RuneGameStatesEnum.h"
#include "Graphics/Fonts/Font.h"

#include "Maps/Grids/GridObject.h"

ScrollerGlobalState::ScrollerGlobalState(){
	id = GAME_STATE_GLOBAL;
	TextFont::defaultFontSource = "font3.png";
}

void ScrollerGlobalState::OnEnter(GameState * previousState){


	/// Set working directory to this folder so that makeRelative works as it should.
	FilePath::workingDirectory = "SideScroller";

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));

	/// Create some default variables needed!
	GameVars.CreateInt("PlayTime");
	int sessionStartTime = Timer::GetCurrentTimeMs();
	GameVars.CreateInt("SessionStartTime", sessionStartTime);

}

void ScrollerGlobalState::OnExit(GameState * nextState){
	std::cout<<"\nLeaving ScrollerGlobalState state.";

	// Load initial texture and set it to render over everything else
	if (nextState == NULL)
		Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/deallocating.png"));
}

/// For key-bindings.
void ScrollerGlobalState::CreateDefaultBindings()
{

}
/// For key-bindings.
void ScrollerGlobalState::InputProcessor(int action, int inputDevice /*= 0*/)
{

}


void ScrollerGlobalState::ProcessPacket( Packet * packet ){

}

void ScrollerGlobalState::ProcessMessage( Message * message ){
	String msg = message->msg;
	msg.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (msg.Contains("BattleTest") || msg.Contains("TestBattle")){
		BattleMan.QueueBattle("Practice");
		StateMan.QueueState(RUNE_GAME_STATE_BATTLE_STATE);
	}
	else if (msg.Contains("EnterState(")){
		/// Fetch name.
		String name = msg.Tokenize("()")[1];
		/// Fetch state
		GameState * gs = StateMan.GetStateByName(name);
		if (gs){
			StateMan.QueueState(gs->GetID());
		}
		else
			std::cout<<"\nEnterState: No such state with name: "<<name;
	}
	else if (msg.Contains("GoToPreviousGameState()")){
		GameState * gs = StateMan.PreviousState();
		if (gs)
			StateMan.QueueState(gs->GetID());
	}
	else if (msg.Contains("SaveGameNew")){
		/// Set an appropriate name for the save.
		GameVariables * var = GameVars.GetString("PlayerName");
		String playerName;
		if (!var){
			std::cout<<"\nWARNING: No name set, setting default name.";
			playerName = "DefaultName";
		}else {
			playerName = var->Get();
		}
		int previousPlayTime = GameVars.GetInt("PlayTime");
		int sessionStart = GameVars.GetInt("SessionStartTime");
		int currentTime = Timer::GetCurrentTimeMs();
		int sessionTotal = currentTime - sessionStart;
		assert(sessionTotal > 0);
		int gameTime = previousPlayTime + sessionTotal;
		gameTime /= 1000;
		String gameTimeString = String::ToString(gameTime);
		String saveName = playerName;
		std::cout<<"Saving as: "<<saveName;
		
		/// Save it.
		SaveFile save;
		String customHeaderData = playerName + " Level 1";
		std::fstream * stream = save.Open(saveName, "RuneRPG", customHeaderData, false);
		if (!stream){
			std::cout<<"\nUnable to open a valid save-file for writing, aborting! D:";
			return;
		}
		
		save.Close();

		/// Update the file with saves, if we use one?

	}
	else if (msg.Contains("ListSaves(")){
		List<String> tokens = msg.Tokenize("(),");
		if (tokens.Size() < 2)
			return;
		String uiName = tokens[1];
		float sizeRatioY = 0.2f;
		// First clear it.
		Graphics.QueueMessage(new GMClearUI(uiName));
		// Add a "save new" option first.
		UIElement * saveNewButton = new UIButton("Save New");
		saveNewButton->sizeRatioY = sizeRatioY;
		saveNewButton->activationMessage = "SaveGameNew";
		Graphics.QueueMessage(new GMAddUI(saveNewButton, uiName));
		// Then find saves.
		// For each file, just add crap.
		List<SaveFileHeader> saveHeaders = SaveFile::GetSaves("RuneRPG");
		// Add a button for each save.
		for (int i = 0; i < saveHeaders.Size(); ++i){
			SaveFileHeader saveFile = saveHeaders[i];
			UIElement * saveNewButton = new UIButton("OverwriteSave:"+String::ToString(i));
			saveNewButton->text = saveFile.customHeaderData;
			saveNewButton->sizeRatioY = sizeRatioY;
			saveNewButton->activationMessage = "OverwriteSaveGame("+saveFile.fileName+")";
			Graphics.QueueMessage(new GMAddUI(saveNewButton, uiName));
		}
		// Add them.
	}
	else if (msg.Contains("LoadSavesList(")){
		List<String> tokens = msg.Tokenize("(),");
		if (tokens.Size() < 2)
			return;
		String uiName = tokens[1];
		float sizeRatioY = 0.2f;
		// First clear it.
		Graphics.QueueMessage(new GMClearUI(uiName));
		// For each file, just add crap.
		List<SaveFileHeader> saveHeaders = SaveFile::GetSaves("RuneRPG");
		// Add a button for each save.
		for (int i = 0; i < saveHeaders.Size(); ++i){
			SaveFileHeader saveFile = saveHeaders[i];
			UIElement * saveNewButton = new UIButton("LoadFile"+String::ToString(i));
			saveNewButton->text = saveFile.customHeaderData;
			saveNewButton->sizeRatioY = sizeRatioY;
			saveNewButton->activationMessage = "LoadGame("+saveFile.fileName+")";
			Graphics.QueueMessage(new GMAddUI(saveNewButton, uiName));
		}
		// Then find saves.
		// Add them.
	}
}

void ScrollerGlobalState::Process(float time){
	Sleep(10);

	// Calculate time since last update
	clock_t newTime = clock();
	int timeDiff = newTime - lastTime;
	lastTime = newTime;
}

void ScrollerGlobalState::CreateUserInterface(){

}

void ScrollerGlobalState::MouseClick(bool down, int x, int y, UIElement * elementClicked){

}

void ScrollerGlobalState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){

}

void ScrollerGlobalState::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){

}
void ScrollerGlobalState::MouseWheel(float delta){

}

void ScrollerGlobalState::OnSelect(Selection &selection){

}

#endif
