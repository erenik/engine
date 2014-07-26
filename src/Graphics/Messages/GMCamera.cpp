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

/// For setting global/main camera to track.
GMSetCamera::GMSetCamera(Camera * cameraToTrack, Window * inWindow)
	: GraphicsMessage(GM_SET_CAMERA), camera(cameraToTrack), window(inWindow)
{
	target = CAMERA_TO_TRACK;
}

GMSetCamera::GMSetCamera(Camera * camera, int target, Vector3f vec3fValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), vec3fValue(vec3fValue)
{
	switch(target)
	{
		case RELATIVE_POSITION:
		case OFFSET_ROTATION:
			break;
		default:
			assert(false);
	}
};

GMSetCamera::GMSetCamera(Camera * camera, int target, float fValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), fValue(fValue)
{
	switch(target)
	{
		case DISTANCE_FROM_CENTER_OF_MOVEMENT:
			break;
		default:
			assert(false);
	}
}

GMSetCamera::GMSetCamera(Camera * camera, int target, bool bValue)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), bValue(bValue)
{
	switch(target)
	{
		case INHERIT_ROTATION:
			break;
		default:
			assert(false);
	}
}
	

GMSetCamera::GMSetCamera(Camera * camera, int target, Entity * entity)
: GraphicsMessage(GM_SET_CAMERA), camera(camera), target(target), entity(entity)
{
	switch(target)
	{
		case ENTITY_TO_TRACK:
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
		case ENTITY_TO_TRACK:
		{
			// Ensure it will actually try and follow it too..?
			camera->entityToTrack = entity;
			break;
		}
		case INHERIT_ROTATION:
		{
			camera->inheritEntityRotation = bValue;
			break;
		}
		case CAMERA_TO_TRACK:
		{
			if (!window)
				window = WindowMan.MainWindow();
			Viewport * vp = window->MainViewport();
			vp->camera = camera;
			break;
		}
		case OFFSET_ROTATION:
			camera->offsetRotation = vec3fValue;
			break;
		case RELATIVE_POSITION:
			camera->relativePosition = vec3fValue;
			break;
		case DISTANCE_FROM_CENTER_OF_MOVEMENT:
			camera->distanceFromCentreOfMovement = fValue;
			break;
	}
}
