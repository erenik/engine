/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "MORPG.h"

#include "MHost.h"

#include "MORPGSession.h"

#include "MORPG/Character/Character.h"

#include "MORPG/Properties/MORPGCharacterProperty.h"

#include "MORPG/World/Zone.h"
#include "MORPG/World/World.h"

#include "MORPG/Physics/MORPGIntegrator.h"
#include "MORPG/Physics/MORPGCD.h"
#include "MORPG/Physics/MORPGCR.h"

#include "Application/Application.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Input/InputManager.h"
#include "Input/Action.h"

#include "Model/Model.h"

#include "Texture.h"
#include "TextureManager.h"

#include "Model/ModelManager.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Maps/MapManager.h"

#include "Network/NetworkManager.h"

/// Only one such session active per application.
MORPGSession * session = NULL;

Camera * mapPreviewCamera = NULL;
Camera * firstPersonCamera = NULL;

List<Camera*> cameras;

MHost * hostState = NULL;

void RegisterStates()
{
	MORPG * global = new MORPG();
	hostState = new MHost();
	StateMan.RegisterState(global);
	StateMan.RegisterState(hostState);
	StateMan.QueueGlobalState(global);
}

void SetApplicationDefaults()
{
	Application::name = "Time and time again - a MORPG sandbox";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}


MORPG::MORPG()
{
	characterProp = NULL;
}

MORPG::~MORPG()
{
	// Delete the world..!
	world.Delete();
}


/// Function when entering this state, providing a pointer to the previous StateMan.
void MORPG::OnEnter(AppState * previousState)
{
	// Set integrator
	Physics.QueueMessage(new PMSet(new MORPGIntegrator()));
	Physics.QueueMessage(new PMSet(new MORPGCD()));
	Physics.QueueMessage(new PMSet(new MORPGCR()));

	if (!mapPreviewCamera)
		mapPreviewCamera = CameraMan.NewCamera();
	if (!firstPersonCamera)
		firstPersonCamera = CameraMan.NewCamera();
	cameras.Add(mapPreviewCamera);
	cameras.Add(firstPersonCamera);

	Graphics.QueueMessage(new GMSetCamera(mapPreviewCamera, CT_ROTATION, Vector3f()));
	Graphics.QueueMessage(new GMSetCamera(mapPreviewCamera, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT, 3.f));
	Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, CT_TRACKING_MODE, TrackingMode::FOLLOW_AND_LOOK_AT));
	// Set it to follow and track us too.
	
	session = new MORPGSession();
	NetworkMan.AddSession(session);

	Input.ForceNavigateUI(true);

	/// World map... 
//	worldMapEntity = MapMan.CreateEntity("World map entity", NULL, NULL);

	// Set up ui.
	if (!ui)
		CreateUserInterface();

	Graphics.QueueMessage(new GMSetUI(ui));
	Graphics.QueueMessage(new GMSetOverlay(NULL));

}

/// Main processing function, using provided time since last frame.
void MORPG::Process(int timeInMs)
{

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void MORPG::OnExit(AppState * nextState)
{
	// Will never be called.
}

/// Creates the user interface for this state
void MORPG::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MainMenu.gui");
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MORPG::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "NewWorld")
			{
				hostState->enterMode = MHost::WORLD_CREATION;
				StateMan.QueueState(hostState);
			}
			else if (msg == "ToggleAutorun")
			{
				// Get our player! o.o
				if (characterProp)
					characterProp->ToggleAutorun();
			}
			else if (msg.Contains("NextCamera"))
			{
				// Get active camera.
				CameraMan.NextCamera();		
			}
			else if (msg.Contains("LoadMap"))
			{
				String mapName = msg.Tokenize(":")[1];
				Zone * zone = world.GetZoneByName(mapName);
				EnterZone(zone);
			}
			else if (msg == "NewGame")
			{
				// Load a zone!
				String username = "UserName1";
				String password = "password";
				session->Login(username, password);

			}
			break;
		}
	}
}

/// Creates default key-bindings for the state.
void MORPG::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	mapping->bindings.Add(new Binding(Action::FromString("ToggleAutorun"), KEY::R));
}

/// Load map/zone by name
void MORPG::EnterZone(Zone * zone)
{
	// Detach cameras.
	// .. TODO

	/// Decide format and stuff later.
	/// Create a test map for now.
	/// First clear ALL entities.
	MapMan.DeleteAllEntities();

	// Test-level of doom >:)
	bool test = true;
	if (test)
	{
		// Create the test level..!
		MapMan.CreateEntity("Base", ModelMan.GetModel("Zones/Test.obj"), TexMan.GetTexture("White"));

		// Add characters..!
		Entity * player = MapMan.CreateEntity("Player", ModelMan.GetModel("Characters/TestCharacter.obj"), TexMan.GetTexture("Red"));
		// Attach camera to the player.
		Graphics.QueueMessage(new GMSetCamera(firstPersonCamera, CT_ENTITY_TO_TRACK, player));

		/// o-o...
		Character * character = new Character();
		world.characters.Add(character);

		// Attach ze propororoty to bind the entity and the player.
		characterProp = new MORPGCharacterProperty(player, character);
		player->properties.Add(characterProp);
		
		// Enable steering!
		characterProp->inputFocus = true;

		Physics.QueueMessage(new PMSetEntity(player, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));

		return;
	}
	/// Load the base/zone model(s).
	if (zone)
	{
		zone->CreateEntities();
	}
	else 
	{
	}
	
	/// Create the characters within.
}



