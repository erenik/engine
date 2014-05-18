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
#include "Message/Message.h"
#include "Input/Keys.h"
#include "Graphics/Camera/Camera.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Entity/EntityManager.h"

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
	cameraType = 0;
	roomSize = 6.0f;
	scale = 2.0f;
}

void HorrorGameState::OnEnter(GameState *)
{
	// We require nice physics... maybe.
	Physics.checkType = OCTREE; // AABB_SWEEP or OCTREE
	Physics.integrator =  Integrator::SPACE_RACE_CUSTOM_INTEGRATOR; // Integrator namespace.
	Physics.collissionResolver = CUSTOM_SPACE_RACE_PUSHBACK; // CUSTOM_SPACE_RACE_PUSHBACK or LAB_PHYSICS_IMPULSES

	Model * roomModel = ModelMan.GetModel(ROOM_NAME);
	roomSize = roomModel->aabb.max.x - roomModel->aabb.min.x;
	roomSize *= scale;


	NewGame();
}

void HorrorGameState::Process(float)
{

	// This check should maybe be optimized somehow later?
	List<Entity*> entities = MapMan.GetEntities();

	// Check player position.
	Vector3f position = player->position;
	int roomX = (position.x + roomSize * 0.5f) / roomSize;
	int roomZ = (position.z + roomSize * 0.5f) / roomSize;
	position.x = roomX;
	position.z = roomZ;

	// Generate list of tiles we should plant.
	for (int x = position.x - 3; x <= position.x + 3; ++x)
	{
		for (int z = position.z - 3; z <= position.z + 3; ++z)
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
#define TURN 12.5f
		if (right && !left)
			Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, player, Vector3f(0, -TURN, 0)));
		else if (left)
			Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, player, Vector3f(0, TURN, 0)));
		else
			Physics.QueueMessage(new PMSetEntity(ANGULAR_ACCELERATION, player, Vector3f()));
#define MOVE_ACC 15.f
		if (forward && !backward)
			Physics.QueueMessage(new PMSetEntity(ACCELERATION, player, Vector3f(0,0,-MOVE_ACC)));
		else if (backward)
			Physics.QueueMessage(new PMSetEntity(ACCELERATION, player, Vector3f(0,0,MOVE_ACC)));
		else
			Physics.QueueMessage(new PMSetEntity(ACCELERATION, player, Vector3f()));

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


// Clear world. Spawn character. 
void HorrorGameState::NewGame()
{

	// reset input
	forward = backward = left = right = false;
	// Set ambient lighting.
	Graphics.QueueMessage(new GMSetAmbience(Vector4f(0.5f,0.5f,0.5f,1.0f))); 

	MapMan.DeleteEntities();
	player = EntityMan.CreateEntity(ModelMan.GetModel("sphere.obj"), TexMan.GetTexture("Blue"));
	player->position = Vector3f(0,10,0);
	player->scale = Vector3f(1,1,1) * 0.5f;
	MapMan.AddEntity(player);
	assert(player);
	// Add a light-source to the player.
	Light * playerLight = new Light();
	playerLight->type = LightType::POINT;
	float lightIntensity = 1.f;
	playerLight->attenuation = Vector3f(1.0f, 0.001f, 0.00001f);
	playerLight->specular = 0.1f * (playerLight->diffuse = Vector3f(lightIntensity,lightIntensity,lightIntensity));
	playerLight->spotCutoff = 45.0f;
	playerLight->spotExponent = 20;
	playerLight->data = player;
	Graphics.QueueMessage(new GMAddLight(player, playerLight));
	Physics.QueueMessage(new PMSetEntity(PHYSICS_TYPE, player, PhysicsType::DYNAMIC));
	Physics.QueueMessage(new PMApplyImpulse(player, Vector3f(0, -1.f, 0), player->position));

	// Create initial room
	for (int x = -3; x <= 3; ++x)
	{
		for (int z = -3; z <= 3; ++z)
		{
			SpawnRoom(Vector2i(x, z), Vector3f(x * roomSize, 0, z * roomSize));
		}
	}

	// Bind camera to the entity.
	OnCameraUpdated();
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
	room->name = "Room x="+String::ToString(index.x)+" z="+String::ToString(index.y);
	// Finally register it for active physics n shit.
	MapMan.AddEntity(room);
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




