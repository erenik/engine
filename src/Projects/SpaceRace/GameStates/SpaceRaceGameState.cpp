/// Emil Hedemalm
/// 2014-02-02
/// Middle-class that provides general functionality like retrieving active players via the game session, etc.

#include "SpaceRaceGameState.h"
#include "Network/NetworkManager.h"
#include "Network/Session/SessionTypes.h"
#include "Network/Session/GameSessionTypes.h"
#include "Input/Keys.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"

/// Retrieves the active gaming session (this assumes only one is active at a time).
SRSession * SpaceRaceGameState::GetSession()
{
	/// Gets session from network-manager. If no session exists, it will create one.
	Session * s = NetworkMan.GetSession(SessionType::GAME, GameSessionType::SPACE_RACE);
	assert(s);
	return (SRSession*)s;
}

/// 
SRPlayer* SpaceRaceGameState::GetPlayer(int byIndex)
{
	SRSession * s = GetSession();
	assert(s);
	return s->GetPlayer(byIndex);
}

/// Gets index for target player.
int SpaceRaceGameState::GetPlayerIndex(SRPlayer * player)
{
	List<SRPlayer*> players = GetPlayers();
	for (int i = 0; i <  players.Size(); ++i){
		if (players[i] == player)
			return i;
	}
	return -1;
}

/// Retrieves a list of active players.
List<SRPlayer*> SpaceRaceGameState::GetPlayers()
{
	SRSession * s = GetSession();
	assert(s && "No valid session to fetch players from D:");
	return s->GetPlayers();	
}

/// Returns list of all local players.
List<SRPlayer*> SpaceRaceGameState::GetLocalPlayers()
{
	SRSession * s = GetSession();
	assert(s && "No valid session to fetch players from D:");
	return s->GetLocalPlayers();	
}





Camera * SpaceRaceGameState::mainCamera = NULL;

/// Creates camera key-bindings! :)
void SpaceRaceGameState::CreateCameraBindings()
{
	inputMapping.CreateBinding("Up", KEY::E)->stringStopAction = "StopUp";
	inputMapping.CreateBinding("Down", KEY::Q)->stringStopAction = "StopDown";
	inputMapping.CreateBinding("Left", KEY::A)->stringStopAction = "StopLeft";
	inputMapping.CreateBinding("Right", KEY::D)->stringStopAction = "StopRight";
	inputMapping.CreateBinding("Forward", KEY::W)->stringStopAction = "StopForward";
	inputMapping.CreateBinding("Backward", KEY::S)->stringStopAction = "StopBackward";

	inputMapping.CreateBinding("Zoom in", KEY::PG_DOWN)->activateOnRepeat = true;
	inputMapping.CreateBinding("Zoom out", KEY::PG_UP)->activateOnRepeat = true;

	inputMapping.CreateBinding("ResetCamera", KEY::HOME);
	inputMapping.CreateBinding("SetProjection", KEY::F1);
	inputMapping.CreateBinding("SetOrthogonal", KEY::F2);
}
/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
bool SpaceRaceGameState::HandleCameraMessages(String message)
{
	if (mainCamera == NULL)
		return false;
	if (message == "Up")
		mainCamera->Begin(Direction::UP);
	else if (message == "Down")
		mainCamera->Begin(Direction::DOWN);
	else if (message == "Left")
		mainCamera->Begin(Direction::LEFT);
	else if (message == "Right")
		mainCamera->Begin(Direction::RIGHT);
	else if (message == "Forward")
		mainCamera->Begin(Direction::FORWARD);
	else if (message == "Backward")
		mainCamera->Begin(Direction::BACKWARD);
	else if (message == "StopUp")
		mainCamera->End(Direction::UP);
	else if (message == "StopDown")
		mainCamera->End(Direction::DOWN);
	else if (message == "StopLeft")
		mainCamera->End(Direction::LEFT);
	else if (message == "StopRight")
		mainCamera->End(Direction::RIGHT);
	else if (message == "StopForward")
		mainCamera->End(Direction::FORWARD);
	else if (message == "StopBackward")
		mainCamera->End(Direction::BACKWARD);
	else if (message == "ResetCamera"){
		mainCamera->position = Vector3f();
	}
	else if (message == "SetProjection")
		SetCameraProjection3D();
	else if (message == "SetOrthogonal")
		SetCameraOrthogonal();
	else if (message == "Zoom in")
	{
		mainCamera->zoom = mainCamera->zoom * 0.95f - 0.01f;
#define CLAMP_DISTANCE Clamp(mainCamera->zoom, 0.01f, 10000.0f);
		CLAMP_DISTANCE;
	}
	else if (message == "Zoom out"){
		mainCamera->zoom = mainCamera->zoom * 1.05f + 0.01f;
		CLAMP_DISTANCE;
	}
	else
		return false;
	return true;
}


/// Modes for the camera. Creates and makes the camera active if it wasn't already.
void SpaceRaceGameState::SetCameraTrackEntity(Entity * entity)
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->entityToTrack = entity;
}

void SpaceRaceGameState::SetCameraFreeFly()
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->entityToTrack = NULL;
}

void SpaceRaceGameState::SetCameraOrthogonal()
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->projectionType = Camera::ORTHOGONAL;
	mainCamera->entityToTrack = NULL;
	mainCamera->rotation = Vector3f();
	mainCamera->distanceFromCentreOfMovement = -5;
	/// Allow for some objects to be seen...
	mainCamera->zoom = 5.0f;
	mainCamera->scaleSpeedWithZoom = true;
	mainCamera->flySpeed = 1.f;
	mainCamera->position = Vector3f();
}

void SpaceRaceGameState::SetCameraProjection3D()
{
	CreateCamera();
	MakeCameraActive();
	/// Normal zoom, please..
	mainCamera->zoom = 0.1f;
	mainCamera->scaleSpeedWithZoom = false;
	mainCamera->flySpeed = 3.0f;
	mainCamera->projectionType = Camera::PROJECTION_3D;
}

/// Creates the camera.
void SpaceRaceGameState::CreateCamera()
{
	if (!mainCamera){
		mainCamera = new Camera();
		ResetCamera();
		SetCameraOrthogonal();
	}
}

void SpaceRaceGameState::MakeCameraActive()
{
	mainCamera->SetRatio(Graphics.width, Graphics.height);
	/// Make the camera active if not already.
	Graphics.QueueMessage(new GMSet(MAIN_CAMERA, mainCamera));
}

/// Resets velocities and stuff
void SpaceRaceGameState::ResetCamera()
{
	mainCamera->flySpeed = 3.0f;
}