/// Emil Hedemalm
/// 2014-07-29
/** Reboot of the TIFS/Virtus project as was conducted during 10 weeks in the spring of 2013 with the following members:
	- Emil Hedemalm
	- Aksel Kornesjö
	- Cheng Wu
	- Andreas Söderberg
	- Michaela Sjöström
	- Fredric Lind

	Old dev blog:	http://focus.gscept.com/gp13-3/
	Old facebook page:	https://www.facebook.com/VirtusLTU
	Our final release for that first iteration of the project: http://svn.gscept.com/gp13-3/public/Virtus.zip
*/

/// The main/global Application state for the game.

#include "TIFS.h"

TIFS * tifs = NULL;
TIFSMapEditor * mapEditor = NULL;

Camera * firstPersonCamera = NULL;
Camera * thirdPersonCamera = NULL;
Camera * freeFlyCamera = NULL;
Camera * flyingDroneCamera = NULL;

WeatherSystem * weather = NULL;

bool tifsInstancingEnabled = true;

float cameraSmoothing = 0.3f;
float timeDiffS = 0.001f;
int64 timeNowMs = 0;
int timeInMs = 0;
float relativeDroneSpawnRate = 1.f;

/// Lists to clear upon deletion of the map.
List< List<Entity*> *> entityLists; 

#define PlayBGM(s) QueueAudio(new AMPlayBGM(s))

void SetApplicationDefaults()
{
	Application::name = "The Invader from Space / VIRTUS";
	TextFont::defaultFontSource = "font3";
	Time::defaultType = TimeType::MILLISECONDS_NO_CALENDER;
}

void RegisterStates()
{
	tifs = new TIFS();
	StateMan.RegisterState(tifs);
	mapEditor = new TIFSMapEditor();
	StateMan.RegisterState(mapEditor);
	StateMan.QueueGlobalState(tifs);
}


TIFS::TIFS()
{
	mothership = NULL;
	tifsInstancingEnabled = true;
	toolParticles = NULL;
	playerProp = NULL;
	paused = false;
	fieldSize = 50.f;
	halfFieldSize = 25.f;
	grid = new TIFSGrid();
	gridSize = Vector3i(5,5,5);
	gameState = NOT_STARTED;
	mapState = 0;
	mapTime = Time(TimeType::MILLISECONDS_NO_CALENDER);
//	Vector3f mapSize(fieldSize, 100.0f, fieldSize);
	// No need to resize here, do that later.
//	grid->Resize(gridSize, mapSize);
}

TIFS::~TIFS()
{
	SAFE_DELETE(weather);
	SAFE_DELETE(grid);
}

TIFSIntegrator * integrator = 0;
TIFSCD * cd = 0;
TIFSCR * cr = 0;

/// Function when entering this state, providing a pointer to the previous StateMan.
void TIFS::OnEnter(AppState * previousState)
{
	entityLists.Add(&drones, &turretEntities, &motherships, &groundEntities, &playersEntities);
	entityLists.AddItem(&flyingDrones);

	// Setup integrator.
	if (!integrator)
		integrator = new TIFSIntegrator();
	if (!cd)
		cd = new TIFSCD();
	if (!cr)
		cr = new TIFSCR();

	// Set default collision group and filter to environment.
	PhysicsProperty::defaultCollisionCategory = CC_ENVIRON;
	PhysicsProperty::defaultCollisionFilter = CC_PLAYER | CC_DRONE;

	weather = new WeatherSystem();
	weather->Initialize();

	PhysicsQueue.Add(new PMSet(integrator));
	PhysicsQueue.Add(new PMSet(cd));
	PhysicsQueue.Add(new PMSet(cr));
	
	// Set 0 gravity for now..
	PhysicsQueue.Add(new PMSet(PT_GRAVITY, Vector3f()));

	freeFlyCamera = CameraMan.NewCamera("FreeFlyCamera");
	firstPersonCamera = CameraMan.NewCamera("1stPersonCamera");
	firstPersonCamera->trackingMode = TrackingMode::FIRST_PERSON;
	thirdPersonCamera = CameraMan.NewCamera("3rdPersonCamera");
	thirdPersonCamera->trackingMode = TrackingMode::THIRD_PERSON;
	cameras.Add(freeFlyCamera, firstPersonCamera);

	flyingDroneCamera = CameraMan.NewCamera("FlyingDroneCamera");
	flyingDroneCamera->trackingMode = TrackingMode::THIRD_PERSON_AIRCRAFT;

	// Set free form camera as active.
	GraphicsQueue.Add(new GMSetCamera(freeFlyCamera));
	ResetCamera();


	// Create and set up particle system we want to use.
	if (!toolParticles)
	{
		toolParticles = new ToolParticleSystem();
		GraphicsQueue.Add(new GMRegisterParticleSystem(toolParticles));
	}

	// Remove shit.
	
	// Do shit.
	if (!ui)
		CreateUserInterface();
	// Set ui as active?
	GraphicsQueue.Add(new GMSetUI(ui));

	// Remove shit
	GraphicsQueue.Add(new GMSetOverlay(NULL));

	Input.ForceNavigateUI(true);

	// Run OnEnter script.
	ScriptMan.PlayScript("OnEnter.txt");
	
	// Play music p.o
	PlayBGM("music/2015-02-07_Impending.ogg");
//	AudioQueue.Add(new AMPlayBGM("music/2015-02-07_Unknown.ogg"));
}

/// Main processing function, using provided time since last frame.
void TIFS::Process(int timeInMsss)
{
	// Sleep.
	Sleep(50);
	if (paused)
		return;
	timeInMs = timeInMsss % 100;
	timeNowMs += timeInMs;
	timeDiffS = timeInMs * 0.001f;
	
	// Process weather..
	weather->Process(timeInMs);
	/// Check for occuring events.
	if (gameState == GAME_STARTED)
	{
		ProcessMapState();
		CheckEventsOccurring();
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void TIFS::OnExit(AppState * nextState)
{
	weather->Shutdown();
	TIFSBuilding::UnloadTypes();
	GraphicsQueue.Add(new GMUnregisterParticleSystem(toolParticles, true));
}

void TIFS::ProcessMapState()
{
	/// o.o
	mapTime += Time(TimeType::MILLISECONDS_NO_CALENDER, timeInMs);
	/// Under 1k seconds.
	int minutes = mapTime.Minutes();
	// Under 1000 seconds.
	if (minutes < 10)
	{
		mapState |= SPAWNING_DRONES;
		// 3 drones at first spawn, increasing every minute until we reach, is it.. 33 drones per minute. o.o'
		dronesPerSpawn = mapTime.Seconds() / 30 + 3;
		droneSpawnIntervalS = 35;
	}
	else if (minutes < 20)
	{
		mapState = SPAWNING_DRONES | SPAWNING_FLYING_DRONES;
		dronesPerSpawn = 30 - (minutes - 10) * 3;
		droneSpawnIntervalS = 30;
		flyingDronesPerSpawn = minutes / 5 + 6;
		flyingDroneSpawnIntervalS = 77 - mapTime.Minutes();
	}
	else if (minutes < 30)
	{
		// Evently vent
		/// !
		/// Deploy anti-turret turrets.
		/// Deploy more nasty stuff.
	}
	// Deploy nasty
	else if (minutes < 40)
	{
	
	}
	/// Deploy life-extractors
	else if (minutes < 50)
	{
		
	}
	/// Swarm. Prepare end-game.
	else 
	{
	
	}
	/// Spawning drones.
	if (mapState & SPAWNING_DRONES && (mapTime - lastDroneSpawn).Seconds() > droneSpawnIntervalS)
	{
		lastDroneSpawn = mapTime;
		SpawnDrones(dronesPerSpawn * relativeDroneSpawnRate);
	}
	/// Spawning flying drones.
	if (mapState & SPAWNING_FLYING_DRONES && (mapTime - lastFlyingDroneSpawn).Seconds() > flyingDroneSpawnIntervalS)
	{
		lastFlyingDroneSpawn = mapTime;
		assert(false);
		SpawnFlyingDrones(flyingDronesPerSpawn);
	}
}

void TIFS::CheckEventsOccurring()
{
	for (int i = 0; i < eventsYetToPass.Size(); ++i)
	{
		int eventID = eventsYetToPass[i];
		bool eventOccurred = false;
		switch(eventID)
		{
			case FIRST_TURRET_REPAIRED:
			{
				if (turretsRepaired >= 1)
				{
					eventOccurred = true;
					// Play a SFX or voice-thingy?
//					PlayBGM("");
				}			
				break;		
			}
			case FIRST_DRONES_ARRIVE:
			{
				if (dronesArrived > 10)
				{
					eventOccurred = true;
					PlayBGM("music/2015-02-23_D-dribble.ogg");
				}
				break;
			}
			case FIRST_DRONES_DESTROYED:
			{
				if (dronesDestroyed > 10)
				{
					eventOccurred = true;
					PlayBGM("music/2015-02-11_Fight_the_power_2.ogg");
				}
				break;
			}
			case FIRST_FLYING_DRONES_DESTROYED:
			{
				if (flyingDronesDestroyed > 5)
				{
					eventOccurred = true;
					PlayBGM("music/2015-02-11_Fight_the_power_2.ogg");
				}								
				break;
			}
			case FIRST_FLYING_DRONES_ARRIVES:
			{
				if (flyingDronesArrived > 5)
				{
					eventOccurred = true;
					PlayBGM("music/2015-02-07_Swarm.ogg");
				}
				break;
			}
		}
		if (eventOccurred)
		{
			eventsYetToPass.RemoveIndex(i);
			--i;
			eventsOccurred.AddItem(i);
		}
	}
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFS::ProcessPacket(Packet * packet)
{

}
/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFS::ProcessMessage(Message * message)
{	
	String msg = message->msg;
	// o.o
	weather->ProcessMessage(message);
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (playerProp)
			{
				if (msg == "Repair")
				{
					playerProp->SetToolMode(0);
				}
				if (msg == "Activate")
					playerProp->SetToolMode(1);
				if (msg == "RedirectFire")
					playerProp->SetToolMode(2);
				if (msg == "ToggleAutorun")
				{
					// Get our player! o.o
					if (playerProp)
						playerProp->ToggleAutorun();
				}
			}
			/// Turret stats.
			if (msg.StartsWith("TurretCooldown"))
				TIFSTurretProperty::defaultTurretCooldown = (int) msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("TurretPitchYawPerSecond"))
				TIFSTurretProperty::defaultPitchYawPerSecond = msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("TurretProjectileSpeed"))
				TIFSTurretProperty::defaultProjectileSpeed = msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("TurretRecoilSpeed"))
				TIFSTurretProperty::defaultRecoilSpeed = msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("TurretRecoilSpringConstant"))
				TIFSTurretProperty::defaultRecoilSpringConstant = msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("TurretRecoilLinearDamping"))
				TIFSTurretProperty::defaultRecoilLinearDamping= msg.Tokenize("()")[1].ParseFloat();
			
			/// Camera?
			if (msg.StartsWith("CameraSmoothing"))
			{
				cameraSmoothing = msg.Tokenize("()")[1].ParseFloat();
				ResetCamera();
			}
			// Map creation.
			if (msg.StartsWith("SetFieldSize"))
			{
				fieldSize = msg.Tokenize("()")[1].ParseFloat();
				halfFieldSize = fieldSize * 0.5f;
				// Resize.
				grid->Resize(gridSize, Vector3f(fieldSize, 100.f, fieldSize));
			}
			else if (msg.StartsWith("SetGridSize"))
			{
				String vecStr = msg.Tokenize("()")[1];
				Vector3f rr;
				rr.ReadFrom(vecStr);
				gridSize = rr;
				assert(gridSize.Length());
				// Resize.
				grid->Resize(gridSize, Vector3f(fieldSize, 100.f, fieldSize));
			}
			else if (msg.StartsWith("CreateField"))
			{
				CreateField();
			}
			else if (msg.StartsWith("RoadTexture"))
				grid->roadTexture = msg.Tokenize("()")[1];
			else if (msg.StartsWith("RoadWidth"))
			{
				grid->roadWidth = msg.Tokenize("()")[1].ParseInt();				
			}
			else if (msg.StartsWith("MaxRoadLength"))
				grid->maxRoadLength = msg.Tokenize("()")[1].ParseInt();
			else if (msg.StartsWith("RoadScale"))
				grid->roadScale = msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("MinDistanceBetweenParallelRoads"))
			{
				grid->minDistanceBetweenParallelRoads = (int) msg.Tokenize("()")[1].ParseFloat();
			}
			else if (msg.StartsWith("RequireRoadConnections"))
				grid->requireRoadConnections = msg.Tokenize("()")[1].ParseBool();
			else if (msg.StartsWith("ParallelDistanceThreshold"))
				grid->parallelDistanceThreshold = msg.Tokenize("()")[1].ParseFloat();
			else if (msg.StartsWith("PlaceRoads"))
			{
				grid->PlaceRoads(msg.Tokenize("()")[1].ParseInt());
			}
			else if (msg.StartsWith("TriesPerBuilding"))
			{
				grid->triesPerBuilding = msg.Tokenize("()")[1].ParseInt();
			}
			else if (msg.StartsWith("MaxTilesPerBuilding"))
			{
				grid->maxTilesPerBuilding = msg.Tokenize("()")[1].ParseInt();
			}
			else if (msg.StartsWith("AddBuildings"))
			{
				AddBuildings(msg.Tokenize("()")[1].ParseInt());
			}
			else if (msg.StartsWith("CreateTurrets"))
			{
				CreateTurrets(msg.Tokenize("()")[1].ParseInt());
			}
			else if (msg.StartsWith("SpawnDrones"))
			{
				SpawnDrones(msg.Tokenize("()")[1].ParseInt());
			}
			if (msg.Contains("Pause/Break"))
			{
				TogglePause();
			}
			if (msg.Contains("NextCamera"))
			{
				// Get active camera.
				Camera * activeCamera = CameraMan.NextCamera();		
				std::cout<<"\nSwitched to camera: "<<activeCamera->name;
			}
			else if (msg == "NewGame")
			{
				NewGame();
			}
			else if (msg == "SpawnPlayer")
				SpawnPlayer();
			else if (msg.StartsWith("SetPlayer"))
			{
				LogMain("TIFS::SetPlayer", DEBUG);
				List<String> tokens = msg.Tokenize("(,)");
				if (tokens.Size() < 3)
					return;
				String arg = tokens[1], 
					value = tokens[2];
				if (arg == "MovementSpeed")
					TIFSPlayerProperty::defaultMovementSpeed = value.ParseFloat();
				if (arg == "FrictionOnStop")
					TIFSPlayerProperty::defaultFrictionOnStop = value.ParseFloat();
				if (arg == "FrictionOnRun")
					TIFSPlayerProperty::defaultFrictionOnRun = value.ParseFloat();
				if (arg == "JumpSpeed")
					TIFSPlayerProperty::defaultJumpSpeed = value.ParseFloat();
			}
			else if (msg.StartsWith("SetCamera"))
			{
				String which = msg.Tokenize("()")[1];
				if (which == "1stPerson")
					GraphicsQueue.Add(new GMSetCamera(firstPersonCamera));
				if (which == "3rdPerson")
					GraphicsQueue.Add(new GMSetCamera(thirdPersonCamera));
			}
			else if (msg == "ToggleMainMenu")
			{
				// check this somehow..
				UserInterface * ui = ActiveUI();
				if (!ui)
					return;
				UIElement * e = ui->GetElementByName("MainMenu");
				if (!e)
					return;
				bool mainMenuOpen = e->visible;
				if (mainMenuOpen)
				{
					HideMainMenu();
					HideTitle();
					ShowHUD();
				}
				else 
				{
					ShowMainMenu();
					ShowTitle();
					HideHUD();
				}
			}
			else if (msg == "Editor" || msg == "GoToEditor")
			{
				// Go there?
				StateMan.QueueState(mapEditor);
			}
			else if (msg == "ToggleMute")
			{
				// Mute?
				AudioQueue.Add(new AudioMessage(AM_TOGGLE_MUTE));
			}
			else if (msg == "OpenMainMenu")
			{
				GraphicsQueue.Add(new GMSetUI(ui));
				StateMan.QueueState(NULL);
			}
			else if (msg == "ShowHUD")
				ShowHUD();
			else if (msg == "Options")
				OpenOptionsMenu();
			else if (msg == "ResetCamera")
				ResetCamera();
			else if (msg.StartsWith("RelativeDroneSpawnRate"))
			{
				relativeDroneSpawnRate = msg.Tokenize("()")[1].ParseFloat();
			}
			else if (msg == "TestFlyingDrone")
			{
				Entity * drone = SpawnFlyingDrone(Vector3f(0,2000,0));
				std::cout<<"\nTesting flying drone: "<<drone->name;
				Input.SetInputFocus(drone);
				// Setup camera for it
				QueueGraphics(new GMSetCamera(flyingDroneCamera));
				// Set camera focus to this drone.
				QueueGraphics(new GMSetCamera(flyingDroneCamera, CT_ENTITY_TO_TRACK, drone));

			}
			else if (msg == "NextMinute")
			{
				mapTime.SetMinute(mapTime.Minute() + 1);
				std::cout<<"\nMap time set to minute: "<<mapTime.Minute();
			}
			break;	
		}
	}
}

/// Creates default key-bindings for the state.
void TIFS::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	List<Binding*> & bindings = mapping->bindings;
	bindings.Add(new Binding(Action::FromString("ResetCamera"), KEY::HOME));
	bindings.Add(new Binding(Action::FromString("NextCamera"), KEY::C));
//	bindings.Add(new Binding(Action::FromString("ToggleAutorun"), KEY::R));
	bindings.Add(new Binding(Action::FromString("ToggleMainMenu"), KEY::ESC));
	bindings.Add(new Binding(Action::FromString("Repair"), KEY::ONE));
	bindings.Add(new Binding(Action::FromString("Activate"), KEY::TWO));
	bindings.Add(new Binding(Action::FromString("RedirectFire"), KEY::THREE));
	bindings.Add(new Binding(Action::FromString("SpawnDrones(3)"), List<int>(KEY::CTRL, KEY::D)));
#define BIND(a,b) bindings.AddItem(new Binding(a,b));
	BIND(Action::FromString("ToggleMute"), KEY::M);
	BIND(Action::FromString("NextMinute"), KEY::N);
	BIND(Action::FromString("TestFlyingDrone"), List<int>(KEY::T, KEY::F));
	BIND(Action::FromString("AdjustMasterVolume(0.05)"), List<int>(KEY::CTRL, KEY::V, KEY::PLUS));
	BIND(Action::FromString("AdjustMasterVolume(-0.05)"), List<int>(KEY::CTRL, KEY::V, KEY::MINUS));
}


/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void TIFS::CreateUserInterface()
{
	// Delete old one in graphics thread?
//	if (ui)
//		delete ui;
	ui = new UserInterface();
	ui->Load("gui/TIFS.gui");

}

void TIFS::ResetCamera()
{
	// Reset the freeFlyCamera?
	GraphicsQueue.Add(new GMSetCamera(freeFlyCamera, CT_POSITION, Vector3f(0,40,30)));
	GraphicsQueue.Add(new GMSetCamera(freeFlyCamera, CT_ROTATION, Vector3f(-0.4f, 0, 0)));
	
	firstPersonCamera->trackingPositionOffset = Vector3f(0,3.5f,0);
	thirdPersonCamera->trackingPositionOffset = Vector3f(0,3.5f,0);

	thirdPersonCamera->smoothing = cameraSmoothing;

	thirdPersonCamera->minTrackingDistance = 3.5f;
	thirdPersonCamera->maxTrackingDistance = 7.5f;
}

// D:
void TIFS::OnPlayerDead(TIFSPlayerProperty * playerProp)
{
	// check alive players.
	for (int i = 0; i < players.Size(); ++i)
	{
		TIFSPlayerProperty * tpp = players[i];
		if (tpp->currentHP > 0)
			return;
	}
	// GAME OVER
	// Run new game?
	ScriptMan.PlayScript("scripts/GameOver.txt");
}


Random droneRandom;

/// Randomly!!!! o-=o
void TIFS::SpawnDrones(int num)
{
	int numDronesToSpawn = num;
	for (int i = 0; i < numDronesToSpawn; ++i)
	{
		/// Put a hard cap on it.
		if (drones.Size() > 100)
			return;
		Vector3f pos;
		pos.x = droneRandom.Randf(fieldSize) - halfFieldSize;
		pos.z = droneRandom.Randf(fieldSize) - halfFieldSize;
		pos.y = droneRandom.Randf(fieldSize);
		pos += mothership->owner->position;
		SpawnDrone(pos);
	}
}

void TIFS::SpawnFlyingDrones(int num)
{
	int numDronesToSpawn = num;
	for (int i = 0; i < numDronesToSpawn; ++i)
	{
		Vector3f pos;
		pos.x = droneRandom.Randf(fieldSize) - halfFieldSize;
		pos.z = droneRandom.Randf(fieldSize) - halfFieldSize;
		pos.y = droneRandom.Randf(fieldSize);	
		pos += mothership->owner->position;
		SpawnFlyingDrone(pos);
	}
}


void TIFS::SpawnDrone(ConstVec3fr atLocation)
{
	Model * model = ModelMan.GetModel("obj/Drones/Drone.obj");
	Texture * diffuseMap = TexMan.GetTexture("img/Drones/DroneDiffuseMap.png");
//ModelMan.GetModel("Sphere")
	//TexMan.GetTexture("Cyan")
	Entity * drone = EntityMan.CreateEntity("Drone "+String(drones.Size()), model, diffuseMap);
	drone->emissiveMap = TexMan.GetTexture("img/Drones/DroneEmissiveMap.png");
	drone->SetPosition(atLocation);
	TIFSDroneProperty * droneProp = new TIFSDroneProperty(drone);
	drone->properties.Add(droneProp);
	drones.Add(drone);
	MapMan.AddEntity(drone);
	// Setup physics and other stuff.
	droneProp->OnSpawn();
	++dronesAlive;
}

Entity * TIFS::SpawnFlyingDrone(ConstVec3fr atLocation)
{

	Model * model = ModelMan.GetModel("obj/Drones/FlyingDrone.obj");
	Texture * diffuseMap = TexMan.GetTexture("img/Drones/FlyingDrone_Diffuse.png");
//ModelMan.GetModel("Sphere")
	//TexMan.GetTexture("Cyan")
	Entity * drone = EntityMan.CreateEntity("FlyingDrone "+String(flyingDrones.Size()), model, diffuseMap);
	drone->emissiveMap = TexMan.GetTexture("img/Drones/FlyingDrone_Emissive.png");
	drone->specularMap = TexMan.GetTexture("img/Drones/FlyingDrone_Specular.png");
	drone->SetPosition(atLocation);
	TIFSDroneProperty * droneProp = new TIFSDroneProperty(drone);
	droneProp->type = FLYING_DRONE;
	drone->properties.Add(droneProp);
	flyingDrones.AddItem(drone);
	MapMan.AddEntity(drone);
	// Setup physics and other stuff.
	droneProp->OnSpawn();
	++dronesAlive;
	return drone;
}

void TIFS::CreateTurrets(int num)
{
//	MapMan.DeleteEntities(turrets);
	turrets.Clear();

	Random turretRandom;
	int turretsToCreate = num;
	Vector3f lastPos;
	for (int i = 0; i < turretsToCreate; ++i)
	{
		Vector3f position;
		bool ok = grid->GetNewTurretPosition(position);
		if (!ok)
		{
			std::cout<<"\nOut of positions on the grid.";
			break;
		}
		assert(lastPos != position);
		lastPos = position;
//		position.x = turretRandom.Randf(fieldSize) - halfFieldSize;
//		position.z = turretRandom.Randf(fieldSize) - halfFieldSize;
		CreateTurret(LARGE, position);
	}
}


/// Creates a turret!
void TIFS::CreateTurret(int ofSize, ConstVec3fr atLocation)
{
	Texture * diffuseMap = TexMan.GetTexture("img/Turrets/BigTurretDiffuse.png");

	List<Entity*> turretParts;

	Entity * turretBase = EntityMan.CreateEntity("TurretBase", ModelMan.GetModel("Turrets/LargeBase"), diffuseMap);
	turretBase->updateChildrenOnTransform = true;
	turretBase->SetPosition(atLocation);
	turretParts.Add(turretBase);
	PhysicsProperty * pp = new PhysicsProperty();
	turretBase->physics = pp;
	pp->type = PhysicsType::KINEMATIC;
//	pp->fullyDynamic = false;
	pp->collisionCategory = CC_TURRET;
	pp->collisionFilter = CC_PLAYER | CC_DRONE;
	pp->shapeType = ShapeType::SPHERE;
	// Set larger radius to avoid player going through it
	pp->physicalRadius = 5.f;

	/// Add a child-mesh-part to the first turret-part!
	Model * swivel = ModelMan.GetModel("Turrets/LargeSwivel");
	Entity * swivelEntity = EntityMan.CreateEntity("TurretSwivel", swivel, diffuseMap);
	pp = swivelEntity->physics = new PhysicsProperty();
	pp->fullyDynamic = false;
	pp->collisionsEnabled = false;

	/// Make the swivel's transformation depend on the base'.
	GraphicsQueue.Add(new GMSetEntity(swivelEntity, GT_PARENT, turretBase)); 
	turretParts.Add(swivelEntity);

	// Move it up a bit.
	Model * underBarrel = ModelMan.GetModel("Turrets/LargeUnderBarrel");
	Entity * underBarrelEntity = EntityMan.CreateEntity("TurretUnderBarrel", underBarrel, diffuseMap);
	GraphicsQueue.Add(new GMSetEntity(underBarrelEntity, GT_PARENT, swivelEntity));
	underBarrelEntity->SetPosition(Vector3f(0, 1.8f, -0.5f));
	turretParts.Add(underBarrelEntity);
	pp = underBarrelEntity->physics = new PhysicsProperty();
	pp->type = PhysicsType::KINEMATIC;
	pp->fullyDynamic = false;
	pp->collisionsEnabled = false;
	
	// Add barrel.
	Model * barrel = ModelMan.GetModel("Turrets/LargeBarrel");
	Entity * barrelEntity = EntityMan.CreateEntity("TurretBarrel", barrel, diffuseMap);
	GraphicsQueue.Add(new GMSetEntity(barrelEntity, GT_PARENT, underBarrelEntity));
	turretParts.Add(barrelEntity);
	pp = barrelEntity->physics = new PhysicsProperty();
	pp->type = PhysicsType::DYNAMIC;
	pp->gravityMultiplier = 0.0f;
	pp->SetLinearDamping(TIFSTurretProperty::defaultRecoilLinearDamping);
	pp->SetMass(100.0f);
	pp->useForces = true;
	pp->fullyDynamic = false;
	pp->collisionsEnabled = false;


	// Create the ... Turret Property.

	TIFSTurretProperty * prop = new TIFSTurretProperty(turretBase, swivelEntity, underBarrelEntity, barrelEntity);
	turretBase->properties.Add(prop);
	swivelEntity->properties.Add(prop);
	underBarrelEntity->properties.Add(prop);
	barrelEntity->properties.Add(prop);

	/// Call Process only from the base.
	swivelEntity->sharedProperties = true;
	underBarrelEntity->sharedProperties = true;
	barrelEntity->sharedProperties = true;

	prop->turretParts = turretParts;
	prop->turretSize = ofSize;
//	prop->yawPerSecond = prop->pitchPerSecond = pow(2.25f, (SIZES - ofSize)) * 0.2f;

	/// Set position for particles of tool to emit correctly?
	prop->position = atLocation + Vector3f(0,1.f,0);

	/// Set common properties
	Texture * specularMap = TexMan.GetTexture("img/Turrets/BigTurretSpecular.png"),
		* normalMap = TexMan.GetTexture("img/Turrets/BigTurretNormal.png"),
		* emissiveMap = TexMan.GetTexture("img/Turrets/BigTurretEmissive.png");
	for (int i = 0; i < turretParts.Size(); ++i)
	{	
		Entity * part = turretParts[i];
		part->specularMap = specularMap;
		part->normalMap = normalMap;
		part->emissiveMap = emissiveMap;
	}
	
	MapMan.AddEntities(turretParts);
	/// Add turret parts.
	turretEntities.Add(turretParts);
	turrets.AddItem(prop);
}

// Spawn player
void TIFS::SpawnPlayer()
{
	LogMain("SpawnPlayer", DEBUG);
	// Add characters..!
	Model * model;
//	ModelMan.GetModel("Characters/TestCharacter.obj");
	model = ModelMan.GetModel("sphere.obj");
//	model = ModelMan.GetModel("dae/TestCharacter.dae");

	// Get position.
	Vector3f position;
	assert(grid->GetNewPlayerPosition(position));
	position.y += 3.5f;

	Entity * player = MapMan.CreateEntity("Player", model, TexMan.GetTexture("Red"), position);
	// Attach camera to the player.
	GraphicsQueue.Add(new GMSetCamera(thirdPersonCamera, CT_ENTITY_TO_TRACK, player));
	GraphicsQueue.Add(new GMSetCamera(firstPersonCamera, CT_ENTITY_TO_TRACK, player));

	// Set physics stuffs.
	PhysicsProperty * pp = new PhysicsProperty();
	player->physics = pp;
	pp->type = PhysicsType::DYNAMIC;
	// collide.
	pp->collisionCategory = CC_PLAYER;
	pp->collisionFilter = CC_ENVIRON | CC_TURRET | CC_DRONE;
	// Wa-wa wee-wa o.o
	pp->gravityMultiplier = 1.0;
	pp->friction = 0.1f;
	pp->restitution = 0.15f;
	pp->collissionCallback = true;

	// Attach ze propororoty to bind the entity and the player.
	playerProp = new TIFSPlayerProperty(player);
	player->properties.Add(playerProp);
	
	// Enable steering!
	playerProp->inputFocus = true;
	players.AddItem(playerProp);
	playersEntities.Add(player);
	// other stuff.
	PhysicsQueue.Add(new PMSetEntity(player, PT_FACE_VELOCITY_DIRECTION, true));
}

Entity * TIFS::GetClosestDefender(ConstVec3fr toPosition)
{
	if (players.Size() == 0)
		return NULL;
	float minDist = 1000000;
	Entity * closest = NULL;
	for (int i = 0; i < players.Size(); ++i)
	{
		Entity * entity = players[i]->owner;
		float dist = (entity->position - toPosition).LengthSquared();
		if (dist < minDist)
		{
			minDist = dist;
			closest = entity;
		}
	}
	return closest;
}

Turret * TIFS::GetClosestTurret(ConstVec3fr toPosition)
{
	Turret * closest = NULL;
	float closestDist = 1000000;
	for (int i = 0; i < tifs->turrets.Size(); ++i)
	{
		TIFSTurretProperty * turret = tifs->turrets[i];
		float dist = (turret->position - toPosition).LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closest = turret;
		}
	}
	return closest;
}

Turret * TIFS::GetClosestActiveTurret(ConstVec3fr toPosition)
{
	if (turrets.Size() == 0)
		return NULL;
	float minDist = 1000000;
	Turret * closest = NULL;
	for (int i = 0; i < turrets.Size(); ++i)
	{
		TIFSTurretProperty * turret = turrets[i];
		if (!turret->active)
			continue;
		float dist = (turret->position - toPosition).LengthSquared();
		if (dist < minDist)
		{
			minDist = dist;
			closest = turret;
		}
	}
	return closest;
}

void TIFS::TogglePause()
{
	paused = !paused;
	if (paused)
	{
		GraphicsMan.Pause();
		PhysicsMan.Pause();
	}
	else
	{
		GraphicsMan.Resume();
		PhysicsMan.Resume();
	}
}

// Creates a new game with standard stuff.
void TIFS::NewGame()
{
	InputMan.SetInputFocus(NULL);
	// Delete previous entities?
	MapMan.DeleteAllEntities();
	turrets.Clear();
	drones.Clear();
	players.Clear();

	mapState = 0;

	turretsActive = turretsRepaired = 0;
	dronesAlive = dronesArrived = dronesDestroyed = 0;
	flyingDronesArrived = flyingDronesDestroyed = 0;

	mapTime = Time();
	lastDroneSpawn = Time();

	eventsOccurred.Clear();
	for (int i = 0; i < MAX_EVENTS; ++i)
	{
		eventsYetToPass.AddItem(i);
	}

	List<Entity*> allEntities = EntityMan.AllEntities();
	// Unregister all from physics
	PhysicsQueue.Add(new PMUnregisterEntities(allEntities));
	// Unregister all from graphics.
	GraphicsQueue.Add(new GMUnregisterEntities(allEntities));
	/// Mark all for deletion.
	EntityMan.MarkEntitiesForDeletion(allEntities);
	/// Clear all lists we have.
	for (int i = 0; i < entityLists.Size(); ++i)
	{
		List<Entity*> * list = entityLists[i];
		list->Clear();
	}


	// Hide the menu
	HideMainMenu();
	HideTitle();
	ShowHUD();


	// Mothership.
	Entity * motherShip = EntityMan.CreateEntity("Mothership", ModelMan.GetModel("obj/Mothership/Mothership.obj"), TexMan.GetTexture("0x77"));
	Vector3f position = Vector3f(0,1000,0);
	motherShip->position = position;
	motherShip->SetScale(Vector3f(1,1,1) * 3.f);
	Mothership * ms = new Mothership(motherShip);
	motherShip->properties.AddItem(ms);
	mothership = ms;
	ms->position = position;
	PhysicsProperty * pp = motherShip->physics = new PhysicsProperty();
	pp->type = PhysicsType::STATIC;
	MapMan.AddEntity(motherShip);

	/// Set 3rd person camera as default.
	GraphicsQueue.Add(new GMSetCamera(thirdPersonCamera));

	PlayBGM("music/2015-02-07_Unknown.ogg");

	ScriptMan.PlayScript("scripts/NewGame.txt");
	
	gameState = GAME_STARTED;
}

void TIFS::CreateField()
{
	LogMain("TIFS::CreateField", DEBUG);
	// Create a plane.
	List<Entity*> entities;
	Model * model = ModelMan.GetModel("plane");
	Texture * tex = TexMan.GetTexture("0x77");
	Entity * plane = EntityMan.CreateEntity("Plane", model, tex);
	PhysicsProperty * pp = new PhysicsProperty();
	plane->physics = pp;
	pp->shapeType = PhysicsShape::MESH;
	plane->Scale(fieldSize * 5); // Make biggar.

	/*
		tex = TexMan.GetTexture("0x88");
		Entity * plane2 = EntityMan.CreateEntity("Plane 2", model, tex);
		plane2->physics = pp = new PhysicsProperty();
		pp->shapeType = PhysicsShape::MESH;
		plane2->position = Vector3f(1,1,1) * 10;
		plane2->Scale(20);
	*/

	// Add ze entities.
	entities.Add(plane);
	// Add zem to ze mapp
	MapMan.AddEntities(entities);
}

void TIFS::AddBuildings(int numBuildings)
{
	LogMain("TIFS::AddBuildings - start", DEBUG);
//	MapMan.DeleteEntities(turrets);
//	turrets.Clear();
	
	bool ok = TIFSBuilding::LoadTypes();
	assert(ok);

	Random buildingRandom;
	int buildingsToCreate = numBuildings;
	for (int i = 0; i < buildingsToCreate; ++i)
	{
		if (i % 10 == 0)
			std::cout<<"\nCreating building "<<i<<" of "<<buildingsToCreate;
		Vector3f position;
		Vector3f maxSize; 
		List<TIFSTile*> tiles;
		bool ok = grid->GetNewBuildingPosition(maxSize, position, tiles);
		if (!ok)
		{
			std::cout<<"\nOut of positions on the grid.";
			LogMain("TIFS::AddBuildings", DEBUG);
			break;
		}		
		List<Entity*> buildingEntities = TIFSBuilding::CreateNew(position, maxSize, tiles);
		if (!buildingEntities.Size())
			continue;
	}
	LogMain("TIFS::AddBuildings - done", DEBUG);
}

void TIFS::HideMainMenu()
{
	GraphicsQueue.Add(new GMSetUIb("MainMenu", GMUI::VISIBILITY, false));
}

void TIFS::ShowMainMenu()
{
	GraphicsQueue.Add(new GMSetUIb("MainMenu", GMUI::VISIBILITY, true));
}

void TIFS::HideTitle()
{
	GraphicsQueue.Add(new GMSetUIb("TIFS", GMUI::VISIBILITY, false));
}

void TIFS::ShowTitle()
{
	GraphicsQueue.Add(new GMSetUIb("TIFS", GMUI::VISIBILITY, true));
}

String hudSource = "gui/HUD.gui";

void TIFS::ShowHUD()
{
	// Create if needed.
	UserInterface * ui = ActiveUI();
	if (!ui)
		return;
	UIElement * element = NULL;
	/// Check if it's a source file, if so try and load that first.
	element = ui->GetElementBySource(hudSource);
	if (!element)
	{
		// For each player, add it to his screen.. just 1 screen now tho.
		UIElement * element = UserInterface::LoadUIAsElement(hudSource);
		assert(element);
		GraphicsQueue.Add(new GMAddUI(element));
	}
	else 
	{
		// Just make it visible.
		GraphicsQueue.Add(new GMSetUIb("HUD", GMUI::VISIBILITY, true, ui));
	}
}

void TIFS::OpenOptionsMenu()
{
	// Create if needed.
	UserInterface * ui = ActiveUI();
	if (!ui)
		return;
	UIElement * element = NULL;
	String optionsMenu = "gui/Options.gui";
	/// Check if it's a source file, if so try and load that first.
	element = ui->GetElementBySource(optionsMenu);
	if (!element)
	{
		// For each player, add it to his screen.. just 1 screen now tho.
		UIElement * element = UserInterface::LoadUIAsElement(optionsMenu);
		assert(element);
		GraphicsQueue.Add(new GMAddUI(element));
	}
	else 
	{
		// Just make it visible.
		GraphicsQueue.Add(new GMSetUIb("Options", GMUI::VISIBILITY, true, ui));
	}	
	// Push it.
	GraphicsQueue.Add(new GMPushUI("Options", ui));
}

void TIFS::HideHUD()
{
	// Just make it visible.
	GraphicsQueue.Add(new GMSetUIb("HUD", GMUI::VISIBILITY, false, ui));	
}

