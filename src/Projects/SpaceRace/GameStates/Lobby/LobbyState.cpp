// Emil Hedemalm
// 2013-07-10

#include "LobbyState.h"
#include "Graphics/GraphicsManager.h"
#include "Player/PlayerManager.h"
#include "Message/Message.h"
#include "Graphics/Messages/GMUI.h"
#include "Player/Player.h"
#include "OS/Sleep.h"
#include "Input/InputMapping.h"
#include "Input/Keys.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "File/FileUtil.h"
#include "Maps/MapManager.h"
#include "Physics/PhysicsManager.h"
#include "../../Ship.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Message/MessageManager.h"
#include "../../ShipManager.h"
#include "../../SRPlayer.h"
#include "../../SRDirectories.h"
#include "Network/NetworkSettings.h"
#include "Network/NetworkManager.h"
#include "UI/UIList.h"
#include "UI/UIButtons.h"
#include "../../SRConstants.h"
#include "Graphics/Messages/GMSet.h"
#include "Input/InputDevices.h"
#include "Chat/ChatManager.h"
#include "Network/NetworkClient.h"
#include "../../Network/SRSession.h"
#include "String/StringUtil.h"
#include "UI/UIInput.h"

#include "Network/Packet/Packet.h"

#include "../../Network/SRPacketTypes.h"
#include "../../Network/SRPacket.h"

enum actions {
	NULL_ACTION,
	INCREASE_LOCAL_PLAYERS,
	DECREASE_LOCAL_PLAYERS,
	INCREASE_AI_PLAYERS,
	DECREASE_AI_PLAYERS,
	NEXT_UI_ELEMENT,
	PREVIOUS_UI_ELEMENT,
	ACTIVATE_UI_ELEMENT,
};

LobbyState::LobbyState(){
    stateName = "Lobby state";
	id = GAME_STATE_LOBBY;
	requestedPlayers = 1;
	activePlayer = NULL;
	shipSelector = -1;
}

LobbyState::~LobbyState()
{

}


void LobbyState::OnEnter(GameState * previousState){
	// Load initial texture and set it to render over everything else
	std::cout<<"\nLobbyState::OnEnter.";

	/// Set camera!
	SetCameraProjection3D();

	Graphics.sleepTime = 3;
	Graphics.outOfFocusSleepTime = 10;

    Input.NavigateUI(true);
    Input.ForceNavigateUI(true);
	// Set gravity before loading shit

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));
	// Set 1 playah if network wasn't active.
	if (PlayerMan.NumPlayers() == 0){
        Ship * ship = ShipMan.GetShips()[0];
        assert(ship);
		PlayerMan.AddPlayer(new SRPlayer("Player 1", ship));
	}
	// Update UI
	Session * gameSession = GetSession();
	isHost = gameSession->IsHost();

	List<String> hostSpecificElements;
	hostSpecificElements += "StartGame";
	hostSpecificElements += "AddAIPlayer";
	hostSpecificElements += "RemoveAIPlayer";
	hostSpecificElements += "Gravity";
	hostSpecificElements += "MaxPlayers";
	hostSpecificElements += "TrackStats";
	for (int i = 0; i < hostSpecificElements.Size(); ++i){
		String element = hostSpecificElements[i];
		if (isHost){
			EnableButton(element);
		}
		else {
			DisableButton(element);
		}
	}
	Graphics.QueueMessage(new GMSetUIb("StartGame", GMUI::ACTIVATABLE, isHost));
	Graphics.QueueMessage(new GMSetUIb("StartGame", GMUI::HOVERABLE, isHost));

//	DisableButton("ReadyButton");
//	DisableButton("ShipStats");
	DisableButton("MaxPlayers");

	OnPlayersUpdated();

	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	/// Update the chatlog
    OnChatMessageReceived(NULL);
}

void LobbyState::Process(float time){
}


void LobbyState::OnExit(GameState * nextState){
	std::cout<<"\nLobbyState::OnExit.";

	if (nextState->GetID() == GAME_STATE_RACING && selectedLevel.Length() > 1)
		nextState->ProcessMessage(new Message("SetLevel("+selectedLevel+")"));

	// Load initial texture and set it to render over everything else
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
}
void LobbyState::CreateDefaultBindings(){
	std::cout<<"\n"<<this->stateName<<"::CreateDefaultBindings() called";
	/// Get pointer to this mapping
	InputMapping * mapping = &inputMapping;

	mapping->CreateBinding("IncreaseAIPlayers", KEY::CTRL, KEY::PLUS)->activateOnRepeat = true;
	mapping->CreateBinding("DecreaseAIPlayers", KEY::CTRL, KEY::MINUS)->activateOnRepeat = true;

	mapping->SetBlockingKeys(mapping->CreateBinding("IncreaseLocalPlayers", KEY::PLUS), KEY::CTRL)->activateOnRepeat = true;
	mapping->SetBlockingKeys(mapping->CreateBinding("DecreaseLocalPlayers", KEY::MINUS), KEY::CTRL)->activateOnRepeat = true;
}

void LobbyState::CreateUserInterface()
{
	if (ui)
		delete ui;
	this->ui = new UserInterface();
	ui->Load("gui/Lobby.gui");
}

void LobbyState::InputProcessor(int action, int inputDevice/* = 0*/){

}

void LobbyState::ProcessMessage(Message * message)
{
	String s = message->msg;
	s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (s == "SetMaxPlayers"){
		// Read in player amount from UI
		UIElement * u = ui->GetElementByName("MaxPlayers");
		if (!u){
			return;
		}
		int n = u->text.ParseInt();
		if (n > 0){
			PlayerMan.SetMaxPlayers(n);
		}
		int maxPlayers = PlayerMan.MaxPlayers();
		String maxPlayersString = String::ToString(maxPlayers);
		Graphics.QueueMessage(new GMSetUIs("MaxPlayers", GMUI::TEXT, maxPlayersString));
		OnPlayersUpdated();
	}
	else if (s == "OnReloadUI")
	{
        OnPlayersUpdated();
	}
	else if (s == "OnHostDisconnected"){
		/// Display a message and maybe pull up a button return to the lobby or out to the network-join menu?
		assert(false);
	}
	// Woo!
	else if (s == "ENTER_RACING_STATE"){
		StateMan.QueueState(GAME_STATE_RACING);
	}
	/// Sent after a map has been selected.
	else if (s.Contains("MapSetTo(")){
		String mapName = s.Tokenize("()")[1];
		ChatMan.PostGeneral("Map set: "+mapName);
	}
	/// Message sent after all players have readied up and settings have been locked.
	else if (s.Contains("MoveToRacingState(")){
		String timeString = s.Tokenize("()")[1];
		int seconds = timeString.ParseInt();
		ChatMan.PostGeneral("Moving to racing state in "+timeString+" seconds.");
		long long timeToEnterRacingState = Timer::GetCurrentTimeMs() + seconds * 1000;
		Message * message = new Message(MessageType::STRING);
		message->msg = "ENTER_RACING_STATE";
		message->timeToProcess = timeToEnterRacingState;
		MesMan.QueueDelayedMessage(message);
	}
	else if (s == "GameSettingsLocked"){
		OnGameSettingsLocked(true);
	}
	else if (s == "GameSettingsUnlocked"){
		OnGameSettingsLocked(false);
	}
	else if (s.Contains("PeersStillNotReady(")){
		String peerNameString = s.Tokenize("()")[1];
		List<String> peerNames = peerNameString.Tokenize(";");
		ChatMan.PostGeneral("Waiting for peers to get ready: "+MergeLines(peerNames, ", "));
	}
	else if (s.Contains("ToggleReady(")){
		String ref = s.Tokenize("()")[1];
		bool value;
		if (ref == "this")
		{
			value = message->element->toggled;
		}
		else
			return;
		SRSession * srs = GetSession();
		srs->SetReady(value);
	}
	else if (s.Contains("SetPreviewLevel")){
		if (!isHost)
			return;
		List<String> tokens = s.Tokenize("()");
		assert(tokens.Size() >= 2);
		String mapName = tokens[1];
		if (!mapName.Contains("map/racing"))
			mapName = "map/racing/" + mapName;

		Map * map = MapMan.LoadMap(mapName);
		if (map){
			MapMan.MakeActive(map);
			previewLevel = mapName;
			String mapState = "Name: "+map->Name()+"\nCreator: "+map->Creator()+"\nLevel entities: "+String::ToString(map->NumEntities());
			Graphics.QueueMessage(new GMSetUIs("PreviewTrackStats", GMUI::TEXT, mapState));
		}
		else {
			Graphics.QueueMessage(new GMSetUIs("PreviewTrackStats", GMUI::TEXT, "Unable to load the level "+mapName+"!"));
		}
	}
	else if (s.Contains("SetPreviewShip")){
		List<String> tokens = s.Tokenize("()");
		assert(tokens.Size() >= 2);
		if (tokens.Size() < 2)
			return;
		String shipName = tokens[1];
		MapMan.DeleteEntities();
		Ship * ship = ShipMan.GetShip(shipName);
		if (ship){
			previewShip = shipName;
			MapMan.CreateEntity(ModelMan.GetModel(ship->modelSource), TexMan.GetTextureBySource(ship->diffuseSource));
			String shipInfo = "Name: "+ship->name+"\nCreator: "+ship->creator+
				"\nThrust: "+String::ToString(ship->thrust)+"\nBoost: "+String::ToString(ship->boostMultiplier);
			float lines = 5.0f;
			Graphics.QueueMessage(new GMSetUIf("PreviewShipStats", GMUI::TEXT_SIZE_RATIO, 1/lines));
			Graphics.QueueMessage(new GMSetUIs("PreviewShipStats", GMUI::TEXT, shipInfo));
		}
		else {
			Graphics.QueueMessage(new GMSetUIs("PreviewShipStats", GMUI::TEXT, "Unable to load the ship "+shipName+"!"));
		}
	}
	else if (s == "SetGravity"){
		if (!isHost)
			return;
		float gravity = ui->GetElementByName("Gravity")->text.ParseFloat();
		Physics.QueueMessage(new PMSetGravity(Vector3f(0,gravity,0)));
	}
	else if (s == "SelectLevel"){
		if (!isHost)
			return;
		GetSession()->SetMap(previewLevel);
		selectedLevel = previewLevel;
	}
	else if (s == "SelectShip"){
		if (shipSelector == -1){
			ChatMan.PostGeneral("Unable to select ship since no player was associated with the choice.");
			return;
		}
		SRSession * srs = GetSession();
		/// Inform session of our choice too.
		srs->SelectShip(shipSelector, previewShip);
		shipSelector = -1;
	}
	else if (s == "OnEnterLevelSelector"){
		if (!isHost)
			return;
		OnEnterLevelSelector();
	}
	///
	else if (s == "EnterShipSelector"){
		MesMan.QueueMessages("OnEnterShipSelector&setVisibility(MainLobbyScreen,false)&setVisibility(ShipSelector,true)");
	}
	else if (s == "OnEnterShipSelector"){
		OnEnterShipSelector();
	}
	else if (s.Contains("SetPlayerShip(")){
		int playerIndex = s.Tokenize("()")[1].ParseInt();
		SRPlayer * player = GetPlayer(playerIndex);
		if (!player){
			ChatMan.PostGeneral("Invalid index");
			return;
		}
		this->shipSelector = player->ID();
		MesMan.QueueMessages("EnterShipSelector");
	}
	else if (s.Contains("RemovePlayer(")){
		if (!isHost)
			return;
		SRSession * srs = GetSession();
		String playerIndex = s.Tokenize(" (,")[1];
		int pi = playerIndex.ParseInt();
		srs->RemovePlayerByIndex(pi);
	}
	else if (s.Contains("SelectInputDevice(")){
        String playerIndex = s.Tokenize(" (,)")[1];
		int pi = playerIndex.ParseInt();
		Player * pl = GetSession()->GetPlayer(pi);
		if (!pl->isLocal)
			return;
		int device = PlayerMan.SetPlayerInputDevice(pl, pl->InputDevice()+1);
        Graphics.QueueMessage(new GMSetUIs("SelectPlayerInput" + playerIndex, GMUI::TEXT, GetInputDeviceName(device)));
	}
	else if (s == "IncreaseLocalPlayers"){
		SRSession * srs = GetSession();
		srs->AddLocalPlayer();
		//IncreaseLocalPlayers();
	}
	else if (s == "DecreaseLocalPlayers"){
		SRSession * srs = GetSession();
		srs->RemoveLocalPlayer();
		//DecreaseLocalPlayers();
	}
	/// Message from the SRSession that player-list has been updated,
	else if (s == "OnPlayersUpdated"){
		/// So update GUI.
		this->OnPlayersUpdated();
	}
	else if (s == "IncreaseAIPlayers"){
		GetSession()->AddAIPlayer();
	}
	else if (s == "DecreaseAIPlayers"){
		GetSession()->RemoveAIPlayer();
	}
    else if (s == "PostChatMessage(this)"){
        UIElement * e = message->element;
        assert(e);
        String message = e->text;
     //   ChatMessage * cm = new ChatMessage(PlayerMan.Me(), message, ChatMessage::PUBLIC);
     //   ChatMan.AddMessage(cm);

        message = PlayerMan.Me()->name + ": " + message;
      //  NetworkMan.QueuePacket(new PacketChat(message.c_str()));
    }
	else if (s.Contains("SetPlayerName(")){
		List<String> tokens = s.Tokenize("(),");
		String playerName = tokens[1];
		String newName = message->element->text;
		List<Player*> players = PlayerMan.GetPlayers();
		for (int i = 0; i < players.Size(); ++i){
			Player * p = players[i];
			if (p->name == playerName){
				p->name = newName;
				return;
			}
		}
	}
	/// Only host can start the game.
	else if (s == "startGame"){
		Session * session = GetSession();
		/// If not in network (no peers), just start!
		if (!session->peers.Size()){
			StateMan.QueueState(GAME_STATE_RACING);
			return;
		}
		// Networked session, check if we are host.
		if (!session->IsHost())
			return;

		// Send a start countdown packet or something..!
		SRRacePacket racePack("Start");
		session->Send(&racePack);

//		Network.QueuePacket(playersPacket);
//	    Network.QueuePacket(new Packet(SRP_START_GAME, PACKET_TARGET_OTHERS));
		StateMan.QueueState(GAME_STATE_RACING);
	}
}

void LobbyState::OnChatMessageReceived(ChatMessage * chatMessage){
    std::cout<<"\nLobbyState::OnChatMessageReceived";
    Graphics.QueueMessage(new GMClearUI("Chatlog"));
    std::cout<<"\nClearing chatlog";
    // Do stuff.
    List<ChatMessage*> chatMessages = ChatMan.GetLastMessages(20);
    ChatMessage * cm;
    for (int i = 0; i < chatMessages.Size(); ++i){
        cm = chatMessages[i];
        std::cout<<"\nChat message "<<i;

        UILabel * l = new UILabel();
   //     std::cout<<"\nCreating new label..";
        switch(cm->type){
            case ChatMessage::GLOBAL_ANNOUNCEMENT:
                break;
			default:
				l->text = cm->playerName+": ";
            ;
      //          l->text = cm->playerName +": ";
        }
        l->text += cm->text;
        l->sizeRatioY = 0.15f;
        Graphics.QueueMessage(new GMAddUI(l, "Chatlog"));
    }
    /// Scroll dauown (100 pages) to it!
    Graphics.QueueMessage(new GMScrollUI("Chatlog", -100.0f));
}


void LobbyState::OnGameSettingsLocked(bool locked)
{
	if (locked)
		ChatMan.PostGeneral("Game settings locked.");
	else
		ChatMan.PostGeneral("Game settings unlocked.");
	/// Lock ui as needed.
	bool hoverable = locked;
//	...
};

void LobbyState::DisableButton(String elementName)
{
	Graphics.QueueMessage(new GMSetUIv4f(elementName, GMUI::TEXT_COLOR, Vector4f(0.5f,0.5f,0.5f,1)));
	Graphics.QueueMessage(new GMSetUIb(elementName, GMUI::HOVERABLE, false));
}
void LobbyState::EnableButton(String elementName)
{
	Graphics.QueueMessage(new GMSetUIv4f(elementName, GMUI::TEXT_COLOR, Vector4f(1,1,1,1)));
	Graphics.QueueMessage(new GMSetUIb(elementName, GMUI::HOVERABLE, true));
}

// Update UI
void LobbyState::OnPlayersUpdated()
{
	SRSession * session = GetSession();
	/// Grab initial element so we get steal default-values.
	List<UIElement*> toAdd;
	/// Create new content to throw into it.
	List<SRPlayer*> players = session->GetPlayers();
	for (int i = 0; i < players.Size(); ++i){
		SRPlayer * player = players[i];
		String playerIndex = String::ToString(i);
		// Add a main div.
		UIColumnList * playerDiv = new UIColumnList("Player"+String::ToString(i)+"Row");
		playerDiv->padding = 0.01f;
		playerDiv->sizeRatioY = 0.1f;
		playerDiv->textureSource = "80Gray50Alpha";
		playerDiv->SetName("Player"+playerIndex+"Div");

		UIElement * playerName;
		UIElement * playerShip;
		if (!player->isAI && player->isLocal){
			playerName = new UITextField();
			playerName->onTrigger = "SetPlayerName("+player->name+",this)";
			playerName->textureSource = "80Gray50Alpha";
			playerShip = new UIButton();
			playerShip->textureSource = "80Gray50Alpha";
			playerShip->activationMessage = "SetPlayerShip("+playerIndex+")";
		}
		else {
			playerName = new UILabel();
			playerShip = new UILabel();
		}
		playerName->name = "Player"+playerIndex+"Name";
		playerName->sizeRatioX = 0.3f;
		playerName->text = player->name;
		playerDiv->AddChild(playerName);

		playerShip->name = "Player"+playerIndex+"Ship";
		playerShip->sizeRatioX = 0.3f;
		playerShip->text = "shipship";
		if (player->ship)
			playerShip->text = player->ship->name;
		playerDiv->AddChild(playerShip);

/*
#ifdef USE_NETWORK
		playerName->text = (((SRPlayer *)PlayerMan.Get(i))->clientIndex >= 0)? Network.GetClient(((SRPlayer *)PlayerMan.Get(i))->clientIndex)->name : "Player " + String::ToString(i+1);
#else
		playerName->text = "Player " + String::ToString(i+1);
#endif
		*/

		if (player->isLocal && !player->isAI){
            UIButton * inputSelectButton = new UIButton();
            inputSelectButton->name = "SelectPlayerInput" + playerIndex;
            inputSelectButton->textureSource = "80Gray50Alpha";
            inputSelectButton->text = GetInputDeviceName(player->InputDevice());
            inputSelectButton->activationMessage = "SelectInputDevice("+playerIndex+")";
            inputSelectButton->sizeRatioX = 0.2f;
            playerDiv->AddChild(inputSelectButton);
        }

		if (isHost)	{
			UIButton * kickButton = new UIButton("Kick");
			kickButton->textColor = Vector4f(1.0f,0.5f,0.5f,1.0f);
			kickButton->activationMessage = "RemovePlayer(" + playerIndex+ ")";
			kickButton->sizeRatioX = 0.1f;
			playerDiv->AddChild(kickButton);
		}
		toAdd.Add(playerDiv);
	}
	/// Clear player list after we're ready to re-fill it..!
	Graphics.QueueMessage(new GMClearUI("PlayerList"));
	for (int i =0; i < toAdd.Size(); ++i){
		Graphics.QueueMessage(new GMAddUI(toAdd[i], "PlayerList"));
	}
}


// Grab file-data
void LobbyState::OnEnterLevelSelector(){

    /// Reset camera if it was off.
    Graphics.QueueMessage(new GraphicsMessage(GM_RESET_CAMERA));

	List<String> files;
	int result = GetFilesInDirectory("map/racing", files);
	if (result == 1){
		// Clear the list and enter a button for each map!
		Graphics.QueueMessage(new GMClearUI("TrackList"));
		Graphics.QueueMessage(new GMSetUIs("TrackList", GMUI::TEXT, ""));
		for (int i = 0; i < files.Size(); ++i){
			UIButton * levelButton = new UIButton();
			levelButton->text = files[i];
			levelButton->sizeRatioY = 0.1f;
			levelButton->textureSource = "80Gray50Alpha";
			levelButton->activationMessage = "SetPreviewLevel("+files[i]+")";
			Graphics.QueueMessage(new GMAddUI(levelButton, "TrackList"));
		}
	}
	else {
		Graphics.QueueMessage(new GMSetUIs("TrackList", GMUI::TEXT, "Track folder empty"));
	}
}


// Grab file-data
void LobbyState::OnEnterShipSelector(){
	bool result = MapMan.MakeActive(SHIP_SELECTOR_LEVEL);
	if (!result){
		MapMan.CreateMap(SHIP_SELECTOR_LEVEL);
		result = MapMan.MakeActive(SHIP_SELECTOR_LEVEL);
		assert(result);
	}
	/// Reset camera if it was off.
    Graphics.QueueMessage(new GraphicsMessage(GM_RESET_CAMERA));

	result = ShipMan.LoadFromDirectory(SHIP_DIR);
	if (result == 1){
		// Clear the list and enter a button for each map!
		Graphics.QueueMessage(new GMClearUI("ShipList"));
		Graphics.QueueMessage(new GMSetUIs("ShipList", GMUI::TEXT, ""));
		List<String> shipNames = ShipMan.GetShipNames();
		for (int i = 0; i < shipNames.Size(); ++i){
			UIButton * levelButton = new UIButton();
			levelButton->text = shipNames[i];
			levelButton->sizeRatioY = 0.1f;
			levelButton->textureSource = "80Gray50Alpha";
			levelButton->activationMessage = "SetPreviewShip("+shipNames[i]+")";
			Graphics.QueueMessage(new GMAddUI(levelButton, "ShipList"));
		}
	}
	else {
		Graphics.QueueMessage(new GMSetUIs("ShipList", GMUI::TEXT, "Ship folder empty"));
	}
}
