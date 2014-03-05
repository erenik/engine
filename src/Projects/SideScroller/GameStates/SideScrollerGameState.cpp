/// Emil Hedemalm
/// 2014-03-04
/// Base game state for a side-scroller game, that delivers camera functions and stuff.

#include "SideScrollerGameState.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Input/Keys.h"

Camera * SideScrollerGameState::mainCamera = NULL;



/// Creates camera key-bindings! :)
void SideScrollerGameState::CreateCameraBindings()
{
	inputMapping.CreateBinding("Up", KEY::W)->stringStopAction = "StopUp";
	inputMapping.CreateBinding("Down", KEY::S)->stringStopAction = "StopDown";
	inputMapping.CreateBinding("Left", KEY::A)->stringStopAction = "StopLeft";
	inputMapping.CreateBinding("Right", KEY::D)->stringStopAction = "StopRight";
	
	inputMapping.CreateBinding("Zoom in", KEY::PG_DOWN)->activateOnRepeat = true;
	inputMapping.CreateBinding("Zoom out", KEY::PG_UP)->activateOnRepeat = true;

	inputMapping.CreateBinding("ResetCamera", KEY::HOME);
	inputMapping.CreateBinding("SetProjection", KEY::F1);
	inputMapping.CreateBinding("SetOrthogonal", KEY::F2);
}
/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
bool SideScrollerGameState::HandleCameraMessages(String message)
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
	else if (message == "StopUp")
		mainCamera->End(Direction::UP);
	else if (message == "StopDown")
		mainCamera->End(Direction::DOWN);
	else if (message == "StopLeft")
		mainCamera->End(Direction::LEFT);
	else if (message == "StopRight")
		mainCamera->End(Direction::RIGHT);
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
void SideScrollerGameState::SetCameraTrackEntity(Entity * entity)
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->entityToTrack = entity;
}

void SideScrollerGameState::SetCameraFreeFly()
{
	CreateCamera();
	MakeCameraActive();
	mainCamera->entityToTrack = NULL;
}

void SideScrollerGameState::SetCameraOrthogonal()
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

void SideScrollerGameState::SetCameraProjection3D()
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
void SideScrollerGameState::CreateCamera()
{
	if (!mainCamera){
		mainCamera = new Camera();
		ResetCamera();
		SetCameraOrthogonal();
	}
}

void SideScrollerGameState::MakeCameraActive()
{
	mainCamera->SetRatio(Graphics.width, Graphics.height);
	/// Make the camera active if not already.
	Graphics.QueueMessage(new GMSet(MAIN_CAMERA, mainCamera));
}

/// Resets velocities and stuff
void SideScrollerGameState::ResetCamera()
{
	mainCamera->flySpeed = 3.0f;
}