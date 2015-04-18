// Emil Hedemalm
// 2013-06-28

#include "Game/GameType.h"

#if BUILD_TARGET  DEMO_PROJECT

#include "MainMenu.h"
// Don't include all managers. Ever.
#include "Message/Message.h"
#include "Actions.h"
#include "Graphics/Messages/GMUI.h"
#include "OS/Sleep.h"
extern UserInterface * ui[MAX_GAME_STATES];


/// For testng particles
#include "Graphics/GraphicsProperty.h"
#include "Graphics/Particles/Exhaust.h"
#include "Graphics/GraphicsManager.h"
#include "Maps/MapManager.h"
#include "TextureManager.h"
#include "StateManager.h"
#include "Input/InputManager.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Audio/AudioManager.h"
#include "Chat/ChatManager.h"
#include "Network/NetworkManager.h"

MainMenu::MainMenu(){
	id = GAME_STATE_MAIN_MENU;
	requestedPlayers = 1;
    stateName = "Space Race Main Menu";
}

MainMenu::~MainMenu()
{

}

void MainMenu::OnEnter(GameState * previousState){
	std::cout<<"\nEntering Main Menu state";
	//	SetWindowPos(hWnd, HWND_TOP, 0, 0, 800, 600, SWP_NOMOVE/*SWP_HIDEWINDOW*/);

    Graphics.QueueMessage(new GMSet(CLEAR_COLOR, Vector3f(0.1f,0.1f,0.15f)));

#ifdef USE_AUDIO
	AudioMan.Play(BGM, "2013-02-27 Blast through.ogg", true);
#endif

	// Load initial texture and set it to render over everything else
//	Graphics.SetOverlayTexture("img/loadingData.png");
//	SleepThread(100);
	Graphics.EnableAllDebugRenders(false);

	Physics.QueueMessage(new PMSet(GRAVITY, 0.f));

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSet(ACTIVE_USER_INTERFACE, ui));
    if (!MapMan.MakeActive("MainMenuMap")){
        Map * m = MapMan.CreateMap("MainMenuMap");
        MapMan.MakeActive("MainMenuMap");
    }
    MapMan.DeleteEntities();

	const int particleSystems = 4;
	Vector3f rotations[particleSystems];
	for (int i = 0; i < particleSystems; ++i){
		rotations[i].y = 2 * PI / particleSystems * i;
		Entity * e = MapMan.CreateEntity(NULL, NULL);
		//	e->rotate(rotations[i]);
		GraphicsProperty * gp = new GraphicsProperty();
		gp->particleSystems = new List<ParticleSystem*>();
		Exhaust * exhaust = new Exhaust(e);
		if (i%2 == 0)
			exhaust->pointsOnly = true;
		time_t currentTime = Timer::GetCurrentTimeMs();
#define DEFAULT_ALPHA	0.5f
		switch(currentTime%8){
			case 0:	exhaust->SetColor(Vector4f(0.1f,0.1f,0.1f,DEFAULT_ALPHA)); break;
			case 1:	exhaust->SetColor(Vector4f(0.5f,0.1f,0.1f,DEFAULT_ALPHA)); break;
			case 2: exhaust->SetColor(Vector4f(0.1f,0.5f,0.1f,DEFAULT_ALPHA)); break;
			case 3: exhaust->SetColor(Vector4f(0.1f,0.1f,0.5f,DEFAULT_ALPHA)); break;
			case 4: exhaust->SetColor(Vector4f(0.1f,0.5f,0.5f,DEFAULT_ALPHA)); break;
			case 5: exhaust->SetColor(Vector4f(0.5f,0.1f,0.5f,DEFAULT_ALPHA)); break;
			case 6: exhaust->SetColor(Vector4f(0.5f,0.5f,0.0f,DEFAULT_ALPHA)); break;
			case 7: exhaust->SetColor(Vector4f(0.2f,0.3f,0.4f,DEFAULT_ALPHA)); break;
			default: exhaust->SetColor(Vector4f(0.1f,0.1f,0.1f,DEFAULT_ALPHA));
		}
		gp->particleSystems->Add(exhaust);
		/// Give it the graphics property.
		e->graphics = gp;
#ifdef USE_QUATERNIONS
		Vector3f angAcc = Vector3f(0.001f, 0.002f, 0.0f);
#else
		Vector3f angAcc = Vector3f(0.1f,2.f,0);
#endif
		Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, e, angAcc));
		Physics.QueueMessage(new PMSetEntity(COLLISSIONS_ENABLED, e, false));
		Physics.QueueMessage(new PMSetEntity(SET_ROTATION, e, rotations[i]));
		
	}

  //  exhaust = new Exhaust(e);
  //  delete exhaust;

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	// Pre-load the loadingData texture for future use!
	TexMan.LoadTexture("loadingData.png");

	/// Update options menu labels
	Graphics.QueueMessage(new GMSetUIs("ActivePlayerButton", GMUI::TEXT, "Player 1"));
	Graphics.QueueMessage(new GMSetUIs("ActivePlayerInput", GMUI::TEXT, "Keyboard 1"));

    /// Update the messages shown in the network log upon entering the state.
	OnChatMessageReceived(NULL);
	
	/// Queue next state straight away?

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


	Physics.QueueMessage(new PMSet(GRAVITY, -250.f));
//	SleepThread(5000);
	// Verify data o-o
	MapMan.GetLighting().VerifyData();
}

#include "../UI/UserInterface.h"
#include "../Graphics/Messages/GMUI.h"

void MainMenu::Process(float time){
	SleepThread(50);
	/// Updated in the global state from now on.
	/*
#ifdef USE_AUDIO
	AudioMan.Update();
#endif
*/
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MainMenu::ProcessMessage(Message * message){
	std::cout<<"\nMainMenu::ProcessMessage called:";
	switch(message->type){
		case MessageType::STRING: {
			String s = message->msg;
			std::cout<<"\nString: "<<s;
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (s == "go_to_racing_state")
				InputProcessor(GO_TO_RACING_STATE);
			else if (s == "GO_TO_LOBBY_STATE"){
				StateMan.QueueState(GAME_STATE_LOBBY);
			}
			else if (s == "GO_TO_PHYSICS_STATE")
                StateMan.QueueState(GAME_STATE_PHYSICS_TEST);
			//	StateMan.QueueState(GAME_STATE_RACING);
			else if (s == "GO_TO_STREAMER_TEST_STATE"){
				StateMan.QueueState(GAME_STATE_STREAM_TEST);
			}
			else if (s == "go_to_main_menu")
				StateMan.QueueState(GAME_STATE_MAIN_MENU);
			else if (s == "go_to_editor")
				StateMan.QueueState(GAME_STATE_EDITOR);
			else if (s == "go_to_options")
				Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
			else if (s == "go_to_blueprint_editor"){
				StateMan.QueueState(GAME_STATE_BLUEPRINT_EDITOR);
			}
			else if (s == "exit")
				StateMan.QueueState(GAME_STATE_EXITING);
			/*
#ifdef USE_NETWORK
			else if (s == "StopHostingGame"){
			    Network.Shutdown();
                ChatMan.AddMessage(new ChatMessage(NULL, "Stopped game server."));
			}
			else if (s == "Disconnect"){
                Network.Shutdown();
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
#endif
			*/
			else {
				std::cout<<"\nUndefined message received: "<<message->msg;
			}
			break;
		}
	}
}



#ifdef USE_NETWORK

/// Hosts game, using provided variables in the state
void MainMenu::HostGame(){
    std::cout<<"\nMainMenu::HostGame: ";
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

/// Joins game, checking UI for destination and stuff
void MainMenu::JoinGame(){
	if (!StateMan.ActiveState())
		return;
	UserInterface * ui = StateMan.ActiveState()->GetUI();
	if (!ui)
		return;
	String ip;
	UIElement * e = ui->GetElementByName("IPInput");
	if(e)
		ip  = e->text;
    else if (e == NULL || ip.Length() < 4)
        ip = "127.0.0.1";
	bool result = Network.JoinServer(ip);
	if (result){
        StateMan.QueueState(GAME_STATE_LOBBY);
        ChatMan.AddMessage(new ChatMessage(NULL, "Game joined."));
    }
    else {
        String errorString = Network.GetLastErrorString();
        String resultString = "Unable to join server, reason: " + errorString;
        ChatMan.AddMessage(new ChatMessage(NULL, resultString));
    }
    Graphics.QueueMessage(new GMScrollUI("NetworkLog", -100.0f));
}
#endif

/// Woshi.
void MainMenu::OnChatMessageReceived(ChatMessage * chatMessage){
    if (chatMessage == NULL || chatMessage->type != ChatMessage::GLOBAL_ANNOUNCEMENT)
        return;

    std::cout<<"\nMainMenu::OnChatMessageReceived";
    Graphics.QueueMessage(new GMClearUI("NetworkLog"));
    std::cout<<"\nClearing network log";
    // Do stuff.
    List<ChatMessage*> chatMessages = ChatMan.GetMessages(ChatMessage::GLOBAL_ANNOUNCEMENT);
    ChatMessage * cm;
    for (int i = 0; i < chatMessages.Size(); ++i){
        cm = chatMessages[i];
        std::cout<<"\nChat message "<<i;

        UILabel * l = new UILabel();
        std::cout<<"\nCreating new label..";
        switch(cm->type){
            case ChatMessage::GLOBAL_ANNOUNCEMENT:
                std::cout<<"\nGlobal Ann..";
                l->text = "";
                break;
            default:
                l->text = cm->playerName +": ";
        }
        l->text += cm->text;
        l->sizeRatioY = 0.15f;
        Graphics.QueueMessage(new GMAddUI(l, "NetworkLog"));
    }
    /// Scroll dauown (100 pages) to it!
    Graphics.QueueMessage(new GMScrollUI("NetworkLog", -100.0f));
}

#endif

