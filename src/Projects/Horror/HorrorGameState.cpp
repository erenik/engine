// Emil Hedemalm
// 2015-05-17

#include "HorrorGameState.h"
#include "StateManager.h"
#include "Graphics/Fonts/Font.h"
#include "ApplicationDefaults.h"
#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMEntity.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMLight.h"
#include "Graphics/Messages/GMSet.h"
#include "Message/Message.h"
#include "Input/Keys.h"
#include "Graphics/Camera/Camera.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Entity/EntityManager.h"
#include "Audio/TrackManager.h"
#include "UI/UserInterface.h"
#include "Graphics/Messages/GMUI.h"

const String applicationName = "Horror";

void RegisterStates()
{
	HorrorGameState * gs = new HorrorGameState();
	StateMan.RegisterState(gs);
	StateMan.QueueState(gs);
}

/// Call to set application name, root directories for various features, etc.
void SetApplicationDefaults()
{
	FilePath::workingDirectory = "/bin";
	TextFont::defaultFontSource = "font3";
	UIElement::defaultTextureSource = "80Gray50Alpha.png";
	UserInterface::rootUIDir = "gui/";
}


HorrorGameState::HorrorGameState()
{
	cameraType = TRACK_PLAYER;
	roomSize = 6.0f;
	scale = 2.0f;
}

void HorrorGameState::OnEnter(GameState *)
{
	// We require nice physics... maybe.
	Physics.checkType = OCTREE; // AABB_SWEEP or OCTREE
	Physics.integrator =  Integrator::SPACE_RACE_CUSTOM_INTEGRATOR; // Integrator namespace.
	Physics.collissionResolver = CollissionResolver::CUSTOM_SPACE_RACE_PUSHBACK; // CUSTOM_SPACE_RACE_PUSHBACK or LAB_PHYSICS_IMPULSES

	Model * roomModel = ModelMan.GetModel(ROOM_NAME);
	roomSize = roomModel->aabb.max.x - roomModel->aabb.min.x;
	roomSize *= scale;

	Graphics.SetUI(ui);

	Graphics.renderGrid = false;

	NewGame();
}

void HorrorGameState::Process(float)
{

	// This check should maybe be optimized somehow later?
	List<Entity*> entities = MapMan.GetEntities();

	// Check player position.
	Vector3f position = player->position;
	int roomX = (int)((position.x + roomSize * 0.5f) / roomSize);
	int roomZ = (int)((position.z + roomSize * 0.5f) / roomSize);
	position.x = (float)roomX;
	position.z = (float)roomZ;

	int roomMatrixSize = 5;

	// Update room in gui
	Graphics.QueueMessage(new GMSetUIs("Room", GMUI::TEXT, "Room X: "+String::ToString(roomX)+" Z: "+String::ToString(roomZ)));

	// Generate list of tiles we should plant.
	for (int x = (int)position.x - roomMatrixSize; x <= position.x + roomMatrixSize; ++x)
	{
		for (int z = (int)position.z - roomMatrixSize; z <= position.z + roomMatrixSize; ++z)
		{
			// Check for grid here.
			String entityName = "Room x="+String::ToString(x)+" z="+String::ToString(z);
			Entity * room = MapMan.GetEntityByName(entityName);
			if (!room)
				CreateRoom(Vector2i(x,z), Vector3f(x * roomSize,0,z * roomSize));
		}
	}
}

void HorrorGameState::OnExit(GameState *)
{}



void HorrorGameState::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
	case MessageType::STRING:
		{
			if (msg == "NewGame")
				NewGame();
			else if (msg == "Forward")
				forward = true;
			else if (msg == "StopForward")
				forward = false;
			else if (msg == "Backward")
				backward = true;
			else if (msg == "StopBackward")
				backward = false;
			else if (msg == "Left")
				left = true;
			else if (msg == "StopLeft")
				left = false;
			else if (msg == "Right")
				right = true;
			else if (msg == "StopRight")
				right = false;
			else if (msg == "SwitchCamera")
			{
				cameraType = (cameraType + 1) % CAMERA_TYPES;
				OnCameraUpdated();
			}
			else if (msg == "Physics")
				Graphics.renderPhysics = !Graphics.renderPhysics;
		}
		// Update movement as needed
#define TURN 8.5f
		if (right && !left)
			Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, player, Vector3f(0, -TURN, 0)));
		else if (left)
			Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, player, Vector3f(0, TURN, 0)));
		else
			Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, player, Vector3f()));
#define MOVE_ACC 35.f
		if (forward && !backward)
			Physics.QueueMessage(new PMSetEntity(ACCELERATION, player, Vector3f(0,0,-MOVE_ACC)));
		else if (backward)
			Physics.QueueMessage(new PMSetEntity(ACCELERATION, player, Vector3f(0,0,MOVE_ACC)));
		else
			Physics.QueueMessage(new PMSetEntity(ACCELERATION, player, Vector3f()));
		
		// For controlling speed.
		if (right || left || forward || backward)
			Physics.QueueMessage(new PMSetEntity(FRICTION, player, 0.09f));
		else {
			// Add friction to reduce the velocity!
			Physics.QueueMessage(new PMSetEntity(FRICTION, player, 0.5f));
		}
	}
}

void HorrorGameState::CreateDefaultBindings()
{
	InputMapping * mapping = &this->inputMapping; 
	mapping->CreateBinding("Physics", KEY::R, KEY::P);
	mapping->CreateBinding("NewGame", KEY::N);
	mapping->CreateBinding("Forward", KEY::W)->stringStopAction = "StopForward";
	mapping->CreateBinding("Backward", KEY::S)->stringStopAction = "StopBackward";
	mapping->CreateBinding("Left", KEY::A)->stringStopAction = "StopLeft";
	mapping->CreateBinding("Right", KEY::D)->stringStopAction = "StopRight";

	mapping->CreateBinding("SwitchCamera", KEY::C); 

}

void HorrorGameState::CreateUserInterface()
{
	if (this->ui)
		delete ui;
	this->ui = new UserInterface();
	ui->Load("gui/HUD.gui");
}



// Clear world. Spawn character. 
void HorrorGameState::NewGame()
{
	// Set loading texture.
	Graphics.QueueMessage(new GMSetOverlay("loading_map", 0));
	// Clear everything first.
	MapMan.DeleteEntities();
	numRooms = 0;

	// Reset le random!
	int64 currentTime = Timer::GetCurrentTimeMicro();
	srand(time(&currentTime));

	// Create initial rooms
	for (int x = -5; x <= 5; ++x)
	{
		for (int z = -5; z <= 5; ++z)
		{
			SpawnRoom(Vector2i(x, z), Vector3f(x * roomSize, 0, z * roomSize));
		}
	}

	// reset input
	forward = backward = left = right = false;
	// Set ambient lighting.
	float intensity = 0.001f;
	Graphics.QueueMessage(new GMSetAmbience(Vector4f(intensity, intensity, intensity,1.0f))); 
	// Set gravity
	Physics.QueueMessage(new PMSetGravity(Vector3f(0, -9.82f, 0)));
	Physics.QueueMessage(new PMSet(LINEAR_DAMPING, 0.1f));
	Physics.QueueMessage(new PMSet(ANGULAR_DAMPING, 0.1f));

	player = EntityMan.CreateEntity(ModelMan.GetModel("sphere.obj"), TexMan.GetTexture("Blue"));
	player->position = Vector3f(0,3,0);
	player->scale = Vector3f(1,1,1) * 0.5f;
	player->physics = new PhysicsProperty();
	player->physics->physicsShape = PhysicsShape::SPHERE;
	player->physics->SetMass(70.f);
//	player->physics->useQuaternions = true;
	MapMan.AddEntity(player);
	assert(player);
	// Add a light-source to the player.
	Light * playerLight = new Light();
	playerLight->type = LightType::POINT;
	float lightIntensity = 1.f;
	playerLight->attenuation = Vector3f(1.0f, 0.101f, 0.01f);
	playerLight->specular = 0.1f * (playerLight->diffuse = Vector3f(lightIntensity,lightIntensity,lightIntensity));
	playerLight->spotCutoff = 45.0f;
	playerLight->spotExponent = 20;
	playerLight->data = player;
	Graphics.QueueMessage(new GMAddLight(player, playerLight));
	Physics.QueueMessage(new PMSetEntity(PHYSICS_TYPE, player, PhysicsType::DYNAMIC));
	Physics.QueueMessage(new PMApplyImpulse(player, Vector3f(0, -1.f, 0), player->position));

	// Bind camera to the entity.
	OnCameraUpdated();
	// Play music..
	Track * track = TrackMan.PlayTrack("music/2014-05-03_Despair.ogg");
	if (track)
		track->Loop(true);
	// Set loading texture.
	Graphics.QueueMessage(new GMSetOverlay("NULL", 1000));
}

// New room! o-o
void HorrorGameState::SpawnRoom(Vector2i index, Vector3f atLocation)
{
	std::cout<<"\nCreating room at location: "<<atLocation;



	Entity * room = EntityMan.CreateEntity(ModelMan.GetModel(ROOM_NAME), TexMan.GetTexture("Grey"));
	if (!room->physics)
		room->physics = new PhysicsProperty();
	room->scale *= scale;
	room->SetPosition(atLocation);
	room->physics->physicsShape = PhysicsShape::MESH;
	room->physics->type = PhysicsType::STATIC;
	// Set infinite mass.
	room->physics->inverseMass = 0;
	room->name = "Room x="+String::ToString(index.x)+" z="+String::ToString(index.y);
	// Finally register it for active physics n shit.
	MapMan.AddEntity(room);

	// Add chance for adding random objects in the corners?
	if (numRooms > 5)
	{
		switch(rand() % 10)
		{
			// Room with flare--.!
			case 0:
			{
				Texture * tex = TexMan.GetTexture("Grey");
				switch(rand()%5)
				{
					case 0: tex = TexMan.GetTexture("Red"); break;
				}
				Entity * object = EntityMan.CreateEntity(ModelMan.GetModel("horror/Pedastal"), tex);
				Vector3f position = atLocation;
				switch(rand()%3)
				{
					case 0:	position.x += 3.f; break;
					case 1: position.x -= 3.f; break;
					
				}
				switch(rand()%3)
				{
					case 0:	position.z += 3.f; break;
					case 1:	position.z -= 3.f; break;
				}
				object->SetPosition(position);
				MapMan.AddEntity(object);
				Physics.QueueMessage(new PMSetEntity(PHYSICS_SHAPE, object, PhysicsShape::MESH));
				// Add a light-source to the player.
				Light * pedastalLight = new Light();
				pedastalLight->type = LightType::POINT;
				float lightIntensity = 1.f;
				pedastalLight->attenuation = Vector3f(1.0f, 0.101f, 0.01f);
				Vector3f color =  Vector3f( (rand()%255) / 255.f, (rand()%255) / 255.f, (rand()%255) / 255.f);
				pedastalLight->specular = pedastalLight->diffuse = color;
				pedastalLight->spotCutoff = 45.0f;
				pedastalLight->spotExponent = 20;
				pedastalLight->data = object;
				// Relative a bit up, so it's where it's supposed to be.
				pedastalLight->position = Vector3f(0,2,0);
				Graphics.QueueMessage(new GMAddLight(object, pedastalLight));

				break;
			}

		}
	}
	

	numRooms++;
}


// Updates camera depending on type.
void HorrorGameState::OnCameraUpdated()
{
	switch(cameraType)
	{
	case GLOBAL:
		Graphics.QueueMessage(new GMTrack(NULL));
		break;
	case TRACK_PLAYER:
		Graphics.QueueMessage(new GMTrack(player));
		Graphics.QueueMessage(new GMSetCamera(RELATIVE_POSITION, Vector3f(0, 0.5f, 0)));
		Graphics.QueueMessage(new GMSetCamera(DISTANCE_FROM_CENTER_OF_MOVEMENT, 1.5f));
		Graphics.QueueMessage(new GMSetCamera(OFFSET_ROTATION, Vector3f(0.3f, 0, 0)));
		break;
	}
}




