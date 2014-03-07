// Emil Hedemalm
// 2013-06-28

#include "Game/GameVariables.h"
#include "Graphics/GraphicsManager.h"
#include "Maps/MapManager.h"
#include "Physics/PhysicsManager.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Network/NetworkManager.h"
#include "Audio/TrackManager.h"

#include "RacingState.h"

#include "Actions.h"
#include "Message/Message.h"
#include "../UI/UserInterface.h"
#include "../Graphics/Messages/GMUI.h"
#include "../Physics/PhysicsProperty.h"
#include "Graphics/Render/RenderViewport.h"
#include "Physics/Messages/CollissionCallback.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "EntityStates/StateProperty.h"
#include "../../EntityStates/RacingShipGlobal.h"
#include "Graphics/Messages/GMParticleMessages.h"
#include "OS/Sleep.h"
#include <Util.h>
#include "Event/TextAnimationEvent.h"
#include "Event/EventManager.h"
#include "String/StringUtil.h"

#include <iomanip>
#include "Player/Player.h"
#include "Player/PlayerManager.h"
#include "../../SRPlayer.h"
#include "../../Ship.h"
#include "../../AI/AIRacer.h"
#include "Graphics/Messages/GMSet.h"
#include "../../SRConstants.h"
#include "UI/UIList.h"

#include "../../SRPlayer.h"

#include "Network/Packet/Packet.h"
#include "Network/Session/GameSession.h"
#include "Network/Packet/SyncPacket.h"

#include "../../Network/SRPacketTypes.h"
#include "../../Network/SRPacket.h"

#include "../../ShipManager.h"
#include "Chat/ChatManager.h"


Racing * Racing::racing = NULL;

Racing::Racing(){
    stateName = "Racing state";
	id = GAME_STATE_RACING;
	requestedPlayers = 1;
	assert(racing == NULL);
	racing = this;
	timerStarted = false;
	gameSession = NULL;

	/// Debugging variables
	stateUpdatesSent = 0;
	hostUpdateDelay = 0;

	/// Client
	estimationMode = 2;
	estimationDelay = 0;
	smoothingDuration = 100;
}

Racing::~Racing()
{
	assert(racing);
	racing = NULL;
	cameras.ClearAndDelete();
}

/// Creates the user interface for this state
void Racing::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/RacingMenu.gui");
}

void Racing::OnEnter(GameState * previousState){
    std::cout<<"\nRacing::OnEnter";
  //  assert(previousState != this);

	Graphics.ResetSleepTimes();

	/// Debug stuffs
	lastStateGUIUpdated = Timer::GetCurrentTimeMs();
	lastUpdateToClients = Timer::GetCurrentTimeMs();

    // Disable UI-navigation so that we can use all of the keyboard for player-movement!
    Input.ForceNavigateUI(false);
	Input.NavigateUI(false);

	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTextureBySource("img/loadingData.png")));
    Graphics.QueueMessage(new GMSet(CLEAR_COLOR, Vector3f(0.4f,0.4f,0.5f)));

	Physics.QueueMessage(new PMSetSpeed(1.0f));

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));
	// Set menu to not visible, as it may remain after state-change!
	Graphics.QueueMessage(new GMPopUI("MainMenu"));
	Graphics.QueueMessage(new GMSetUIb("Results", GMUI::VISIBILITY, false));

	// Enable/disable some ui depending on host-type.
	#define	DisableButton(b) Graphics.QueueMessage(new GMSetUIv4f(b, GMUI::TEXT_COLOR, Vector4f(0.5f,0.5f,0.5f,1))); \
		Graphics.QueueMessage(new GMSetUIb(b, GMUI::HOVERABLE, false));
	#define	EnableButton(b) 	 Graphics.QueueMessage(new GMSetUIv4f(b, GMUI::TEXT_COLOR, Vector4f(1,1,1,1))); \
		Graphics.QueueMessage(new GMSetUIb(b, GMUI::HOVERABLE, true));

	/// Client-specific
	List<String> clientSpecificElements;
	clientSpecificElements += "SynchronizationProtocol";
	clientSpecificElements += "EstimationMode";
	clientSpecificElements += "EstimationTime";
	clientSpecificElements += "SmoothingDuration";

	Graphics.QueueMessage(new GMSetUIs("SynchronizationProtocol", GMUI::TEXT, String::ToString(GetSession()->RequestedProtocol())));
	Graphics.QueueMessage(new GMSetUIs("EstimationMode", GMUI::TEXT, String::ToString(estimationMode)));
	Graphics.QueueMessage(new GMSetUIs("EstimationTime", GMUI::TEXT, String::ToString(estimationDelay)));
	Graphics.QueueMessage(new GMSetUIs("SmoothingDuration", GMUI::TEXT, String::ToString(smoothingDuration)));

	bool isHost = GetSession()->IsHost();
	for (int i = 0; i < clientSpecificElements.Size(); ++i){
		String element = clientSpecificElements[i];
		if (isHost){
			DisableButton(element);
		}
		else {
			EnableButton(element);
		}
	}
	// Host-specific options
	List<String> hostSpecificElements;
	hostSpecificElements += "HostUpdateDelay";
	for (int i = 0; i < hostSpecificElements.Size(); ++i){
		String element = hostSpecificElements[i];
		if (isHost){
			EnableButton(element);
		}
		else {
			DisableButton(element);
		}
	}
	/// List


	// Clear any existing players.
	std::cout<<"\nClearing existing players..";
	List<Entity*> entities = MapMan.GetEntities();
	for (int i = 0; i < entities.Size(); ++i){
		Entity * e = entities[i];
		if (e->name.Contains("Player"))
			MapMan.DeleteEntity(e);
	}

	// Get previous state ID for when we return later on.
	if (previousState){
        if (previousState != this)
            previousStateID = previousState->GetID();
	}
	else
		previousStateID = 0;


	// Verify data o-o
	MapMan.GetLighting().VerifyData();
	// Resume le game!
	menuOpen = false;
	timerStarted = false;
	StateMan.Resume();
	Physics.Pause();

	/// Toggle debug renders
#ifdef _DEBUG
	Graphics.EnableAllDebugRenders();
	Graphics.renderAI = false;
#else
	Graphics.EnableAllDebugRenders(false);
#endif

	/// Set editor selection as the renderable one!
	Graphics.selectionToRender = &editorSelection;

	// And set it as active
//	Graphics.cameraToTrack = &cameras[0];
//	Graphics.cameraToTrack->SetRatio(Graphics.width, Graphics.height);
	Graphics.UpdateProjection();
	Graphics.EnableAllDebugRenders(false);
	Graphics.renderFPS = true;

	/// Set up viewports for our players
	viewports.Clear();
	viewports;
	RenderViewport * vp;

	/// Fetch session
	SRSession * srs = GetSession();

	/// Get local players for screen setup.
	List<SRPlayer*> localPlayers = GetLocalPlayers();

    std::cout<<"\nSetting up viewports..";
	/// Six-screens... lol?
	if (localPlayers.Size() > 4){
		int rows = localPlayers.Size() / 2;
		int columns = 2;
		for (int i = 0; i < 6; ++i){
			vp = new RenderViewport("Racing.gui");
			float width = 1.0f / columns;
			float height = 1.0f / rows;
			float left = width * (i%2);
			vp->SetRelative(left, 1.0f - height * (1 + i / columns), width, height);
			viewports.Add(vp);
		}
	}
	/// Four-screens
	else if (localPlayers.Size() > 2){
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0, 0.5f, 0.5f, 0.5f);
		viewports.Add(vp);
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0.5, 0.5f, 0.5f, 0.5f);
		viewports.Add(vp);
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0.0, 0.0f, 0.5f, 0.5f);
		viewports.Add(vp);
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0.5, 0.0f, 0.5f, 0.5f);
		viewports.Add(vp);
	}
	/// Two screens
	else if (localPlayers.Size() == 2){
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0, 0.5f, 1.0f, 0.5f);
		viewports.Add(vp);
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0, 0.0f, 1.0f, 0.5f);
		viewports.Add(vp);
	}
	/// Single-player
	else {
		vp = new RenderViewport("Racing.gui");
		vp->SetRelative(0, 0, 1.0f, 1.0f);
		viewports.Add(vp);
	}
	Graphics.QueueMessage(new GMSetViewports(viewports));

    /// Create cameras, one per viewport.
    cameras.ClearAndDelete();
    for (int i = 0; i < viewports.Size(); ++i){
        Camera * c = new Camera();
    	c->name = "RacingCamera " + String::ToString(i+1);
		/// For dat racing feeling.
		c->scaleDistanceWithVelocity = true;
        cameras.Add(c);
        /// Set camera to track for the selected viewport.
        viewports[i]->SetCameraToTrack(c);
    }

    std::cout<<"\nLoading map..";
	/// Fetch map to load from session.
	String mapToLoad = srs->GetMap();
    Map * map;
	map = MapMan.LoadMap(mapToLoad);

	if (!map){
		ChatMan.PostGeneral("Unable to load target map: "+mapToLoad);
		StateMan.QueuePreviousState();
		return;
	}
	/// Make it active too.
	bool result = MapMan.MakeActive(map);

	/// Find out how many checkpoints we've got!
	std::cout<<"\nGathering checkpoints..";
	List<Entity*> mapEntities = MapMan.GetEntities();
	checkpoints.Clear();
	for (int i = 0; i < mapEntities.Size(); ++i){
		Entity * e = mapEntities[i];
		String name = e->name;
		name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (name.Contains("CheckPoint"))
			checkpoints.Add(e);
	}
	/// Save amount of checkpoints in the session.
	srs->SetCheckpoints(checkpoints.Size());

	/// Create players
	List<SRPlayer*> players = srs->GetPlayers();
	CreatePlayers(players);

	/// Assign viewports to all local players.
	std::cout<<"\nAssigning viewports to local players..";
	bool assignViewports = true;
	if (assignViewports){
        for (int i = 0; i < localPlayers.Size(); ++i){
            assert(i < viewports.Size());
            SRPlayer * player = (SRPlayer*)localPlayers[i];
            player->viewport = viewports[i];
			/// Update UI too while at it.
			OnPlayerCheckpointsPassedUpdated(player);
        }
    }

    std::cout<<"\nConfiguring viewport cameras to local players..";
    for (int i = 0; i < localPlayers.Size(); ++i){
        assert(i < cameras.Size());
        SRPlayer * player = (SRPlayer*)localPlayers[i];
        Entity * entity = player->entity;
        cameras[i]->entityToTrack = entity;
		cameras[i]->elevation = -entity->radius;	
    }
	/// If no local player, attach a camera to the first entity there is.
	if (localPlayers.Size() == 0 && players.Size())
	{
		Camera * camera = new Camera();
		SRPlayer * player = players[0];
		Entity * entity = player->entity;
		camera->entityToTrack = entity;
		camera->elevation = -entity->radius;
		cameras.Add(camera);
		/// Set camera to track.
		viewports[0]->SetCameraToTrack(camera);
	}

	/// Update player positions and laps now after most have been assigned a viewport.
	std::cout<<"\nUpdating initial player statistics..";
	for (int i = 0; i < players.Size(); ++i){
		Player * p = players[i];
		SRPlayer * sr = (SRPlayer*) p;
		OnPlayerPositionUpdated(sr);
		OnPlayerLapsUpdated(sr);
	}

    std::cout<<"\nSetting up physics environment..";
	Physics.QueueMessage(new PMSetPhysicsType(playerEntities, PhysicsType::DYNAMIC));
	Physics.QueueMessage(new PMSetEntity(RESTITUTION, playerEntities, 0.15f));
	Physics.QueueMessage(new PMSetEntity(FRICTION, playerEntities, 0.01f));

	/// Reset physics settings
//	Physics.QueueMessage(new PhysicsMessage(PM_RESET_SETTINGS));
	Physics.Resume();
//	Physics.Pause();
    std::cout<<"\nResuming physics..";

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	// And start tha music..
//	TrackMan.PlayTrackFromCategory("Race", true);
}

void Racing::OnExit(GameState *nextState){
	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTextureBySource("img/loadingData.png")));
	/// Set empty list of viewports = keep only global default one.
	Graphics.QueueMessage(new GMSetViewports(List<RenderViewport*>()));
	Physics.Pause();

	// Delete all player-entities.
	for (int i = 0; i < playerEntities.Size(); ++i)
		MapMan.DeleteEntity(playerEntities[i]);
	playerEntities.Clear();

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	Graphics.cameraToTrack->entityToTrack = NULL;

	std::cout<<"\nLeaving Racing state.";

	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.SetOverlayTexture(NULL);

}



#include <ctime>

void Racing::Process(float time){
	/// Process key input for navigating the 3D - Space
	SRSession * srs = GetSession();

	/// Update network debug data
	int bss = srs->BytesSentPerSecond();
	int brs = srs->BytesReceivedPerSecond();
//	std::cout<<"\nBytes per second: "<<bss<<" "<<brs;
	Graphics.QueueMessage(new GMSetUIs("bytesSent", GMUI::TEXT, String::ToString(bss)+" bytes/s"));
	Graphics.QueueMessage(new GMSetUIs("bytesReceived", GMUI::TEXT, String::ToString(brs)+" bytes/s"));

	long long cTime = Timer::GetCurrentTimeMs();

	// Update game state to clients!
	/// .. 1 second updates...!
	if (srs->IsHost() && cTime > lastUpdateToClients /*+ hostUpdateDelay*/){
		lastUpdateToClients = cTime;
		/// Send update every... X seconds.
		stateUpdatesSent++;
		for (int i = 0; i < playerEntities.Size(); ++i){
			Entity * e = playerEntities[i];
			/// IDs are the same as their indices, so go!
			Vector3f rotation = e->rotationVector;
			RacingShipGlobal * rsg = (RacingShipGlobal *)e->state->GlobalState();
			assert(rsg);
		//	std::cout<<"\nRotation: "<<rotation;
			SRPlayerPositionPacket ppPacket(i, e->positionVector, e->Velocity(), rotation, rsg->GetStateAsString(true), Timer::GetCurrentTimeMs() + hostUpdateDelay);
			srs->Send(&ppPacket);
			/*SyncPacket syncPacket;
			syncPacket.AddEntity(e);
			syncPacket.UpdateData();
			srs->Send(&syncPacket);
			*/
		//	NetworkMan.QueuePacket(positionPacket, PACKET_TARGET_OTHERS);
		}
	}
	/// If not host, post input for all our players to the server.
	else if (!srs->IsHost()){
		stateUpdatesSent++;
		/// Send update packets for all our local players
		List<SRPlayer *> players = GetPlayers();
		for (int i = 0; i < players.Size(); ++i)
		{
			SRPlayer * p = players[i];
			if (!p->isLocal)
				continue;
			if (!p->entity)
				continue;
			RacingShipGlobal * rsg = (RacingShipGlobal*)p->entity->state->GlobalState();
			SRPlayerMovePacket movePack(i);
			movePack.SetMessage(rsg->GetStateAsString(false));
			srs->Send(&movePack);
		}
	}
	if (cTime > lastStateGUIUpdated + 1000){
		Graphics.QueueMessage(new GMSetUIs("statesSent", GMUI::TEXT, String::ToString(stateUpdatesSent)+"/s"));
		stateUpdatesSent = 0;
		lastStateGUIUpdated = cTime;
	}


	// Don't update UI if menu is open!
	if (menuOpen)
		return;

	/// Lap starto timeu.
	if (!timerStarted){
		int currTime = (int)Timer::GetCurrentTimeMs();
		if (GameVars.Get(LAP_START_TIME_STR) == 0){
			GameVars.CreateInt(LAP_START_TIME_STR, currTime);
		}
		else {
			GameVars.SetInt(LAP_START_TIME_STR, currTime);
		}
		List<SRPlayer*> players = GetPlayers();
		for (int i = 0; i < players.Size(); ++i){
			SRPlayer * player = (SRPlayer*) players[i];
			player->lastLapStart = currTime;
		}
		timerStarted = true;
	}

	/// Last update time in ms
	static clock_t lastTime = 0;
	// Calculate time since last update
	clock_t newTime = clock();
	int timeDiff = newTime - lastTime;
	lastTime = newTime;

    if (playerEntities.Size() == 0){
        return;
    }
	// Update UI o-o
	List<SRPlayer*> localPlayers = GetLocalPlayers();
	for (int i = 0; i < localPlayers.Size(); ++i){
		SRPlayer * srp = (SRPlayer*)localPlayers[i];
		Entity * playerEntity = srp->entity;
		if (!playerEntity)
			continue;
		if (playerEntity->physics == NULL)
			continue;
		float velF = playerEntity->physics->velocity.Length() * 3.6f;
	//	velF *= 0.05f;
		int velocity = (int)floor(velF+0.5f);
		String currentVelocityString = String::ToString(velocity) + " km/h";
		GMSetUIs * setTextMessage = new GMSetUIs("CurrentVelocity", GMUI::TEXT, currentVelocityString, srp->viewport->ID());
		Graphics.QueueMessage(setTextMessage);

		RacingShipGlobal * rsg = ((RacingShipGlobal*)srp->entity->state->GlobalState());
		float remainingBoost = rsg->RemainingBoost();
		String curretBoostString = "Boost: " + String::ToString(remainingBoost, 3);
		setTextMessage = new GMSetUIs("CurrentBoost", GMUI::TEXT, curretBoostString, srp->viewport->ID());
		Graphics.QueueMessage(setTextMessage);
	}

	/// Update time ui
	Graphics.QueueMessage(new GMSetUIs("serverTime", GMUI::TEXT, String::ToString((int)Timer::GetCurrentTimeMs()/1000)));
	Graphics.QueueMessage(new GMSetUIs("localTime", GMUI::TEXT, String::ToString((int)Timer::GetCurrentTimeMs(true)/1000)));
	Graphics.QueueMessage(new GMSetUIs("timeAdjustment", GMUI::TEXT, String::ToString((int)Timer::GetAdjustment()) + " ms"));

/*	/// Fly! :D
	/// Rotate first, yo o.O
	/// Rotation multiplier.
	float rotMultiplier = 0.05f;
	mainCamera->rotation += mainCamera->rotationVelocity * (float) (mainCamera->rotationSpeedMultiplier * timeDiff);
	// Check input for moving camera
	if (mainCamera->velocity.Length() > 0){
		Vector4d moveVec;
		moveVec = Vector4d(mainCamera->velocity);
		/// Flight-speed multiplier.
		float multiplier = 0.5f * mainCamera->flySpeedMultiplier;
		moveVec = moveVec * multiplier * (float)timeDiff;
		Matrix4d rotationMatrix;
		rotationMatrix.InitRotationMatrixY(-mainCamera->rotation.y);
		rotationMatrix.multiply(Matrix4d::GetRotationMatrixX(-mainCamera->rotation.x));
		moveVec = rotationMatrix.product(moveVec);
		mainCamera->position += Vector3f(moveVec);
	}
	*/
};

enum mouseCameraStates {
	NULL_STATE,
	ROTATING,
	PANNING
};


/// Input functions for the various states
void Racing::MouseClick(bool down, int x, int y, UIElement * elementClicked){

}
void Racing::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){

}

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void Racing::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){
}

void Racing::MouseWheel(float delta){
	Camera * camera = Graphics.cameraToTrack;
	camera->distanceFromCentreOfMovement += delta / 100.0f;
	if (delta > 0){
		camera->distanceFromCentreOfMovement *= 0.8f;
	}
	else if (delta < 0){
		camera->distanceFromCentreOfMovement *= 1.25f;
	}
	if (camera->distanceFromCentreOfMovement > 0)
		camera->distanceFromCentreOfMovement = 0;
}


void Racing::ProcessMessage(Message * message){

//	std::cout<<"\nRacing::ProcessMessage: ";
	switch(message->type){
		case MessageType::COLLISSION_CALLBACK: {

			CollissionCallback * c = (CollissionCallback*)message;

			// Assume one of the colliders is a player, namely "one".
			Entity * playerEntity = NULL;
			int index = -1;
			for (int i = 0; i < playerEntities.Size(); ++i){
				if (playerEntities[i] == c->one){
					playerEntity = c->one;
					index = i;
					break;
				}
			}
			if (playerEntity == NULL)
				return;
			assert(playerEntity);
			/// Get player associated to this entity.
			/// Just update the gui for now..
			if (c->two->name.Contains("CheckPoint")){
				assert(playerEntity->player);
				SRPlayer * player = (SRPlayer*)playerEntity->player;
				Entity * checkpoint = c->two;
				List<String> tokens = checkpoint->name.Tokenize("CheckPoint");
				int checkpointNumber = tokens[0].ParseInt();
				bool validCheckpoint = false;
				/// Check if it's the next checkpoint they're supposed to pass.
				if (player->checkpointsPassed == checkpointNumber - 1)
					PlayerPassCheckpoint(player);
			}

		//	std::cout<<"\nCollissionCallback received for entity "<<c->one->name<<" and "<<c->two->name;
			if (c->one->state)
				c->one->state->ProcessMessage(message);
	//		if (c->one->scaleVector.MaxPart() < 5.0f)
	//			;//Physics.QueueMessage(new PMSetEntity(SET_SCALE, c->one, c->one->scaleVector * 1.01f));
			// Let all fat collissions generate sparks, alright?
			if (c->impactVelocity > 5.0f){
				Graphics.QueueMessage(new GMGenerateParticles("CollissionSparks", c));
			}
			return;
			break;
		}
		case MessageType::STRING: {
			String string = message->msg;
			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "SetCameraDefault")
			{
				/// Fetch 1st player camera.
				Camera * camera = cameras[0];
				camera->scaleDistanceWithVelocity = false;
				camera->revert = false;
			}
			else if (string == "SetCameraRace")
			{
				/// Fetch 1st player camera.
				Camera * camera = cameras[0];
				camera->scaleDistanceWithVelocity = true;
				camera->revert = false;
			}
			else if (string == "SetCameraRear")
			{
				/// Fetch 1st player camera.
				Camera * camera = cameras[0];
				camera->revert = true;
			}
			else if (string == "OnHostDisconnected"){
				/// Display a message and maybe pull up a button return to the lobby or out to the network-join menu?
				SetGeneralMessage("Host disconnected.");
			}
			else if (string == "ToggleMenu"){
				ToggleMenu();
				return;
			}
			else if (string == "SetEstimationType(this)"){
				std::cout<<"cpp";
				estimationMode = message->element->text.ParseInt();;
				/// Change sync-type for all players.
				List<SRPlayer*> players = GetPlayers();
				for (int i = 0; i < players.Size(); ++i){
					SRPlayer * p = players[i];
					Physics.QueueMessage(new PMSetEntity(ESTIMATION_MODE, p->entity, estimationMode));
				}
			}
			else if (string == "SetEstimationDelay(this)"){
				estimationDelay = message->element->text.ParseInt();
				/// Change sync-type for all players.
				List<SRPlayer*> players = GetPlayers();
				for (int i = 0; i < players.Size(); ++i){
					SRPlayer * p = players[i];
					Physics.QueueMessage(new PMSetEntity(ESTIMATION_DELAY, p->entity, estimationDelay));
				}
			}
			else if (string == "SetSmoothingDuration(this)"){
				smoothingDuration = message->element->text.ParseInt();
				/// Change sync-type for all players.
				List<SRPlayer*> players = GetPlayers();
				for (int i = 0; i < players.Size(); ++i){
					SRPlayer * p = players[i];
					Physics.QueueMessage(new PMSetEntity(ESTIMATION_SMOOTHING_DURATION, p->entity, smoothingDuration));
				}
			}
			else if (string == "SetHostUpdateDelay(this)"){
				hostUpdateDelay = message->element->text.ParseInt();
				std::cout<<"\nHost update delay set to: "<<hostUpdateDelay;
			}
			else if (string == "PreviousState"){
				assert(previousStateID != 0);
				StateMan.QueueState(previousStateID);
			//	NetworkMan.QueuePacket(new Packet(SRP_GO_TO_LOBBY, PACKET_TARGET_OTHERS));
			}
			else if (string == "interpret_console_Command"){
				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
				return;
			}
			else if (string.Contains("SetLevel")){
				List<String> tokens = string.Tokenize("()");
				if (tokens.Size() >= 2){
					String requestLevel = tokens[1];
					level = requestLevel;
				}
				return;
			}
			else if (string.Contains("set players")){
				List<String> tokens = string.Tokenize(" ");
				if (tokens.Size() > 2){
					int tmp = tokens[2].ParseInt();
					if (tmp <= MAX_LOCAL_RACING_PLAYERS && tmp > 0)
						requestedPlayers = tmp;
				}
				return;
			}
			else if (string == ("MenuClosed")){
                OnCloseMenu();
			}
		}
	}
	GameState::ProcessMessage(message);
}

void Racing::ProcessPacket(Packet * packet){

    SRPlayer * player = (SRPlayer*) PlayerMan.Me();
    assert(player);
	Entity * playerEntity = NULL;
	RacingShipGlobal * rsg = NULL;
	if (player){
		playerEntity = player->entity;
		if (playerEntity)
			rsg = (RacingShipGlobal*)playerEntity->state->GlobalState();
	}
    /// Send the message straight to the player entity's global ship state!
	switch(packet->type){
		case PacketType::SR_PACKET:
		{
			SRPacket * srp = (SRPacket*)packet;
			switch(srp->srPacketType)
			{
				case SRPacketType::RACE:
				{
					SRRacePacket * rp = (SRRacePacket*)srp;
					String msg, additionalData;
					rp->Parse(msg, additionalData);
					/// checkpoint passed!
					if (msg == "CheckpointsPassed")
					{
						if (GetSession()->IsHost())
							return;
						List<String> toks = additionalData.Tokenize(";");
						int playerID = toks[0].ParseInt();
						int checkpoints = toks[1].ParseInt();
						SRPlayer * p = GetPlayer(playerID);
						/// Set it.
						p->checkpointsPassed = checkpoints;
						/// Update gui.
						OnPlayerCheckpointsPassedUpdated(p);
						std::cout<<"\nPlayer "<<playerID<<" passed checkpoint: "<<checkpoints;
					}
					/// Lap passed!
					else if (msg == "LapsPassed")
					{
						if (GetSession()->IsHost())
							return;
						List<String> toks = additionalData.Tokenize(";");
						int playerID = toks[0].ParseInt();
						int laps = toks[1].ParseInt();
						int lapTime = toks[2].ParseInt();
						SRPlayer * p = GetPlayer(playerID);
						/// Set laps finished and add the time the host sent us to the array of completed lap-times!
						p->lapsFinished = laps;
						p->lapTimes.Add(lapTime);
						p->totalTime += lapTime;
						/// Update gui.
						OnPlayerLapsUpdated(p);
						std::cout<<"\nPlayer "<<playerID<<" passed checkpoint: "<<checkpoints;			
					}
					/// All player positions
					else if (msg == "PlayerPositions")
					{
						List<String> posStrings = additionalData.Tokenize(";");
						List<SRPlayer*> players = GetSession()->GetPlayers();
						for (int i = 0; i < players.Size(); ++i)
						{
							SRPlayer * player = players[i];
							int position = posStrings[i].ParseInt();
							player->position = position;
							OnPlayerPositionUpdated(player);
						}
					}
					break;
				}
				case SRPacketType::PLAYER_POSITION:
				{
		//			std::cout<<"\nPlayerPosition packet received.";
					SRPlayerPositionPacket * ppp = (SRPlayerPositionPacket*) srp;
					int playerID;
					Vector3f playerPosition, playerVelocity, rotation;
					String state;
					long long timeCreated = ppp->timeCreated;
					ppp->Parse(playerID, playerPosition, playerVelocity, rotation, state);
					SRPlayer * player = GetPlayer(playerID);
					assert(player);
					assert(player->entity);
					/// Set new position of the player entity!
				//	Physics.QueueMessage(new PMSet(
					Physics.QueueMessage(new PMSetEntity(POSITION, player->entity, playerPosition, timeCreated));
					Physics.QueueMessage(new PMSetEntity(VELOCITY, player->entity, playerVelocity, timeCreated));
					Physics.QueueMessage(new PMSetEntity(SET_ROTATION, player->entity, rotation, timeCreated));
					/// Synchronize visual effects such as thrusting and stuff.
					RacingShipGlobal * rsg = (RacingShipGlobal*)player->entity->state->GlobalState();
					rsg->LoadStateFromString(state);
					break;
				}
				case SRPacketType::PLAYER_MOVE:
				{
					SRPlayerMovePacket * pmp = (SRPlayerMovePacket*) srp;
					int playerID;
					String message;
					bool parse = pmp->Parse(playerID, message);
					if (!parse)
						break;
					/// Fetch player with ID.
					SRPlayer * player = GetPlayer(playerID);
					Entity * playerEntity = player->entity;
					RacingShipGlobal * rsg = (RacingShipGlobal*)playerEntity->state->GlobalState();
					rsg->LoadStateFromString(message);
					break;
				}
			}
			break;
		}
	}
}

/// Creates ship-entity for target player.
Entity * Racing::CreateShipForPlayer(SRPlayer * player, Vector3f startPosition){
	SRSession * srs = GetSession();
	/// Woo.
	Model * model = NULL;
	Texture * tex = NULL;
	Entity * entity = NULL;
	std::cout<<"\nCreating ship for player "<<player->name;
	player->ResetRacingStatistics();
	Ship * ship = player->ship;
	if (ship == NULL){
		ship = ShipMan.GetShips()[0];
	}
	assert(ship != NULL);
	if (ship){
		model = ModelMan.GetModel(ship->modelSource);
		tex = TexMan.GetTextureBySource(ship->diffuseSource);
	}
    std::cout<<"\nModel: "<<(model? model->Name() : "NULL" )<<" from source: "<<ship->modelSource;
	entity = MapMan.CreateEntity(model, tex);
    ship->AssignTexturesToEntity(entity);

	/// If not host, make it static physically.
	if (!srs->IsHost()){
		Physics.QueueMessage(new PMSetPhysicsType(entity, PhysicsType::STATIC));
		Physics.QueueMessage(new PMSetEntity(SIMULATION_ENABLED, entity, false));
		Physics.QueueMessage(new PMSetEntity(ESTIMATION_ENABLED, entity, true));
		Physics.QueueMessage(new PMSetEntity(ESTIMATION_DELAY, entity, estimationDelay));
		Physics.QueueMessage(new PMSetEntity(ESTIMATION_MODE, entity, estimationMode));
		Physics.QueueMessage(new PMSetEntity(ESTIMATION_SMOOTHING_DURATION, entity, smoothingDuration));
	}

	entity->name = player->name + "'s Ship";
	// Set up physics properties of the entity depending on the ship model?
	Physics.QueueMessage(new PMSetEntity(VELOCITY_RETAINED_WHILE_TURNING, entity, ship->velocityRetainedWhileTurning));
	RacingShipGlobal * rsg = NULL;
	assert(entity != NULL);
	if (entity == NULL)
        return NULL;

	/// Move it to the starting position.
	entity->translate(startPosition);

    if (entity->state == NULL){
        entity->state = new StateProperty(entity);
		/// Create ship state machine primary state. Starting position should then be saved as well!
        rsg = new RacingShipGlobal(entity, ship);
		/// Set synchronization type for the ship state. If we're not host, it should be synchronized.
		rsg->synchronized = !srs->IsHost();
        entity->state->SetGlobalState(rsg);
		// Re-register the entity for rendering.
		Graphics.QueueMessage(new GMRegisterEntity(entity));
    }

	/// Player ready to rrrrrolll!
	playerEntities.Add(entity);
	std::cout<<"\nSetting up all necessary pointers.";
	entity->player = player;
	player->entity = entity; // Bind the entity to the player.

    // Add ai to all, but disable it by default!
    AIRacer * srai = new AIRacer(entity, player, rsg, this);
    rsg->AssignAI(srai);
    if (!player->isAI)
        srai->enabled = false;

	return entity;
}


void Racing::ToggleMenu(){
	UIElement * e = ui->GetElementByName("MainMenu");
	if (e->visible){
        Graphics.QueueMessage(new GMPopUI("MainMenu"));
		OnCloseMenu();
    }
	else {
	    Graphics.QueueMessage(new GMPushUI("MainMenu"));
		OnOpenMenu();
	}
}

// For resetting position
Entity * Racing::GetCheckpoint(int index){
	if (index < 0)
		index = 0;
	if (racing->checkpoints.Size() <= 0){
		std::cout<<"\nWARNiNG: No checkpoints in map :B";	
		return NULL;
	}
	assert(racing);
	if (index == -1)
		return racing->checkpoints[racing->checkpoints.Size()-1];
	assert(index >= 0 && index < racing->checkpoints.Size());
	return racing->checkpoints[index];
}

/// Creates player entities!
bool Racing::CreatePlayers(List<SRPlayer*> players)
{
	/// TODO: Delete any existing ships first?
	/// Gather necessary assets
	Model * model = ModelMan.LoadObj("racing/ship_UVd.obj");
	Texture * tex = TexMan.LoadTexture("racing/ship_textured.png");

	// Start-positions, should be placed in the editor or somewhere?
	std::cout<<"\nGathering starting positions..";
	int numPlayers = players.Size();
	Vector3f startingPosition;
	Map * map = MapMan.ActiveMap();
	Entity * entity = map->GetEntity("StartingPosition");
	if (entity){
		startingPosition = entity->positionVector;
		/// Disable entity from physics and rendering, in-case it was registered in any of them!
		Physics.QueueMessage(new PMUnregisterEntity(entity));
		Graphics.QueueMessage(new GMUnregisterEntity(entity));
	}
	Vector3f * startPositions = new Vector3f[numPlayers];
	for (int i = 0; i < numPlayers; ++i){
		startPositions[i] = startingPosition + Vector3f(i%2? 20.f : -20.f, 0, i * 20.f);
	}

	/// One for each player, yow.
	std::cout<<"\nCreating player entities..";
	SRSession * srs = GetSession();
	for (int i = 0; i < numPlayers; ++i)
	{
		SRPlayer * player = players[i];
		Entity * entity = CreateShipForPlayer(player, startPositions[i]);
		/// If client, disable physics simulations for all entities.
		if (!srs->IsHost())
		{
			Physics.QueueMessage(new PMSetEntity(SIMULATION_ENABLED, entity, false));
		}
	}
	delete[] startPositions;
	startPositions = NULL;
	return true;
}

/// Sets message to be displayed in-game, for example when host disconnects or returns to the lobby.
void Racing::SetGeneralMessage(String str)
{
	Graphics.QueueMessage(new GMSetUIs("GeneralMessage", GMUI::TEXT, str));
}


///!
void Racing::OnOpenMenu(){

//	Physics.Pause();
//	StateMan.Pause();

	// TODO: Pause "interactive"-graphics?
	menuOpen = true;
	// Disable UI-navigation so that we can use all of the keyboard for player-movement!
    Input.NavigateUI(true);
}
/// ?
void Racing::OnCloseMenu(){
	Physics.Resume();
	StateMan.Resume();
    // Disable UI-navigation so that we can use all of the keyboard for player-movement!
    Input.NavigateUI(false);
	menuOpen = false;
}

/// Increments checkpoints passed and posts updates to the session manager. Also calls OnPlayerCheckpointsPassedUpdated() to update UI.
void Racing::PlayerPassCheckpoint(SRPlayer * player, int checkpointsPassed /*= -1*/){
	if (!GetSession()->IsHost())
		return;
	/// Send a notification of this.
	player->checkpointsPassed++;
	if (checkpointsPassed != -1)
		assert(checkpointsPassed == player->checkpointsPassed);
	int playerIndex = GetPlayerIndex(player);
	SRSession * srs = GetSession();

	/// Send a packet to the peers informing of this player's progress.
	SRRacePacket checkPack("CheckpointsPassed", String::ToString(playerIndex)+";"+String::ToString(player->checkpointsPassed));
	srs->Send(&checkPack);

	RacingShipGlobal * rsg = (RacingShipGlobal*) player->entity->state->GlobalState();
	rsg->RefillBoost(1.0f);
	// Update gui as needed
	OnPlayerCheckpointsPassedUpdated(player);
	
	// Inform the AI that it passed a checkpoint (if any)
	if (rsg->ai){
		Message msg("PassedCheckpoint");
		rsg->ai->ProcessMessage(&msg);
	}
	/// Check if the player completed a lap
	if (player->checkpointsPassed == srs->Checkpoints()){
		player->lapsFinished++;
		player->checkpointsPassed = 0;

		int time = (int)Timer::GetCurrentTimeMs();
		int startTime = GameVars.GetInt(LAP_START_TIME_STR);
		int lapTime = (int) time - player->lastLapStart;
		player->lastLapStart = time;
		player->lapTime = lapTime;
		if (player->lapsFinished <= GetSession()->Laps()){
			player->totalTime += lapTime;
			player->lapTimes.Add(lapTime);
		}
		// Inform of the lap update, including laptime!
		SRRacePacket lapPacket("LapsPassed", String::ToString(playerIndex)+";"+String::ToString(player->lapsFinished)+";"+String::ToString(lapTime));
		srs->Send(&lapPacket);

		/// Check which position all players have and update gui for this player?
		OnPlayerLapsUpdated(player);

		/// If the player finished the race.
		if (player->lapsFinished >= GetSession()->Laps()){
			player->finished = true;
			bool allPlayersFinished = true;

			List<SRPlayer*> players = GetPlayers();
			for (int i = 0; i < players.Size(); ++i){
				if (!players[i]->finished){
					allPlayersFinished = false;
					break;;
				}
			}
			/// Check if all players have finished the race
			if (allPlayersFinished == true){
				Physics.QueueMessage(new PMSetSpeed(0.1f));
				FormatResults();
				/// Hide all separate windows.
				for (int i = 0; i < viewports.Size(); ++i){
					RenderViewport * rv = viewports[i];
					Graphics.QueueMessage(new GMSetUIb("root", GMUI::VISIBILITY, false, rv->ID()));
				}
				Graphics.QueueMessage(new GMSetUIb("Results", GMUI::VISIBILITY, true));
			}
		}
	}

	/// Sort players by checkpoints passed
	List<SRPlayer*> sortedByPosition;

	List<SRPlayer*> players = GetPlayers();
	List<SRPlayer*> temp = players;
	if (temp.Size() >= 1){
		while(temp.Size()){
			SRPlayer * bestPositionOfTheRest = temp[0];
			for (int j = 0; j < temp.Size(); ++j){
				SRPlayer * pl = temp[j];
				if (pl->lapsFinished > bestPositionOfTheRest->lapsFinished){
					bestPositionOfTheRest = pl;
				}
				else if (pl->lapsFinished == bestPositionOfTheRest->lapsFinished &&
						pl->checkpointsPassed > bestPositionOfTheRest->checkpointsPassed)
					bestPositionOfTheRest = pl;
			}
			sortedByPosition.Add(bestPositionOfTheRest);
			temp.Remove(bestPositionOfTheRest);
		}
	}
	/// Update their respective GUIs now
	for (int i = 0; i < sortedByPosition.Size(); ++i){
		SRPlayer * player = sortedByPosition[i];
		int previousPosition = player->position;
		int newPosition = i+1;
		player->position = newPosition;
		if (previousPosition != newPosition){
			OnPlayerPositionUpdated(player);
		}
	}
	/// Send a packet with updated player positions!
	List<String> positions;
	for (int i = 0; i < players.Size(); ++i)
	{
		SRPlayer * player = players[i];
		positions.Add(String::ToString(player->position));
	}
	String positionStringsMerged = MergeLines(positions, ";");
	SRRacePacket pack("PlayerPositions", positionStringsMerged);
	srs->Send(&pack);
}

/// Updates GUI
void Racing::OnPlayerCheckpointsPassedUpdated(SRPlayer * player)
{
	if (player->isLocal == false ||
		player->isAI == true)
		return;
	String text = String::ToString(player->checkpointsPassed) + "/" + String::ToString(checkpoints.Size());
	Graphics.QueueMessage(new GMSetUIs("CheckpointsPassed", GMUI::TEXT, text, player->viewport->ID()));
}



void Racing::OnPlayerPositionUpdated(SRPlayer * player){
	// Update gui
//	std::cout<<"\nOnPlayerPositionUpdated";
	if (player->isLocal && !player->isAI){
		List<SRPlayer*> players = GetPlayers();
		String text = String::ToString(player->position) + "/" + String::ToString(players.Size());
		Graphics.QueueMessage(new GMSetUIs("Position", GMUI::TEXT, text, player->viewport->ID()));
	}
}

String OrdinalNumber(int position)
{
	String str = String::ToString(position);
	int modulo = position % 10;
	// Teens
	if (position % 100 < 20 &&
		position % 100 > 10)
	{
		str += "th";
	}
	/// All other regulars
	else {
		switch(modulo){
			case 1: str += "st"; break;
			case 2: str += "nd"; break;
			case 3: str += "rd"; break;
			case 4: str += "th"; break;
			case 5: case 6: case 7: case 8: case 9: case 0:
				str += "th"; break;
		}
	}
	return str;
}

void Racing::OnPlayerLapsUpdated(SRPlayer * player){
//    std::cout<<"\nOnPlayerLapsUpdated";
	// Update gui
	if (player->isLocal && !player->isAI){
		String text = String::ToString(player->lapsFinished) + "/" + String::ToString(GetSession()->Laps());
		Graphics.QueueMessage(new GMSetUIs("Lap", GMUI::TEXT, text, player->viewport->ID()));

		text = String::ToString(player->lapTime*0.001f,2);
		Graphics.QueueMessage(new GMSetUIs("LapTime", GMUI::TEXT, text, player->viewport->ID()));

		if (player->lapsFinished){
			String lapString;
			lapString += OrdinalNumber(player->lapsFinished)+" lap finished!";
			/// Set lap text
			Graphics.QueueMessage(new GMSetUIs("LapInfoLabel", GMUI::TEXT, lapString, player->viewport->ID()));
			/// Animate text for the player if it was a local one and not AI (i.e. in here!)
			TextAnimationEvent * tae = new TextAnimationEvent(TextAnimationEvent::NOTICE, "LapInfoLabel", player->viewport->ID());
			tae->fadeInDuration = 500 - player->lapsFinished * 50;
			tae->fadeOutDuration = 500 + player->lapsFinished * 100;
			EventMan.PlayEvent(tae);

			// If final lap, post final position too.
			if (player->lapsFinished == GetSession()->Laps())
			{
				/// Set position text
				Graphics.QueueMessage(new GMSetUIs("FinalPositionLabel", GMUI::TEXT, OrdinalNumber(player->position)+" position!", player->viewport->ID()));
				TextAnimationEvent * tae2 = new TextAnimationEvent(TextAnimationEvent::NOTICE, "FinalPositionLabel", player->viewport->ID());
				tae2->fadeInDuration = 50;
				tae2->fadeOutDuration = 10000;
				EventMan.PlayEvent(tae2);

			}
		}
	}
}

/// Formats the Results-div of the swhoshi for when the players finish the race!
void Racing::FormatResults(){

    // Old
    /*
	Graphics.QueueMessage(new GMClearUI("Positions"));
	Graphics.QueueMessage(new GMClearUI("Names"));
	Graphics.QueueMessage(new GMClearUI("TotalLapTimes"));
	Graphics.QueueMessage(new GMClearUI("BestLapTimes"));
	*/
	Graphics.QueueMessage(new GMClearUI("ResultsList"));

	List<SRPlayer*> playersSorted, playersToBeSorted = GetPlayers();
	SRPlayer * best = NULL;
	while(playersToBeSorted.Size()){
		best = playersToBeSorted[0];
		for (int i = 1; i < playersToBeSorted.Size(); ++i){
			/// Sort by laps passed, then total time.
			SRPlayer * sr = playersToBeSorted[i];
			if (sr->lapTimes.Size() > best->lapTimes.Size()){
				best = sr;
			}
			else if (sr->lapTimes.Size() == best->lapTimes.Size() && sr->totalTime < best->totalTime)
				best = sr;
		}
		playersSorted.Add(best);
		playersToBeSorted.Remove(best);
	}

	// assert(playersSorted.Size());

	UIElement * textColorSample = ui->GetElementByName("PositionNumberLabel");
	Vector4f textColor = textColorSample->textColor;

	/// Fetch statistics defined in the ui file (hopefully..)
	float padding = ui->GetElementByName("ResultsTableLabels")->padding;
	std::cout<<"\nPadding pding-ding: "<<padding;

///	UIElement * parent = ui->GetElementByName("ResultsList");
  //  parent->padding = padding;

    float sizeY = ui->GetElementByName("ResultTemplate")->sizeRatioY;

    float nrSizeX = ui->GetElementByName("PositionNumberLabel")->sizeRatioX;
    float nameSizeX = ui->GetElementByName("NameLabel")->sizeRatioX;
    float totTimeSizeX = ui->GetElementByName("TotalTimeLabel")->sizeRatioX;
    float bestTimeSizeX = ui->GetElementByName("BestLapLabel")->sizeRatioX;

	UIColumnList * playerStats = NULL;

	for (int i = 0; i < playersSorted.Size(); ++i){
		SRPlayer * player = (SRPlayer*)playersSorted[i];

#define INITIALIZE(p) {p = new UIElement(); \
		p->textColor = textColor; \
		p->textureSource = "img/80Gray50Alpha.png";\
		}
		UIElement * pos, * name, * totLapTime, * bestLapTime;

        playerStats = new UIColumnList();
        playerStats->padding = padding;
        playerStats->formatX = false;
        playerStats->sizeRatioY = sizeY;
        playerStats->textureSource = "img/80Gray50Alpha.png";

		INITIALIZE(pos);
		pos->text = String::ToString(i+1);
		pos->sizeRatioX = nrSizeX;
		playerStats->AddChild(pos);

		INITIALIZE(name);
		name->text = player->name;
		name->sizeRatioX = nameSizeX;
		playerStats->AddChild(name);

		INITIALIZE(totLapTime);
		totLapTime->text = String::ToString(player->totalTime * 0.001f, 3) + " (" + String::ToString(player->lapTimes.Size())+")";
		totLapTime->sizeRatioX = totTimeSizeX;
		playerStats->AddChild(totLapTime);

		INITIALIZE(bestLapTime);
		assert(player->lapTimes.Size() <= player->lapsFinished);
		int bestLapTimur = 10000000;
		if (player->lapsFinished > 0){
			for (int i = 0; i < player->lapTimes.Size(); ++i){
				int bettrur = player->lapTimes[i];
				if (bettrur < bestLapTimur)
					bestLapTimur = bettrur;
			}
			if (bestLapTimur == 10000000)
				bestLapTimur = 0;
		}
		bestLapTime->text = String::ToString(bestLapTimur * 0.001f, 3);
		bestLapTime->sizeRatioX = bestTimeSizeX;
		playerStats->AddChild(bestLapTime);

        /// Add it to le boiya!
		Graphics.QueueMessage(new GMAddUI(playerStats, "ResultsList"));
	}
}
