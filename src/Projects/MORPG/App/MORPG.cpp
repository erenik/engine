/// Emil Hedemalm
/// 2014-07-27
/// Global state used for this multiplayer online RPG game.

#include "MORPG.h"
#include "MORPGSession.h"

#include "MORPG/Character/Character.h"

#include "MORPG/Properties/MORPGCharacterProperty.h"

#include "MORPG/World/Zone.h"
#include "MORPG/World/WorldGenerator.h"
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

#include "Model.h"

#include "Texture.h"
#include "TextureManager.h"

#include "ModelManager.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Maps/MapManager.h"

/// Only one such session active per application.
MORPGSession * session = NULL;

WorldGenerator * activeWorldGenerator = NULL;
List<WorldGenerator*> worldGenerators;

Camera * mapPreviewCamera = NULL;
Camera * firstPersonCamera = NULL;

List<Camera*> cameras;


void RegisterStates()
{
	MORPG * global = new MORPG();
	StateMan.RegisterState(global);
	StateMan.QueueGlobalState(global);
}

void SetApplicationDefaults()
{
	Application::name = "Time and time again - a MORPG sandbox";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}


MORPG::MORPG()
{
	worldMapEntity = NULL;
	characterProp = NULL;
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

	Input.ForceNavigateUI(true);

	/// World map... 
//	worldMapEntity = MapMan.CreateEntity("World map entity", NULL, NULL);
	Model * plane = ModelMan.GetModel("plane");
	Texture * white = TexMan.GetTexture("White");
	worldMapEntity = MapMan.CreateEntity("World map entity", plane, white);

	// Set up ui.
	if (!ui)
		CreateUserInterface();

	Graphics.QueueMessage(new GMSetUI(ui));
	Graphics.QueueMessage(new GMSetOverlay(NULL));

	/// Create world-generators.
	if (worldGenerators.Size() == 0)
	{
		WorldGenerator * generator = new WorldGenerator();
		worldGenerators.Add(generator);
		activeWorldGenerator = generator;
	}
}

/// Main processing function, using provided time since last frame.
void MORPG::Process(int timeInMs)
{

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void MORPG::OnExit(AppState * nextState)
{

}

/// Creates the user interface for this state
void MORPG::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MORPG.gui");
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void MORPG::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "ToggleAutorun")
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
			if (msg == "NewWorld" || msg == "GenerateWorld")
			{
				world.Delete();
				activeWorldGenerator->GenerateWorld(world);
				Texture * tex = world.GeneratePreviewTexture();
				Model * model = world.GenerateWorldModel();

				if (!worldMapEntity)
					MapMan.CreateEntity("WorldMap", model, tex);
				else
				{
		//			
					Model * plane = ModelMan.GetModel("plane");
					Texture * white = TexMan.GetTexture("White");
					Graphics.QueueMessage(new GMSetEntity(worldMapEntity, GT_MODEL, plane));
					Graphics.QueueMessage(new GMSetEntityTexture(worldMapEntity, DIFFUSE_MAP, white));

					// Re-bufferize the texture.
					Graphics.QueueMessage(new GMBufferTexture(tex));
					// Same for the model..
					Graphics.QueueMessage(new GMBufferMesh(model->GetTriangulatedMesh()));
					// Try our model..
					Graphics.QueueMessage(new GMSetEntity(worldMapEntity, GT_MODEL, model));

					Graphics.QueueMessage(new GMSetEntityTexture(worldMapEntity, DIFFUSE_MAP, tex));
					Physics.QueueMessage(new PMSetEntity(worldMapEntity, PT_SET_SCALE, Vector3f(15.f, 1.f, 15.f)));
				}
				return;
			}
			else if (msg == "MoarWater")
			{
				activeWorldGenerator->water += 0.05f;
				if (activeWorldGenerator->water > 0.95f)
					activeWorldGenerator->water = 0.95f;
				MesMan.QueueMessages("GenerateWorld");
			}
			else if (msg == "LessWater")
			{
				activeWorldGenerator->water -= 0.05f;
				if (activeWorldGenerator->water < 0.0f)
					activeWorldGenerator->water = 0.0f;
				MesMan.QueueMessages("GenerateWorld");
			}
			else if (msg == "SaveWorld")
			{
				std::fstream file;
				file.open("tmp.world", std::ios_base::out);
				if (!file.is_open())
					return;
				world.WriteTo(file);
				file.close();			
			}
			else if (msg == "LoadWorld")
			{
				std::fstream file;
				file.open("tmp.world", std::ios_base::in);
				if (!file.is_open())
					return;
				world.ReadFrom(file);
				file.close();
			}
			break;
		}
	}
}

/// Creates default key-bindings for the state.
void MORPG::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping;
	mapping->CreateBinding("ToggleAutorun", KEY::R);
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



