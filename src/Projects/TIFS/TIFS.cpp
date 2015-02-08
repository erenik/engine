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
	Sleep(100);
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
			if (msg.Contains("NextCamera"))
			{
				// Get active camera.
				CameraMan.NextCamera();		
			}
			else if (msg == "NewGame")
			{
				NewGame();
			}
			else if (msg.Contains("Rain"))
			{
				float amount = msg.Tokenize(" ")[1].ParseFloat();
				weather->Rain(amount);
			}
			else if (msg.Contains("Snow"))
			{
				float amount = msg.Tokenize(" ")[1].ParseFloat();
				weather->Snow(amount);
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
			else if (msg == "CreateTurrets")
			{
				CreateTurrets();
			}
			else if (msg == "SpawnDrones")
			{
				SpawnDrones();
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
void TIFS::SpawnDrones()
{
	MapMan.DeleteEntities(drones);
	drones.Clear();

	int numDronesToSpawn = 1;
	for (int i = 0; i < numDronesToSpawn; ++i)
	{
		Vector3f pos;
		pos.x = droneRandom.Randf(50.f) - 25.f;
		pos.z = droneRandom.Randf(50.f) - 25.f;
		pos.y = droneRandom.Randf(50.f);
	
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

void TIFS::CreateTurrets()
{
	MapMan.DeleteEntities(turrets);
	turrets.Clear();

	Random turretRandom;
	int turretsToCreate = 1;
	for (int i = 0; i < turretsToCreate; ++i)
	{
		Vector3f position;
		position.x = turretRandom.Randf(50.f) - 25.f;
		position.z = turretRandom.Randf(50.f) - 25.f;
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
	model = ModelMan.GetModel("dae/TestCharacter.dae");

	Entity * player = MapMan.CreateEntity("Player", model, TexMan.GetTexture("Red"));
	// Attach camera to the player.
	Graphics.QueueMessage(new GMSetCamera(thirdPersonCamera, CT_ENTITY_TO_TRACK, player));
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, CT_ENTITY_TO_TRACK, player));
	

	// Attach ze propororoty to bind the entity and the player.
	playerProp = new TIFSPlayerProperty(player);
	player->properties.Add(playerProp);
	playerProp->movementSpeed = 5.f;
	
	// Enable steering!
	playerProp->inputFocus = true;

	Physics.QueueMessage(new PMSetEntity(player, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
	PhysicsMan.QueueMessage(new PMSetEntity(player, PT_FACE_VELOCITY_DIRECTION, true));
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
	CreateTurrets();

	// Spawn drones
	SpawnDrones();

	// Spawn player
	SpawnPlayer();

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

