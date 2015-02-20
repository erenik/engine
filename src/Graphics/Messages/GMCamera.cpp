/// Emil Hedemalm
/// 2014-06-13
/// Messages for adjusting parameters for the active camera.

#include "GMCamera.h"
#include "Graphics/GraphicsManager.h"
#include "Window/WindowManager.h"
#include "Viewport.h"

void GMTrack::Process()
{
	Graphics.ActiveCamera()->entityToTrack = entity;
};

GMDeleteCamera::GMDeleteCamera(Camera * camera)
: GraphicsMessage(GM_DELETE_CAMERA), camera(camera)
{
}

void GMDeleteCamera::Process()
{
	CameraMan.DeleteCamera(camera);
}

/// For setting global/main camera to track.
GMSetCamera::GMSetCamera(Camera * cameraToTrack, Window * inWindow)
	: GraphicsMessage(GM_SET_CAMERA), camera(cameraToTrack), window(inWindow)
{
	target = CT_CAMERA_TO_TRACK;
}

GMSetCamera::GMSetCamera(Camera * camera, int target, ConstVec3fr vec3fValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), vec3fValue(vec3fValue)
{
	assert(camera);
	switch(target)
	{
		case CT_POSITION:
		case CT_ROTATION:
		case CT_RELATIVE_POSITION:
		case CT_OFFSET_ROTATION:
		case CT_TRACKING_POSITION_OFFSET:
		case CT_VELOCITY:
			break;
		default:
			assert(false);
	}
};

GMSetCamera::GMSetCamera(Camera * camera, int target, int iValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), iValue(iValue)
{
	assert(camera);
	switch(target)
	{
		case CT_TRACKING_MODE:
			break;
		case CT_ENTITY_TO_TRACK:
			entity = (Entity*)iValue;
			break;
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT:
			fValue = iValue;
			break;
		default:
			assert(false);
	}
}

GMSetCamera::GMSetCamera(Camera * camera, int target, float fValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), fValue(fValue)
{
	switch(target)
	{
		case CT_RELATIVE_POSITION_Y:
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT:
		case CT_ROTATION_SPEED_YAW:
		case CT_ROTATION_SPEED_PITCH:
		case CT_ZOOM_SPEED:
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED:
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED_MULTIPLIER:
			break;
		default:
			assert(false);
	}
}

GMSetCamera::GMSetCamera(Camera * camera, int target, bool bValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), bValue(bValue)
{
	assert(camera);
	switch(target)
	{
		case CT_INHERIT_ROTATION:
			break;
		default:
			assert(false);
	}
}
	

GMSetCamera::GMSetCamera(Camera * camera, int target, Entity * entity)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), entity(entity)
{
	assert(camera);
	switch(target)
	{
		case CT_ENTITY_TO_TRACK:
			break;
		default:
			assert(false);
	}
}

void GMSetCamera::Process()
{

	// For setting camera attributes
//		if (!camera)
//			camera = Graphics.ActiveCamera();
	switch(target)
	{
		case CT_TRACKING_MODE:
		{
			camera->trackingMode = iValue;
			break;
		}
		case CT_ENTITY_TO_TRACK:
		{
			// Ensure it will actually try and follow it too..?
			camera->entityToTrack = entity;
			if (entity)
				entity->cameraFocus = camera;
			break;
		}
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED:
			camera->dfcomSpeed = fValue;
			break;
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED_MULTIPLIER:
			camera->dfcomSpeedMultiplier = fValue;
			break;
		case CT_ZOOM_SPEED:
			assert(false);
//			camera->zoomSpeed = fValue;
			break;
		case CT_ROTATION_SPEED_YAW:
		{
			camera->rotationalVelocityEuler[1] = fValue;
			break;
		}
		case CT_ROTATION_SPEED_PITCH:
		{
			camera->rotationalVelocityEuler[0] = fValue;
			break;
		}
		case CT_VELOCITY:
			camera->velocity = vec3fValue;
			break;
		case CT_POSITION:
		{
			camera->position = vec3fValue;
			break;
		}
		case CT_TRACKING_POSITION_OFFSET:
			camera->trackingPositionOffset = vec3fValue;
			break;
		case CT_ROTATION:
		{
			camera->rotation = -vec3fValue;
			// If using quaternions, should supply one as argument...
			break;
		}
		case CT_INHERIT_ROTATION:
		{
			camera->inheritEntityRotation = bValue;
			break;
		}
		case CT_CAMERA_TO_TRACK:
		{
			if (!window)
				window = WindowMan.MainWindow();
			Viewport * vp = window->MainViewport();
			// Notify the camera that it lost camera focus.
			if (vp->camera)
				vp->camera->OnLoseCameraFocus();
			vp->camera = camera;
			if (vp->camera)
				vp->camera->OnGainCameraFocus();
			break;
		}
		case CT_OFFSET_ROTATION:
			camera->offsetRotation = -vec3fValue;
			break;
		case CT_RELATIVE_POSITION_Y:
			camera->relativePosition[1] = fValue;
			break;
		case CT_RELATIVE_POSITION:
			camera->relativePosition = vec3fValue;
			break;
		case CT_DISTANCE_FROM_CENTER_OF_MOVEMENT:
			camera->distanceFromCentreOfMovement = fValue;
			break;
		default:
			assert(false);
	}
}
