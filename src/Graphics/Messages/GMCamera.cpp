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

GMSetCamera::GMSetCamera(int target, Vector3f vec3fValue)
: GraphicsMessage(GM_SET_CAMERA), target(target), vec3fValue(vec3fValue)
{

};

GMSetCamera::GMSetCamera(int target, float fValue)
: GraphicsMessage(GM_SET_CAMERA), target(target), fValue(fValue)
{

}

void GMSetCamera::Process()
	{
		// For setting camera
		switch(target)
		{
			case CAMERA_TO_TRACK:
			{
				if (!window)
					window = WindowMan.MainWindow();
				Viewport * vp = window->MainViewport();
				vp->camera = camera;
				break;
			}
		}
		// For setting camera attributes
		if (!camera)
			camera = Graphics.ActiveCamera();
		switch(target)
		{
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