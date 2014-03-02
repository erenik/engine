/// Emil Hedemalm
/// 2013-10-20

#define print(arg) std::cout << arg << "\n";

#include "GlobalState.h"

#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"

//#include "../Managers.h"
#include "UI/UserInterface.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/Message.h"
#include <iomanip>
#include "OS/Sleep.h"
#include "Player/PlayerManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Audio/AudioManager.h"

extern UserInterface * ui[MAX_GAME_STATES];
#define UI ui[StateMan.currentGameState];

DemoProjectGlobalState::DemoProjectGlobalState()
: GameState(){
    id = GAME_STATE_GLOBAL;
    stateName = "Space Race Global State";
}


DemoProjectGlobalState::~DemoProjectGlobalState()
{

}


void DemoProjectGlobalState::OnEnter(GameState * previousState){
	// Defaultur.
	Physics.QueueMessage(new PMSet(GRAVITY, -9.82f));

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));
}

void DemoProjectGlobalState::OnExit(GameState * nextState){
//	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	std::cout<<"\nLeaving DemoProjectGlobalState state.";

	// Load initial texture and set it to render over everything else
	if (nextState && nextState->GetID() != GAME_STATE_EXITING)
		Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/loadingData.png"));
	else
		Graphics.QueueMessage(new GMSets(OVERLAY_TEXTURE, "img/deallocating.png"));
}

void DemoProjectGlobalState::ProcessPacket( Packet * packet ){
#ifdef USE_NETWORK
	switch(packet->packetType)
	{
	   /* Network.QueuePacket(new Packet(), PACKET_TARGET_ALL);
	    case
	    */
	/*    case SRP_START_GAME:{
	        StateMan.QueueState(GAME_STATE_RACING);
	    }
	*/
		case PACKET_PING:
		{
			Network.QueuePacket(new Packet(PACKET_PINGREPLY, packet->sender));
			break;
		}
		case PACKET_PINGREPLY:
		{
			Network.GetClient(packet->sender)->pingRetries = PING_RESENDS;
			break;
		}
		case PACKET_CHAT:
		{
			PacketChat *pkt = (PacketChat *)packet;
			time_t timeCreated = (pkt->timeCreated)/1000;
			struct tm *tim = std::localtime (&timeCreated);
			print(Network.GetClient(pkt->sender)->name << "[" << ((tim->tm_hour<10)?"0":"") << tim->tm_hour << ":" << ((tim->tm_min<10)?"0":"") << tim->tm_min << ":" << ((tim->tm_sec<10)?"0":"") << tim->tm_sec << "]: " << pkt->message);
			String message = pkt->message;
            NetworkClient * client = Network.GetClient(pkt->sender);
			Player * player = PlayerMan.GetPlayerByClientIndex(client->index);
            ChatMan.AddMessage(new ChatMessage(player, message, ChatMessage::PUBLIC));
			break;
		}
		case PACKET_VERSIONCHECK:
		{
			// Host valifies a clients version
			Network.MakeVersionCheck((PacketVersionCheck *)packet);
			break;
		}
		case PACKET_ESABLISHCONNECTION:
		{
			// A client's version is correct, establish connection
			Network.EstablishConnection((PacketEstablishConnection *) packet);
			break;
		}
		case PACKET_UPDATEMYCLIENTINFO:
		{
			// The server receives a clients new/updated information, broadcasts it
			Network.ApplyClientInfo((PacketUpdateMyClientInfo *) packet);
			break;
		}
		case PACKET_BROADCASTCLIENTSINFO:
		{
			// A client receives updated information on the network users
			Network.UpdateClientsInfo((PacketBroadcastClientsInfo *) packet);
			break;
		}
		case PACKET_MAKEFTPREQUEST:
		{
			// Checks what the FTP request is containing
			Ftp.ReceiveFtpRequest((PacketMakeFtpRequest *) packet);
			break;
		}
		case PACKET_ANSWERFTPREQUEST:
		{
			// Check if client accepted or declined FTP request
			Ftp.CheckFtpRequest((PacketAnswerFtpRequest *) packet);
			break;
		}
		case PACKET_FTPDATA:
		{
			// Receive a part of data from a file in the FTP request
			Ftp.ReceiveFtpData((PacketFtpData *) packet);
			break;
		}
		case PACKET_FTPDATA_REPLY:
		{
			// Get answer that target of FTP request received a data part
			Ftp.ReceiveFtpDataPartConfirmation((PacketFtpDataReply *) packet);
			break;
		}
		case PACKET_FTP_FINISHED:
		{
			// Sender of FTP request says that he have received confirmation on all data parts
			Ftp.FinishFtpRequest((PacketFtpFinished *) packet);
			break;
		}
		default:
		{
			// We don't handle these kind of packets :O
			break;
		}
	}
#endif
}

void DemoProjectGlobalState::ProcessMessage( Message * message ){
	String s = message->msg;
	s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (s == "go_to_main_menu")
		StateMan.QueueState(GAME_STATE_MAIN_MENU);
	else if (s == "ToggleRenderEntityLookAtVectors"){
		Graphics.renderLookAtVectors = !Graphics.renderLookAtVectors;
	}
	/*
#ifdef USE_NETWORK
	if (s.Contains("join_game")){
		JoinGame();
	}
	else if (s.Contains("host_game")){
	    std::cout<<"\nSpaceRace::ProcessMessage: "<<s;
		bool result = Network.StartServer();
		if (result){
		    StateMan.QueueState(GAME_STATE_LOBBY);
		    ChatMan.AddMessage(new ChatMessage(NULL, "Game hosted."));
        }
        else {
            String errorString = Network.GetLastErrorString();
            String resultString = "Unable to start server, reason: " + errorString;
            ChatMan.AddMessage(new ChatMessage(NULL, resultString));
        }
	}
#endif
*/
}

/*
/// Wosh.
void DemoProjectGlobalState::OnChatMessageReceived(ChatMessage * cm){

}
*/

void DemoProjectGlobalState::Process(float time){
	Sleep(10);
// std::cout<<"\nGLOBAL STATE.";
	// Calculate time since last update
	clock_t newTime = clock();
	int timeDiff = newTime - lastTime;
	lastTime = newTime;
#ifdef USE_AUDIO
	/// Update audio no matter current state.
	AudioMan.Update();
#endif
}

void DemoProjectGlobalState::CreateUserInterface(){

}

void DemoProjectGlobalState::MouseClick(bool down, int x, int y, UIElement * elementClicked){

}

void DemoProjectGlobalState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){

}

void DemoProjectGlobalState::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){

}
void DemoProjectGlobalState::MouseWheel(float delta){

}

void DemoProjectGlobalState::OnSelect(Selection &selection){

}

