// Emil Hedemalm
// 2013-06-28

#include "Game/GameType.h"

#ifdef RUNE_RPG

#include "MapState.h"

#include "OS/Sleep.h"
#include "Message/Message.h"
#include "../UI/UserInterface.h"
#include "../Graphics/Messages/GMUI.h"
#include "../Physics/PhysicsProperty.h"
#include "Graphics/Render/RenderViewport.h"
#include "Physics/Messages/CollissionCallback.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "EntityStates/StateProperty.h"
#include "Graphics/Messages/GMParticleMessages.h"
#include <Util.h>
#include <iomanip>
#include "Maps/2D/TileMap2D.h"
#include "../../EntityStates/Player/ScrollerPlayerState.h"
#include "Player/PlayerManager.h"
#include "Event/Event.h"
#include "Event/EventProperty.h"
#include "Graphics/Camera/Camera.h"
#include "Entity/EntityFlags.h"
#include "../../RuneDirectories.h"
#include "Graphics/Messages/GMSet.h"
#include "../RuneGameStatesEnum.h"
#include "Graphics/Messages/GMSetEntity.h"


#include "Graphics/GraphicsManager.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"
#include "Input/InputManager.h"
#include "Event/EventManager.h"
#include "ModelManager.h"
#include "Graphics/Animation/AnimationManager.h"
#include "Pathfinding/WaypointManager.h"
#include <ctime>

extern UserInterface * ui[MAX_GAME_STATES];

MapState::MapState()
: SideScrollerGameState()
{
	id = RUNE_GAME_STATE_MAP;
	stateName = "MapState";
	enterMode = EnterMode::NULL_MODE;
	camera = new Camera();
	activeMap = NULL;
	player = NULL;
	playerEntity = NULL;
	playerState = NULL;
	lastModifiedEntity = NULL;
	menuOpen = false;
	keyPressedCallback = true;
}

MapState::~MapState(){
	delete camera;
	camera = NULL;
}


/// Creates the user interface for this state
void MapState::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/RuneRPG/MapState.gui");
}

void MapState::OnEnter(GameState * previousState){
	std::cout<<"\nMapState::OnEnter";
	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));
	
	// Load animation sets for map!
	AnimationMan.LoadFromDirectory("anim/RuneRPG/Map");
	// Set physics integrator to simple!
	Physics.QueueMessage(new PMSet(INTEGRATOR, Integrator::SIMPLE_PHYSICS));

	Sleep(100);
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSet(ACTIVE_USER_INTERFACE, ui));
	/// Depending on previous state, modify menu.
	if (previousState->GetID() == RUNE_GAME_STATE_MAIN_MENU){
		/// Hide the menu.
		CloseMenu();
	}

	/// Do special stuff depending on enter-mode.
	assert(enterMode != EnterMode::NULL_MODE && "Forgot to set enter-mode?!");
	if (enterMode == EnterMode::NULL_MODE) {
		std::cout<<"\nMapState::OnEnter Null-mode, returning.";
		StateMan.QueueState(GAME_STATE_MAIN_MENU);
		return;
	}
	/// Continue should probably be default mode, yo.
	else if (enterMode == EnterMode::CONTINUE){
		std::cout<<"\nMapState::OnEnter Continue";
		/// Stuff. o.o
		/// Check if menu is open, if so set NavigateUI to true!
		if (menuOpen){
			Input.NavigateUI(true);
		}
	}
	else if (enterMode == EnterMode::NEW_GAME){
		std::cout<<"\nMapState::OnEnter New game";
		/// Run the newgame-script!
		Event * ev = new Event();
		activeMap = NULL;
		ev->Load("data/RuneRPG/Events/NewGame.e");
		ev->flags |= DELETE_WHEN_ENDED;
		EventMan.PlayEvent(ev);
	}
	else if (enterMode == EnterMode::TESTING_MAP){
		std::cout<<"\nMapState::OnEnter Testing map.";
		assert(activeMap != NULL && "Test map should have been set before entering!");
		// Load all maps that were created/added in the editor...!
		activeMap->ResetEvents();
		Graphics.QueueMessage(new GMSetUIb("MainMenuButton", GMUI::VISIBILITY, false));
		Graphics.QueueMessage(new GMSetUIb("ReturnToEditorButton", GMUI::VISIBILITY, true));
		/// Check for player character/spawn, if not, place somewhere within camera range.
		playerEntity = activeMap->GetEntity("Player");
		if (!playerEntity)
			PlacePlayer(Vector3i(10,10,0));
		/// Call it explicitly if we came from the editor.
		activeMap->OnEnter();
	}
	/// Track camera on ze playah!
	TrackPlayer();

	/// Establish link to player entity state if existing.
	if (playerEntity){
		playerState = (ScrollerPlayerState*)playerEntity->state->GlobalState();
		Graphics.QueueMessage(new GMSetEntity(playerEntity, ANIMATION_SET, "Map/Test"));
	}
	
	// Verify data o-o
	MapMan.GetLighting().VerifyData();

	/// Set editor selection as the renderable one!

	// And set it as active
	Graphics.cameraToTrack = camera;
	Graphics.cameraToTrack->SetRatio(Graphics.width, Graphics.height);
	Graphics.UpdateProjection();
	/// Toggle debug renders
	Graphics.EnableAllDebugRenders(false);
	Graphics.renderFPS = true;
	Graphics.selectionToRender = NULL;
	Graphics.renderNavMesh = true;

	// And start tha music..
#ifdef USE_AUDIO
	AudioMan.Play(BGM, "2013-02-21 Impacto.ogg", true);
#endif

	ResetCamera();

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
}

void MapState::OnExit(GameState *nextState){
	std::cout<<"\nMapState::OnExit";
	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));
	Graphics.QueueMessage(new GMSetViewports(NULL));

	Sleep(100);
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	Graphics.cameraToTrack->entityToTrack = NULL;

	std::cout<<"\nLeaving MapState state.";
	MapMan.MakeActive(NULL);

	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, (Texture*)NULL));
	std::cout<<"\nLeaving MapState statewewe";
	switch(nextState->GetID()){
		case RUNE_GAME_STATE_BATTLE_STATE:
		case RUNE_GAME_STATE_RUNE_STATE:
			enterMode = EnterMode::CONTINUE;
			break;
		default: 
			enterMode = EnterMode::NULL_MODE;
			break;
	}
}

/// For key-bindings.
void MapState::CreateDefaultBindings()
{

}
/// For key-bindings.
void MapState::InputProcessor(int action, int inputDevice /*= 0*/)
{

}

void MapState::Process(float time){
	/// Process key input for navigating the 3D - Space
	Sleep(10);

	// Update player position if possiblu
	if (playerEntity){
		String s = String::ToString(playerEntity->positionVector.x) + ", " + String::ToString(playerEntity->positionVector.y);
		Graphics.QueueMessage(new GMSetUIs("PositionLabel", GMUI::TEXT, s));
	}

#ifdef USE_AUDIO
	AudioMan.Update();
#endif

	/// Last update time in ms
	static clock_t lastTime = 0;
	// Calculate time since last update
	clock_t newTime = clock();
	int timeDiff = newTime - lastTime;
	lastTime = newTime;
};

enum mouseCameraStates {
	NULL_STATE,
	ROTATING,
	PANNING
};


/// Input functions for the various states
void MapState::MouseClick(bool down, int x, int y, UIElement * elementClicked){
	if (down){
		NavMesh * nm = WaypointMan.ActiveNavMesh();
		Waypoint * wp = nm->GetClosestVacantWaypoint(cursorPosition);
		std::cout<<"\nMouse-click waypoint: "<<wp;
		if (wp)
			std::cout<<" pos: "<<wp->position;
		std::cout<<"\n";
		if (playerEntity)
			Physics.QueueMessage(new PMSetEntity(DESTINATION, playerEntity, wp->position));
		/*
		/// Extract position on the plane.
		TileMapLevel * level = map->ActiveLevel();
		Vector2i size = level->Size();
		

		TileType * t = tileType;
		assert(t);
		switch(brushType){
			case SQUARE: {
				for (int i = cursorPosition.x - brushSize; i <= cursorPosition.x + brushSize; ++i){
					if (i < 0 || i >= size.x)
						continue;
					for (int j = cursorPosition.y - brushSize; j <= cursorPosition.y + brushSize; ++j){
						if (j < 0 || j >= size.y)
							continue;
						map->SetTileType(Vector2i(i, j), t);
					}
				}
			}
		}
		Graphics.ResumeRendering();
		*/
	}
}
void MapState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
}

/// Macros to access the active map via the MapManager.
Map * MapState::ActiveMap(){
	Map * map = MapMan.ActiveMap();
	return map;
}
	

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void MapState::MouseMove(float x, float y, bool lDown, bool rDown, UIElement * elementOver){
	if (elementOver)
		return;
	Camera * camera = Graphics.cameraToTrack;
//	float diffX = mouseX - x;
//	float diffY = mouseY - y;
	
	/// Get position in le welt.
	Plane plane;
	plane.Set3Points(Vector3f(0,-1,0), Vector3f(1,0,0), Vector3f(0,0,0));
	Vector3f collissionPoint;
	Ray clickRay = GetRayFromScreenCoordinates(x, y, *Graphics.cameraToTrack);
	if (RayPlaneIntersection(clickRay, plane, &collissionPoint)){
		cursorPosition = collissionPoint;
	}
}

void MapState::MouseWheel(float delta){
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

/// Callback from the Input-manager, query it for additional information as needed.
void MapState::KeyPressed(int keyCode, bool downBefore){
	if (downBefore)
		return;
//	std::cout<<"\nKey pressed: "<<keyCode;
	switch(keyCode){
		case KEY::ESCAPE:
			std::cout<<"\nOpening menu";
			OpenMenu();
			break;
		default: 
			break;
	}
}

void MapState::OpenMenu(){
	/// Open the menu if it isn't open already.
	Input.NavigateUI(true);
	if (!ui->IsInStack("MainMenu")){
		menuOpen = true;
		std::cout<<"\nOpening menu.";
		/// Push it to the stack!
		Graphics.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, true));
		Graphics.QueueMessage(new GMPushUI("MainMenu"));
		DisableMovement();
	}
	else {
		std::cout<<"\nMenu already open o-o";
	}
}
void MapState::CloseMenu(){
	if (menuOpen){
		menuOpen = false;
		Graphics.QueueMessage(new GMPopUI("MainMenu"));
	}
	else
		std::cout<<"\nMenu already closed o-o";
}

void MapState::ProcessMessage(Message * message){

//	std::cout<<"\nRacing::ProcessMessage: ";
	switch(message->type){
		case MessageType::COLLISSION_CALLBACK: {
			CollissionCallback * c = (CollissionCallback*)message;
		//	std::cout<<"\nCollissionCallback received for entity "<<c->one->name<<" and "<<c->two->name;
			if (c->one->state)
				c->one->state->ProcessMessage(message);
//			if (c->one->scaleVector.MaxPart() < 5.0f)
//				;//Physics.QueueMessage(new PMSetEntity(SET_SCALE, c->one, c->one->scaleVector * 1.01f));
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
			string.RemoveInitialWhitespaces();
			if (string == "main_menu"){
				StateMan.QueueState(GAME_STATE_MAIN_MENU);
			}
			else if (string == "stop_testing"){
				assert(enterMode == EnterMode::TEST_LEVEL);
				ReturnToEditor();
			}
			else if (string == "OpenMainMenu"){
				OpenMenu();
			}
			else if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string.Contains("Zone(")){
				List<String> tokens = string.Tokenize("() ,");
				if (tokens.Size() < 2){
					assert(tokens.Size() >= 2);
					return;
				}
				Zone(tokens[1]);
				return;
			}
			else if (string.Contains("PlacePlayer(")){
				std::cout<<"\nPlacing player--";
				List<String> tokens = string.Tokenize("() ,");
				if (tokens.Size() < 3){
					assert(tokens.Size() >= 3);
					return;
				}
				int tileX = tokens[1].ParseInt();
				int tileY = tokens[2].ParseInt();
				PlacePlayer(Vector2i(tileX, tileY));
			}
			else if (string == "TrackPlayer"){
				TrackPlayer();
			}
			else if (string == "MenuClosed"){
				EnableMovement();
				return;
			}
			else if (string == "OnReloadUI"){
				if (this->menuOpen){
					/// Make it visible again.
					this->OpenMenu();
				}
			}
			else if (string == "DisableMovement"){
				playerState->DisableMovement();
				return;
			}
			else if (string == "EnableMovement"){
				playerState->EnableMovement();
				return;
			}
			else if (string.Contains("OnInteract")){
				/*
				if (!lastModifiedEntity){
					std::cout<<"\nERROR: lastModifiedEntity NULL, cannot set OnInteract";
					return;
				}
				assert(lastModifiedEntity);
				if (lastModifiedEntity->events == NULL)
					lastModifiedEntity->events = new EventProperty();
				bool hasInteract = lastModifiedEntity->events->onInteract;
				assert(hasInteract == false);
				if (hasInteract == true){
					std::cout<<"\nERROR: Entity already has OnInteract bound to it!";
					return;
				}
				Event * event = new Event("OnInteract");
				String source = string.Tokenize(" \t")[1];
				event->Load(source);
				lastModifiedEntity->events->onInteract = event;
				*/
			}
		}
	}
	GameState::ProcessMessage(message);
}

// To look at ze player?
void MapState::ResetCamera(){

	camera->projectionType = Camera::ORTHOGONAL;
	camera->rotation = Vector3f();
	camera->zoom = 10.f;
	camera->farPlane = -50.0f;

	camera->SetRatio(Graphics.width, Graphics.height);
	camera->Update();
	/// Reset what parts of the map are rendered too..!
	TileMap2D * map = (TileMap2D *)MapMan.ActiveMap();
	if (map)
		map->SetViewRange(20);
}

// For when testing..!
void MapState::ReturnToEditor(){
	MapMan.MakeActive(NULL);
	StateMan.QueueState(GAME_STATE_EDITOR);
}

// Disables movement for the player!
void MapState::DisableMovement(){
	if (playerState)
		playerState->DisableMovement();
}
void MapState::EnableMovement(){
	if (playerState)
		playerState->EnableMovement();
}

/// New game, load game, testing map, etc.
void MapState::SetEnterMode(int mode) {
	enterMode = mode;
}

void MapState::SetCamera(Camera & reference){
	/// Just copy the camera data
	*camera = reference;
}

/// Place player on ze mappur ^3^
bool MapState::PlacePlayer(Vector3i position){
	

	std::cout<<"\nPlacePlayer: "<<position;
	activeMap = MapMan.ActiveMap();
	assert(activeMap);
	/// check if the player already exists...
	Entity * e = MapMan.ActiveMap()->GetEntity("Player");
	if (e){
		std::cout<<"\nEntity already exists, moving it.";
		Physics.QueueMessage(new PMSetEntity(POSITION, e, e->positionVector));
		return true;
	}

	
	return true;
}

/// Zone to map!
void MapState::Zone(String mapName){
	std::cout<<"\n====================== ";
	std::cout<<"\nZone: "<<mapName;
	String filename = mapName;
	if (activeMap){
		String mn = activeMap->Name();
		if (mn == mapName)
			return;
	}
	if (!filename.Contains(ROOT_MAP_DIR)){
		filename = ROOT_MAP_DIR + filename;
	}
	if (!filename.Contains(".tmap")){
		filename += ".tmap";
	}
	Map * map = MapMan.GetMapBySource(filename);
	if (map == NULL){
		std::cout<<"\nUnable to get map \""<<filename<<"\" to zone to.";
		return;
	}
	map->SetName(mapName);
	MapMan.MakeActive(map);

	/// TODO: Change stuff in the graphics to inform that we are generating a navmesh for the map.
	// Create navmesh.
	NavMesh * navMesh = WaypointMan.GetNavMeshByName(map->Name());
	if (!navMesh){
		navMesh = WaypointMan.CreateNavMesh(map->Name());
		/// TODO: Make sure that it doesn't already have any data, if so clear it?
		/// Create waypoints using tiles in the map!
		TileMap2D * tileMap2D = (TileMap2D*)map;
		tileMap2D->GenerateWaypoints(navMesh);
		WaypointMan.MakeActive(navMesh);
	}
	/// Query the physics-manager to generate a nav-mesh to be used for our map!
//	Physics.QueueMessage(new PMSet(NAV_MESH, navMesh));
}

/// Bind camera to ze playah.-
void MapState::TrackPlayer(){
	camera->entityToTrack = playerEntity;
	camera->trackingMode = TrackingMode::FROM_BEHIND;
}

#endif
