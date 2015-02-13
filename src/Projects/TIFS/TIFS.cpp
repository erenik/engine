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
#include "TIFSMapEditor.h"

#include "TIFS/Physics/TIFSIntegrator.h"
#include "TIFS/Physics/TIFSCD.h"
#include "TIFS/Physics/TIFSCR.h"

#include "TIFS/Properties/TIFSPlayerProperty.h"
#include "TIFS/Properties/TIFSTurretProperty.h"
#include "TIFS/Properties/TIFSDroneProperty.h"

#include "TIFS/Graphics/ToolParticles.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Camera/Camera.h"

#include "Message/Message.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Input/InputManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Random/Random.h"

#include "Script/ScriptManager.h"

#include "Weather/WeatherSystem.h"

TIFS * tifs = NULL;
TIFSMapEditor * mapEditor = NULL;

Camera * firstPersonCamera = NULL;
Camera * thirdPersonCamera = NULL;
Camera * freeFlyCamera = NULL;

WeatherSystem * weather = NULL;

void SetApplicationDefaults()
{
	Application::name = "The Invader from Space / VIRTUS";
	TextFont::defaultFontSource = "font3";
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
	toolParticles = NULL;
	playerProp = NULL;
	paused = false;
	fieldSize = 50.f;
	halfFieldSize = 25.f;
}

TIFS::~TIFS()
{
	SAFE_DELETE(weather);
}

TIFSIntegrator * integrator = 0;
TIFSCD * cd = 0;
TIFSCR * cr = 0;

/// Function when entering this state, providing a pointer to the previous StateMan.
void TIFS::OnEnter(AppState * previousState)
{
	// Setup integrator.
	if (!integrator)
		integrator = new TIFSIntegrator();
	if (!cd)
		cd = new TIFSCD();
	if (!cr)
		cr = new TIFSCR();

	weather = new WeatherSystem();
	weather->Initialize();

	Physics.QueueMessage(new PMSet(integrator));
	Physics.QueueMessage(new PMSet(cd));
	Physics.QueueMessage(new PMSet(cr));
	
	// Set 0 gravity for now..
	Physics.QueueMessage(new PMSet(PT_GRAVITY, Vector3f()));

	freeFlyCamera = CameraMan.NewCamera("FreeFlyCamera");
	firstPersonCamera = CameraMan.NewCamera("1stPersonCamera");
	firstPersonCamera->trackingMode = TrackingMode::FIRST_PERSON;
	thirdPersonCamera = CameraMan.NewCamera("3rdPersonCamera");
	thirdPersonCamera->trackingMode = TrackingMode::THIRD_PERSON;
	cameras.Add(freeFlyCamera, firstPersonCamera);

	// Set free form camera as active.
	Graphics.QueueMessage(new GMSetCamera(freeFlyCamera));
	ResetCamera();


	// Create and set up particle system we want to use.
	if (!toolParticles)
	{
		toolParticles = new ToolParticleSystem();
		GraphicsMan.QueueMessage(new GMRegisterParticleSystem(toolParticles));
	}

	// Remove shit.
	
	// Do shit.

	if (!ui)
		CreateUserInterface();
	// Set ui as active?
	Graphics.QueueMessage(new GMSetUI(ui));

	// Remove shit
	Graphics.QueueMessage(new GMSetOverlay(NULL));

	Input.ForceNavigateUI(true);

	// Run OnEnter script.
	ScriptMan.PlayScript("OnEnter.txt");

}

/// Main processing function, using provided time since last frame.
void TIFS::Process(int timeInMs)
{
	// Sleep.
	Sleep(50);
	if (paused)
		return;
	timeInMs %= 100;
	weather->Process(timeInMs);
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void TIFS::OnExit(AppState * nextState)
{
	weather->Shutdown();
	GraphicsMan.QueueMessage(new GMUnregisterParticleSystem(toolParticles, true));
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
			if (msg.StartsWith("SetFieldSize"))
			{
				fieldSize = msg.Tokenize("()")[1].ParseFloat();
				halfFieldSize = fieldSize * 0.5;
			}
			else if (msg.StartsWith("CreateTurrets"))
			{
				CreateTurrets(msg.Tokenize("()")[1].ParseFloat());
			}
			else if (msg.StartsWith("SpawnDrones"))
			{
				SpawnDrones(msg.Tokenize("()")[1].ParseFloat());
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
			else if (msg == "ToggleMainMenu")
			{
				// check this somehow..
				UserInterface * ui = ActiveUI();
				UIElement * e = ui->GetElementByName("MainMenu");
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
			else if (msg == "OpenMainMenu")
			{
				Graphics.QueueMessage(new GMSetUI(ui));
				StateMan.QueueState(NULL);
			}
			else if (msg == "ResetCamera")
				ResetCamera();
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
	bindings.Add(new Binding(Action::FromString("ToggleAutorun"), KEY::R));
	bindings.Add(new Binding(Action::FromString("ToggleMainMenu"), KEY::ESC));
	bindings.Add(new Binding(Action::FromString("Repair"), KEY::F1));
	bindings.Add(new Binding(Action::FromString("Activate"), KEY::F2));
	bindings.Add(new Binding(Action::FromString("RedirectFire"), KEY::F3));
}


/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void TIFS::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/TIFS.gui");

}

void TIFS::ResetCamera()
{
	// Reset the freeFlyCamera?
	Graphics.QueueMessage(new GMSetCamera(freeFlyCamera, CT_POSITION, Vector3f(0,40,30)));
	Graphics.QueueMessage(new GMSetCamera(freeFlyCamera, CT_ROTATION, Vector3f(-0.4f, 0, 0)));
	
	firstPersonCamera->trackingPositionOffset = Vector3f(0,3.5f,0);
	thirdPersonCamera->trackingPositionOffset = Vector3f(0,3.5f,0);

	thirdPersonCamera->minTrackingDistance = 3.5f;
	thirdPersonCamera->maxTrackingDistance = 7.5f;
}


Random droneRandom;

/// Randomly!!!! o-=o
void TIFS::SpawnDrones(int num)
{
	MapMan.DeleteEntities(drones);
	drones.Clear();

	int numDronesToSpawn = num;
	for (int i = 0; i < numDronesToSpawn; ++i)
	{
		Vector3f pos;
		pos.x = droneRandom.Randf(fieldSize) - halfFieldSize;
		pos.z = droneRandom.Randf(fieldSize) - halfFieldSize;
		pos.y = droneRandom.Randf(fieldSize);
	
		SpawnDrone(pos);
	}
}


void TIFS::SpawnDrone(ConstVec3fr atLocation)
{
	Model * model = ModelMan.GetModel("obj/Drones/Drone.obj");
	Texture * diffuseMap = TexMan.GetTexture("img/Drones/DroneDiffuseMap.png");
//ModelMan.GetModel("Sphere")
	//TexMan.GetTexture("Cyan")
	Entity * drone = MapMan.CreateEntity("Drone", model, diffuseMap, atLocation);
	TIFSDroneProperty * droneProp = new TIFSDroneProperty(drone);
	drone->properties.Add(droneProp);
	drones.Add(drone);
	// Setup physics and other stuff.
	droneProp->OnSpawn();
}

void TIFS::CreateTurrets(int num)
{
	MapMan.DeleteEntities(turrets);
	turrets.Clear();

	Random turretRandom;
	int turretsToCreate = num;
	for (int i = 0; i < turretsToCreate; ++i)
	{
		Vector3f position;
		position.x = turretRandom.Randf(fieldSize) - halfFieldSize;
		position.z = turretRandom.Randf(fieldSize) - halfFieldSize;
		CreateTurret(LARGE, position);
	}
}


/// Creates a turret!
void TIFS::CreateTurret(int ofSize, ConstVec3fr atLocation)
{
	Texture * diffuseMap = TexMan.GetTexture("img/Turrets/BigTurretDiffuse.png");
// TexMan.GetTexture("Green")

	List<Entity*> turretParts;

	Entity * turretBase = MapMan.CreateEntity("TurretBase", ModelMan.GetModel("Turrets/LargeBase"), diffuseMap);
	turretBase->updateChildrenOnTransform = true;
	Physics.QueueMessage(new PMSetEntity(turretBase, PT_POSITION, atLocation));
	turretParts.Add(turretBase);


	/// Add a child-mesh-part to the first turret-part!
	Model * swivel = ModelMan.GetModel("Turrets/LargeSwivel");
	Entity * swivelEntity = MapMan.CreateEntity("TurretSwivel", swivel, diffuseMap);
	
	/// Make the swivel's transformation depend on the base'.
	Graphics.QueueMessage(new GMSetEntity(swivelEntity, GT_PARENT, turretBase)); 
	turretParts.Add(swivelEntity);

	// Move it up a bit.
	Model * underBarrel = ModelMan.GetModel("Turrets/LargeUnderBarrel");
	Entity * underBarrelEntity = MapMan.CreateEntity("TurretUnderBarrel", underBarrel, diffuseMap, Vector3f(0, 1.8f, -0.5f));
	Graphics.QueueMessage(new GMSetEntity(underBarrelEntity, GT_PARENT, swivelEntity));
	turretParts.Add(underBarrelEntity);

	// Add barrel.
	Model * barrel = ModelMan.GetModel("Turrets/LargeBarrel");
	Entity * barrelEntity = MapMan.CreateEntity("TurretBarrel", barrel, diffuseMap);
	Graphics.QueueMessage(new GMSetEntity(barrelEntity, GT_PARENT, underBarrelEntity));
	turretParts.Add(barrelEntity);


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

	prop->turretSize = ofSize;
	prop->yawPerSecond = prop->pitchPerSecond = pow(2.25f, (SIZES - ofSize)) * 0.2f;

	/// Add turret parts.
	turrets.Add(turretParts);
}

// Spawn player
void TIFS::SpawnPlayer()
{
	// Add characters..!
	Model * model;
//	ModelMan.GetModel("Characters/TestCharacter.obj");
	model = ModelMan.GetModel("sphere.obj");
//	model = ModelMan.GetModel("dae/TestCharacter.dae");

	Entity * player = MapMan.CreateEntity("Player", model, TexMan.GetTexture("Red"));
	// Attach camera to the player.
	Graphics.QueueMessage(new GMSetCamera(thirdPersonCamera, CT_ENTITY_TO_TRACK, player));
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, CT_ENTITY_TO_TRACK, player));
	

	// Attach ze propororoty to bind the entity and the player.
	playerProp = new TIFSPlayerProperty(player);
	player->properties.Add(playerProp);
	playerProp->movementSpeed = 15.f;
	
	// Enable steering!
	playerProp->inputFocus = true;

	Physics.QueueMessage(new PMSetEntity(player, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
	PhysicsMan.QueueMessage(new PMSetEntity(player, PT_FACE_VELOCITY_DIRECTION, true));
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
	// Delete previous entities?
	MapMan.DeleteAllEntities();

	// Hide the menu
	HideMainMenu();
	HideTitle();
	ShowHUD();

	// Spawn turrets
//	CreateTurrets();

	// Spawn drones
//	SpawnDrones();

	// Spawn player
	SpawnPlayer();

	// Create a plane.
	Model * model = ModelMan.GetModel("plane");
	Texture * tex = TexMan.GetTexture("0xAA");
	Entity * plane = MapMan.CreateEntity("Plane", model, tex);
	PhysicsMan.QueueMessage(new PMSetEntity(plane, PT_SET_SCALE, fieldSize));

	tex = TexMan.GetTexture("0xEE");
	plane = MapMan.CreateEntity("Plane 2", model, tex);
	PhysicsMan.QueueMessage(new PMSetEntity(plane, PT_SET_SCALE, 10.f));
	PhysicsMan.QueueMessage(new PMSetEntity(plane, PT_POSITION, Vector3f(5,5,5)));


	// Mothership.
	Entity * mothership = MapMan.CreateEntity("Mothership", ModelMan.GetModel("obj/Mothership/Mothership.obj"), TexMan.GetTexture("0x77"));
	PhysicsMan.QueueMessage(new PMSetEntity(mothership, PT_POSITION, Vector3f(0,1000,0)));
	PhysicsMan.QueueMessage(new PMSetEntity(mothership, PT_SET_SCALE, 3.f));

	/// Set 3rd person camera as default.
	GraphicsMan.QueueMessage(new GMSetCamera(thirdPersonCamera));
}


void TIFS::HideMainMenu()
{
	Graphics.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, false));
}

void TIFS::ShowMainMenu()
{
	Graphics.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, true));
}

void TIFS::HideTitle()
{
	Graphics.QueueMessage(new GMSetUIb("TIFS", GMUI::VISIBILITY, false));
}

void TIFS::ShowTitle()
{
	Graphics.QueueMessage(new GMSetUIb("TIFS", GMUI::VISIBILITY, true));
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
		Graphics.QueueMessage(new GMAddUI(element));
	}
	else 
	{
		// Just make it visible.
		Graphics.QueueMessage(new GMSetUIb("HUD", GMUI::VISIBILITY, true, ui));
	}
}

void TIFS::HideHUD()
{
	// Just make it visible.
	Graphics.QueueMessage(new GMSetUIb("HUD", GMUI::VISIBILITY, false, ui));	
}

