// Emil Hedemalm
// 2014-03-05
// 

#define print(arg) std::cout << arg << "\n";

#include "Graphics/GraphicsManager.h"
#include "Physics/PhysicsManager.h"
#include "Game/GameVariables.h"
#include "StateManager.h"

#include "GlobalState.h"
#include "UI/UserInterface.h"
#include "UI/UIButtons.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/Message.h"
#include <iomanip>
#include "OS/Sleep.h"
#include "Player/PlayerManager.h"
#include "../../ShipManager.h"
#include "../../SRDirectories.h"
#include "Graphics/Messages/GMSet.h"
#include "Audio/AudioManager.h"
#include "Audio/TrackManager.h"
#include "../../SRPlayer.h"
#include "UI/UIList.h"
#include "UI/UITypes.h"
#include "Audio/Tracks/Track.h"
#include "System/PreferencesManager.h"
#include "Message/VectorMessage.h"

//#include "Network/NetworkSettings.h"
#include "Network/Packet/Packet.h"
#include "Network/Peer.h"
#include "Network/NetworkManager.h"
#include "Chat/ChatManager.h"
#include "../../Network/SRSession.h"
#include "../../Network/SRSessionData.h"
#include "../../Network/SRPacket.h"
#include "../../Network/SRPacketTypes.h"
#include "../../SRConstants.h"
#include "Graphics/Fonts/Font.h"

#include "Game/Game.h"

/// MAin space race stuff
String applicationName;

extern UserInterface * ui[MAX_GAME_STATES];
#define UI ui[StateMan.currentGameState];


/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults()
{
	applicationName = "Space Race";
	TextFont::defaultFontSource = "font3";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	UserInterface::rootUIDir = "gui/";
}


GlobalState::GlobalState()
: SpaceRaceGameState()
{
	applicationName = "SpaceRace";
    id = GAME_STATE_GLOBAL;
    stateName = "Space Race Global State";
}


GlobalState::~GlobalState()
{

}


void GlobalState::OnEnter(GameState * previousState)
{
	LoadPreferences();

	/// Create the SRSession
	SRSession * srs = new SRSession("Game name");
	NetworkMan.AddSession(srs);

	// Defaultur.
	Physics.QueueMessage(new PMSet(GRAVITY, -250.f));

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));

	/// Allocate managers required by zis game!
	if (!ShipManager::IsAllocated()){
		ShipManager::Allocate();
		ShipMan.LoadFromDirectory(SHIP_DIR);
	}

	// Laps!
	GameVars.CreateInt("Laps", 3);

    Physics.checkType = OCTREE;
	Physics.collissionResolver = CUSTOM_SPACE_RACE_PUSHBACK;
	Physics.integrator = Integrator::SPACE_RACE_CUSTOM_INTEGRATOR;

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	/// Register packet sizes.
//	SRSetPacketSize();

	/// Load tracks
//	TrackMan.CreateTrack("sound/bgm/SpaceRace/.ogg", "Bitsurf", "Race");
	TrackMan.CreateTrack("Bitsurf", "sound/bgm/SpaceRace/2013-08-26_Bitsurf.ogg", "Race");
	TrackMan.CreateTrack("Space race", "sound/bgm/SpaceRace/2013-03-25 Space race.ogg", "Race");
	TrackMan.CreateTrack("Wapp", "sound/bgm/SpaceRace/2014-02-18_Wapp.ogg", "Race");
}

void GlobalState::OnExit(GameState * nextState){
//	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	std::cout<<"\nLeaving GlobalState state.";

	/// Deallocate managers we were using.
	if (ShipManager::IsAllocated()){
		ShipManager::Deallocate();
	}

	// Load initial texture and set it to render over everything else
	if (nextState && nextState->GetID() != GAME_STATE_EXITING)
		Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/loadingData.png"));
	else
		Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/deallocating.png"));

	SavePreferences();
}

void GlobalState::ProcessPacket( Packet * packet ){
	switch(packet->type)
	{
		case PacketType::SR_PACKET:
		{
			SRPacket * srp = (SRPacket*) packet;
			// Check subtype
			switch(srp->srPacketType){
				case SRPacketType::CHAT:
				{
					String text = ((SRChatPacket*)srp)->GetText();
					// Extra body and push it to chat.
					ChatMan.AddMessage(new ChatMessage(packet->sender, text));
					break;
				}
				case SRPacketType::RACE:
				{

					SRRacePacket * race = (SRRacePacket*) srp;
					String msg, additionalData;
					race->Parse(msg, additionalData);
					/// Only if client, TODO: check somehow?
					// Start the race
					if (msg == "Start")
					{
						StateMan.QueueState(GAME_STATE_RACING);
					}
					// Start countdown
					else if (msg == "Countdown")
					{
						StateMan.QueueState(GAME_STATE_RACING);
					}
					// Finished, fetch result table?
					else if (msg == "Finished")
					{

					}
					/// Simulation ended, go to lobby?
					else if (msg == "Ended")
					{
						StateMan.QueueState(GAME_STATE_LOBBY);
					}
					break;
				}
				case SRPacketType::START_GAME:
					StateMan.QueueState(GAME_STATE_RACING);
					break;
				case SRPacketType::GO_TO_LOBBY:
					StateMan.QueueState(GAME_STATE_LOBBY);
					break;
			}
		}
		/// All default packets should be handled internally by the NetworkManager.
		/// Any actions here should be entirely optional or game-specific.
		default:
		{
			// We don't handle these kind of packets :O
			break;
		}
	}
}

/// Hosts game, using provided variables in the state
bool GlobalState::HostGame(){
	std::cout<<"\nMainMenu::HostGame: ";
	bool result = NetworkMan.StartSIPServer();
	if (!result){
		ChatMan.AddMessage(new ChatMessage(NULL, "Unable to start SIP server."));
		return false;
	}
	assert(result);
	/// Start a new SpaceRace-session!
	SRSession * srs = GetSession();
	result = srs->Host(33001, 33002);
	assert(result);
    if (result){
        StateMan.QueueState(GAME_STATE_LOBBY);
        ChatMan.AddMessage(new ChatMessage(NULL, "Game hosted."));
    }
    else {
        String errorString = NetworkMan.GetLastErrorString();
        String resultString = "Unable to start server, reason: " + errorString;
        ChatMan.AddMessage(new ChatMessage(NULL, resultString));
		return false;
    }
	return true;
}

/// Joins game, checking UI for destination and stuff
void GlobalState::JoinGame(){
	if (!StateMan.ActiveState())
		return;
	UserInterface * ui = StateMan.ActiveState()->GetUI();
	if (!ui)
		return;
	String ip;
	UIElement * e = ui->GetElementByName("IPInput");

	/// Fetch ip from element if we find any.
	if(e){
		ip  = e->text;
		NetworkMan.targetIP = ip;
	}
	/// If a target IP has been set via command-line, try and use it.
	if (e == NULL || ip.Length() < 4){
		ip = "127.0.0.1";
	}
	bool result = NetworkMan.ConnectTo(ip, 33000);
	if (result){
		/// Subscribe to all games.
		ChatMan.AddMessage(new ChatMessage(NULL, "Connection successful."));
    }
    else {
        String errorString = NetworkMan.GetLastErrorString();
        String resultString = "Unable to connect, reason: " + errorString;
        ChatMan.AddMessage(new ChatMessage(NULL, resultString));
    }
    Graphics.QueueMessage(new GMScrollUI("NetworkLog", -100.0f));
}

void GlobalState::ProcessMessage( Message * message ){
    switch(message->type)
    {
		case MessageType::FLOAT_MESSAGE:
		{
			String msg = message->msg;
			FloatMessage * fm = (FloatMessage*) message;
			if (msg == "SetMasterVolume"){
				float value = fm->value;
				AudioMan.SetMasterVolume(value);
			}
			else if (msg == "SetLaps")
			{
				GetSession()->SetLaps(fm->value);
			}
			break;
		}
		case MessageType::STRING: {
			String s = message->msg;
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (s == "editor"){
				StateMan.QueueState(GAME_STATE_EDITOR);
				return;
			}
			else if (s == "OnRaceOptionsUpdated")
			{
				/// Update the gui with current options.
				Graphics.QueueMessage(new GMSetUIf("Laps", GMUI::FLOAT_INPUT, GetSession()->Laps()));

			}
			else if (s == "ShowPeerUdpStatus")
			{
				/// Check for ui in current userinterface.
				UserInterface * ui = Graphics.GlobalUI();
				if (!ui)
					return;
				String elementName = "PeerUdpStatus";
				UIElement * element = ui->GetElementByName(elementName);
				/// Add it if not there.
				if (!element){
					element = UserInterface::LoadUIAsElement("gui/PeerUdpStatus.gui");
					if (!element)
						return;
					Graphics.QueueMessage(new GMAddUI(element));
				}
				/// Update the list
				UpdatePeerUdpStatusUI();

				/// Push it to stack if not.
				Graphics.QueueMessage(new GMPushUI(elementName));
				/// Make sure it is exitable.
				element->exitable = true;
			}
			else if (s == "HidePeerUdpStatus"){
				String elementName = "PeerUdpStatus";
				/// Send a pop ui message. No harm will come if it isn't available!
				Graphics.QueueMessage(new GMPopUI(elementName));
			}
			else if (s.Contains("PostChatMessage(")){
				String uiName = s.Tokenize("()")[1];
				UIElement * e;
				if (uiName == "this")
					e = message->element;
				/// Post a message to the chat-manager!
				String text = e->text;
				GetSession()->PostChatMessage(text);
			}
			else if (s.Contains("Udp tested and working ")){
				ChatMan.PostGeneral(s);
			}
			else if (s == "NewLocalGame" || s == "New game")
			{
				GetSession()->HostLocalGame();
				StateMan.QueueState(GAME_STATE_LOBBY);
				return;
			}
			else if (s == "go_to_main_menu")
				StateMan.QueueState(GAME_STATE_MAIN_MENU);
			else if (s == "ToggleRenderEntityLookAtVectors"){
				Graphics.renderLookAtVectors = !Graphics.renderLookAtVectors;
			}
			else if (s == "TogglePhysicsPaused"){
				if (Physics.IsPaused())
					Physics.Resume();
				else
					Physics.Pause();
				return;
			}
			else if (s == "ConnectionFailed"){
				StateMan.QueueState(GAME_STATE_MAIN_MENU);
			}
			else if (s == "ToggleAudioEnabled"){
				bool enabled = AudioMan.AudioEnabled();
				if (enabled){
					TrackMan.Pause();
					AudioMan.DisableAudio();
				}
				else {
					AudioMan.EnableAudio();
					TrackMan.Resume();
				}
				OnAudioEnabledUpdated();
			}
			else if (s == "AudioEnabled")
			{
				OnAudioEnabledUpdated();
			}
			else if (s == "OnOptionsMenuOpened")
			{
				OnAudioEnabledUpdated();
				OnMasterVolumeUpdated();
			}
			else if (s == "SetSynchronizationProtocol(this)"){
				int protocol = message->element->text.ParseInt();
				/// Change sync-type for all players.
				List<SRPlayer*> players = GetPlayers();
				GetSession()->SetRequestedProtocol(protocol);
			}
			else if (s.Contains("PlayTrack(")){
				String trackName = s.Tokenize("()")[1];
				Track * t = TrackMan.GetTrack(trackName);
				if (t){
					t->Loop(true);
					TrackMan.Play(t);
				}
			}
			else if (s.Contains("SetTrackEnabled("))
			{
				String trackName = s.Tokenize("(),")[1];
				Track * t = TrackMan.GetTrack(trackName);
				if (t)
				{
					t->enabled = !message->element->toggled;
					String uiName = "SetTrackEnabled("+t->name+",!this->toggled)";
					Graphics.QueueMessage(new GMSetUIb(uiName, GMUI::TOGGLED, t->enabled));
					Graphics.QueueMessage(new GMSetUIs(uiName, GMUI::TEXT, t->enabled? "Enabled" : "Disabled"));
				}
			}
			else if (s.Contains("UpdateBGMPreferencesUIList(")){
				String targetListName = s.Tokenize("()")[1];
				Graphics.QueueMessage(new GMClearUI(targetListName));

				List<Track*> tracks = TrackMan.GetTracks();
				for (int i = 0; i < tracks.Size(); ++i)
				{
					Track * t = tracks[i];
					UIColumnList * track = new UIColumnList("TrackColumnList"+String::ToString(i));
					track->sizeRatioY = 0.1f;
					UILabel * label = new UILabel();
					label->text = t->name;
					if (label->text.Length() < 1)
						label->text = "No name";
					label->sizeRatioX = 0.3f;
					track->AddChild(label);
					UIButton * toggle = new UIButton();
					toggle->text = t->enabled ? "Enabled" : "Disabled";
					toggle->activationMessage = "SetTrackEnabled("+t->name+",!this->toggled)";
					toggle->name = toggle->activationMessage;
					toggle->type = UIType::CHECKBOX;
					toggle->toggled = t->enabled;
					toggle->sizeRatioX = 0.3f;
					track->AddChild(toggle);
					UIButton * preview = new UIButton();
					preview->text = "Preview";
					preview->activationMessage = "PlayTrack("+t->name+")";
					preview->sizeRatioX = 0.3f;
					track->AddChild(preview);
					Graphics.QueueMessage(new GMAddUI(track, targetListName));
				}
			}

			else if (s == "StopHostingGame"){
				GetSession()->Stop();
				ChatMan.AddMessage(new ChatMessage(NULL, "Stopped game server."));
			}
			else if (s == "Disconnect"){
				GetSession()->Stop();
				ChatMan.AddMessage(new ChatMessage(NULL, "Disconnected."));
			}
			else if (s == "join_game" || s == "JOIN_GAME"){
                std::cout<<"\nJoin game";
                JoinGame();
            }
            else if (s == "host_game" || s == "HOST_GAME"){
                std::cout<<"\nHost game";
                HostGame();
                return;
            }
			else if (s == "networkTest" || s == "testNetwork"){
			    std::cout<<s;
			    bool result = HostGame();
			    if (!result)
					JoinGame();
			}
			else if (s.Contains("OnPeerConnected(")){
				String name = s.Tokenize("()")[1];
				if (!NetworkMan.IsHost())
					return;
				/// Properly add this to some chat log..?
				ChatMan.AddMessage(new ChatMessage(NULL, "Peer "+name+" joined."));
			}
			else if (s.Contains("OnPeerDisconnected(")){
				String name = s.Tokenize("()")[1];
				if (!NetworkMan.IsHost())
					return;
				/// Properly add this to some chat log..?
				ChatMan.AddMessage(new ChatMessage(NULL, "Peer "+name+" disconnected."));
			}
			else if (s.Contains("JoinRequestAccepted(")){
				/// Now we can head on to the lobby or wherever they are.
				ChatMan.AddMessage(new ChatMessage(NULL, "Join request accepted."));
				/// Connect to the space race session straight away too!
				StateMan.QueueState(GAME_STATE_LOBBY);
			}
			else if (s.Contains("JoinRequestRefused(")){
				String reason = s.Tokenize(";")[1];
				ChatMan.AddMessage(new ChatMessage(NULL, "Join request refused. "+reason));
			}
			else if (s.Contains("OnAvailableGamesUpdated")){
				List<Game*> games = NetworkMan.GetAvailableGames();
				if (NetworkMan.IsHost())
					return;
				ChatMan.AddMessage(new ChatMessage(NULL, "Games available: "+String::ToString(games.Size())));
				Game * spaceRaceGame = NULL;
				for (int i = 0; i < games.Size(); ++i){
					if (games[i]->type == SPACE_RACE_GAME_NAME){
						spaceRaceGame = games[i];
						break;
					}
				}
				if (!spaceRaceGame)
				{
					ChatMan.AddMessage(new ChatMessage(NULL, String("No ")+SPACE_RACE_GAME_NAME+" games available"));
					return;
				}
				/// Start a new SpaceRace-session!
				ChatMan.PostGeneral("Attempting to connect to "+SPACE_RACE_GAME_NAME+" session...");
				/// Better use same host as used in the when connecting earlier. There is no guarantee whatsoever that the server knows its own public IP...
				String host = NetworkMan.targetIP;
				/// 33000 SIP, 33001 SR TCP, 33002 SR Host UDP, 33003 SR Client UDP
				GetSession()->Stop();
				bool result = GetSession()->ConnectTo(host, spaceRaceGame->port, 33003);
				if (result){
					ChatMan.AddMessage(new ChatMessage(NULL, String("Connected to ")+SPACE_RACE_GAME_NAME+" session."));
				}
				else {
					ChatMan.AddMessage(new ChatMessage(NULL, String("Unable to connect to ")+SPACE_RACE_GAME_NAME+" session. "));
					ChatMan.AddMessage(new ChatMessage(NULL, String("Reason: ")+GetSession()->GetLastErrorString()));
				}
			}
			break;
		}
	}
}

/*
/// Wosh.
void GlobalState::OnChatMessageReceived(ChatMessage * cm){

}
*/

void GlobalState::Process(float time){
	// Calculate time since last update
	clock_t newTime = clock();
	int timeDiff = newTime - lastTime;
	lastTime = newTime;
}

void GlobalState::CreateUserInterface(){

}

void GlobalState::MouseClick(bool down, int x, int y, UIElement * elementClicked){

}

void GlobalState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){

}

void GlobalState::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * element){

}
void GlobalState::MouseWheel(float delta){

}

void GlobalState::OnSelect(Selection &selection){

}

///
void AddLabel(String text, String toUi)
{
	UIElement * newElement = new UILabel();
	newElement->text = text;
	newElement->sizeRatioY = 0.1f;
	Graphics.QueueMessage(new GMAddUI(newElement, toUi));
}

void GlobalState::LoadPreferences()
{
	bool success = Preferences.Load();
	/// Set defaults.
	if (!success){
		AudioMan.SetMasterVolume(0.1f);
	}
	/// Load preferences data into all places needed.
	else {
		AudioMan.LoadPreferences();
	}
}
void GlobalState::SavePreferences()
{
	/// Gather preferences data.
	AudioMan.SavePreferences();
	Preferences.Save();
}

/// Updates relevant gui.
void GlobalState::OnAudioEnabledUpdated()
{
	bool enabled = AudioMan.AudioEnabled();
	Graphics.QueueMessage(new GMSetUIs("AudioEnabled", GMUI::TEXT, enabled ? "Audio Enabled" : "Audio Disabled"));
}

void GlobalState::OnMasterVolumeUpdated()
{
	Graphics.QueueMessage(new GMSetUIf("MasterVolume", GMUI::FLOAT_INPUT, AudioMan.MasterVolume()));
}



/// Update the list
void GlobalState::UpdatePeerUdpStatusUI()
{
	/// Clear the lists
	Graphics.QueueMessage(new GMClearUI("UdpPeerList"));
	Graphics.QueueMessage(new GMClearUI("UdpPeerUdpTestPacketsSent"));
	Graphics.QueueMessage(new GMClearUI("UdpPeerUdpTestPacketsReceived"));
	Graphics.QueueMessage(new GMClearUI("UdpPeerUdpWorkingPacketsSent"));
	Graphics.QueueMessage(new GMClearUI("UdpPeerUdpWorkingPacketsReceived"));
//	Graphics.QueueMessage(new GMClearUI("UdpPeerList"));
	SRSession * srs = GetSession();
	List<Peer*> peers = GetSession()->GetPeers();
	///
	for (int i = 0; i < peers.Size(); ++i){
		Peer * peer = peers[i];
		SRSessionData * srsd = srs->GetSessionData(peer);

		AddLabel(peer->name, "UdpPeerList");
		AddLabel(String::ToString(srsd->udpTestPacketsSent), "UdpPeerUdpTestPacketsSent");
		AddLabel(String::ToString(srsd->udpTestPacketsReceived), "UdpPeerUdpTestPacketsReceived");
		AddLabel(String::ToString(srsd->udpWorkingPacketsSent), "UdpPeerUdpWorkingPacketsSent");
		AddLabel(String::ToString(srsd->udpWorkingPacketsReceived), "UdpPeerUdpWorkingPacketsReceived");
	}
}
