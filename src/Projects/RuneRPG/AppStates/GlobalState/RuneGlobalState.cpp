// Emil Hedemalm
// 2014-04-06
// Global app/game-state for the RuneRPG game.

#include "OS/Sleep.h"
#include "RuneGlobalState.h"
#include "UI/UserInterface.h"
#include "Battle/BattleManager.h"
#include "Maps/Grids/TileTypeManager.h"
#include "Script/Script.h"
#include "Script/ScriptManager.h"

String applicationName = "RuneRPG";

extern UserInterface * ui[GameStateID::MAX_GAME_STATES];
#define UI ui[StateMan.currentGameState];

#include "Graphics/Messages/GMUI.h"
#include "UI/UIButtons.h"
#include <iomanip>
#include "Message/Message.h"
#include "RuneRPG/Battle/RuneBattlerManager.h"
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
#include "Graphics/Fonts/TextFont.h"

#include "Maps/Grids/GridObject.h"
#include "UI/UIInputs.h"
#include "RuneRPG/Network/RRPacket.h"
#include "RuneRPG/PopulationManager.h"
#include "RuneRPG/Battle.h"

#include "Window/WindowManager.h"

RuneGlobalState::RuneGlobalState()
{
	id = GameStateID::GAME_STATE_GLOBAL;
	TextFont::defaultFontSource = "font3.png";
	targetPort = RR_DEFAULT_PORT;
	targetIP = "127.0.0.1";
}

void RuneGlobalState::OnEnter(AppState * previousState)
{
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetGlobalUI(ui));

	WindowMan.lockChildWindowRelativePositions = true;

	/// Allocate all necessary managers!
	if (!BattleManager::IsAllocated()){
		BattleManager::Allocate();
		RuneBattlerManager::Allocate();
		BattleActionLibrary::Allocate();
//		BALib.LoadFromDirectory(ACTIONS_DIRECTORY);
//		RuneBattlers.LoadFromDirectory(BATTLERS_DIRECTORY);
//		RuneBattlers.LoadBattles(BATTLES_DIRECTORY);
		TileTypeManager::Allocate();

		RuneBattleActionLibrary::Allocate();

		/// Create Attack, etc.
		RBALib.CreateDefaultActions();
		/// Load from file!
		RBALib.LoadSpellsFromCSV("data/Spells.csv"); // Mage-magic focused spells
		RBALib.LoadSkillsFromCSV("data/Skills.csv"); // Combat magical skills
		RBALib.LoadMundaneAbilitiesFromCSV("data/Abilities.csv"); // Mundane actions/abilities

		/// Load battlers 
		RuneBattlers.LoadBattlersFromCSV("data/TestCharactersMage.csv");
		RuneBattlers.LoadBattlersFromCSV("data/TestCharactersWarrior.csv");
		RuneBattlers.LoadBattlersFromCSV("data/Enemies.csv");

		// Enter some tile types into the manager
		TileTypes.CreateDefaultTiles();
		PopulationManager::Allocate();

		RuneBattle::LoadBattleFunctions("data/Equations.txt");
	}
	// Load grid object types.
	GridObjectTypeMan.SetSavePath("data/Map/objects.dat");
	GridObjectTypeMan.Load();

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GMSet(GT_OVERLAY_TEXTURE, (Texture*)NULL));

	/// Create some default variables needed!
	GameVars.CreateInt("PlayTime");
	long long sessionStartTime = Timer::GetCurrentTimeMs();
	GameVars.CreateInt64("SessionStartTime", sessionStartTime);


	/// Run enter script.
	Script * onEnter = new Script();
	onEnter->SetDeleteOnEnd(true);
	onEnter->lines = File::GetLines("OnEnter.ini");
	onEnter->loaded = true;
	ScriptMan.PlayScript(onEnter);

}

void RuneGlobalState::OnExit(AppState * nextState)
{
	std::cout<<"\nLeaving RuneGlobalState state.";

	if (BattleManager::IsAllocated())
	{
	    BattleActionLibrary::Deallocate();
		BattleManager::Deallocate();
		RuneBattlerManager::Deallocate();
		PopulationManager::Deallocate();
		RuneBattleActionLibrary::Deallocate();
	}

	// Load initial texture and set it to render over everything else
//	if (nextState == NULL)
	//	Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/deallocating.png"));
}

void RuneGlobalState::ProcessPacket( Packet * packet ){

}

void RuneGlobalState::ProcessMessage( Message * message )
{
	String msg = message->msg;
	msg.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (msg.Contains("BattleTest") || msg.Contains("TestBattle")){
		BattleMan.QueueBattle("Practice");
		StateMan.QueueState(StateMan.GetStateByID(RUNE_GAME_STATE_BATTLE_STATE));
	}
	else if (msg == "LoadMultiplayerDefaults")
	{
		// TODO: Actually load something.
		Graphics.QueueMessage(new GMSetUIs("IP", GMUI::STRING_INPUT_TEXT, "127.0.0.1"));
		Graphics.QueueMessage(new GMSetUIs("Port", GMUI::INTEGER_INPUT_TEXT, "33000"));
	}
	else if (msg == "Host game")
	{
		// Host session.
		bool result = this->Host(targetPort);
		if (result)
		{
			// Go to lobby!
			NetworkLog("Game hosted.");
			// Push lobby ui.
			Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", ui));
		}
		else 
		{
			// Inform user.
			NetworkLog("Unable to host game.");
		}
	}
	else if (msg == "Join game")
	{
		bool result = this->Join(targetIP, targetPort);
	}
	else if (msg == "CancelGame")
	{
		// Abort hosted game
		this->CancelGame();
	}
	else if (msg == "SetGameType(New)")
	{
		// Set flags to enable editing of characters and in-game options.
		session->NewGame();
	}
	else if (msg == "PrepareForLoadGame")
	{
		// Set label in save/load menu.
		Graphics.QueueMessage(new GMSetUIs("SavesMenuActionLabel", GMUI::TEXT, "Load game"));
	}
	else if (msg == "Load game")
	{
	
	}
	else if (msg == "StartButton")
	{
		/// Lock settings completely, lock players and peers and load starting-script.
		session->StartGame();
	}
	else if (msg == "ChatInput")
	{
		// Fetch text and clear it.
		UITextField * input = (UITextField*)message->element;
		String text = input->text;
		Graphics.QueueMessage(new GMSetUIs("ChatInput", GMUI::TEXT, ""));
		// Post it to current game state and over the session to any peers.
		session->PostChatMessage(text);
	}
	else if (msg.Contains("EnterState(")){
		/// Fetch name.
		String name = msg.Tokenize("()")[1];
		/// Fetch state
		AppState * gs = StateMan.GetStateByName(name);
		if (gs){
			StateMan.QueueState(gs);
		}
		else
			std::cout<<"\nEnterState: No such state with name: "<<name;
	}
	else if (msg.Contains("GoToPreviousGameState()")){
		AppState * gs = StateMan.PreviousState();
		if (gs)
			StateMan.QueueState(gs);
	}
	else if (msg.Contains("PlayersReady("))
	{
		// Sent by session when all players are ready or not.
		bool allReady = msg.Tokenize("()")[1].ParseBool();
		// Adjust enabled-property of start-button for host.
		Graphics.QueueMessage(new GMSetUIb("StartButton", GMUI::ENABLED, allReady));
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
		std::fstream * stream = save.OpenSaveFileStream(saveName, "RuneRPG", customHeaderData, false);
		if (!stream){
			std::cout<<"\nUnable to open a valid save-file for writing, aborting! D:";
			return;
		}
		
		save.Close();

		/// Update the file with saves, if we use one?

	}
	else if (msg.Contains("ListSaves"))
	{

		String uiName = "SavesList";
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
	else if (msg.Contains("UpdateBattlersList("))
	{

		String targetUIList = msg.Tokenize("()")[1];
		// Clear the target ui first.
		UserInterface * inUI = message->element->ui;
		Graphics.QueueMessage(new GMClearUI(targetUIList, inUI));
		List<RuneBattler*> battlers = RuneBattlers.GetBattlers();
		for (int i = 0; i < battlers.Size(); ++i)
		{
			RuneBattler * battler = battlers[i];
			if (battler->name.Length() == 0)
				continue;
			UIButton * addBattlerButton = new UIButton(battler->name);
			addBattlerButton->activationMessage = "AddBattler:"+battler->name;
			addBattlerButton->sizeRatioY = 0.1f;
			Graphics.QueueMessage(new GMAddUI(addBattlerButton, targetUIList, inUI));
		}
	}
}

/*
/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
void RuneGlobalState::NetworkLog(String message)
{
	Graphics.QueueMessage(new GMSetUIs("NetworkLog", GMUI::TEXT, `
}
*/

void RuneGlobalState::Process(int timeInMs)
{
	Sleep(10);
}

void RuneGlobalState::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("GlobalUI.gui");
}

void RuneGlobalState::MouseClick(bool down, int x, int y, UIElement * elementClicked){

}

void RuneGlobalState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){

}

void RuneGlobalState::MouseMove(int x, int y, bool lDown, bool rDown, UIElement * elementOver){

}
void RuneGlobalState::MouseWheel(float delta){

}

void RuneGlobalState::OnSelect(Selection &selection){

}
