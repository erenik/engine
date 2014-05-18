/// Emil Hedemalm
/// 2014-05-18
/// Messages for adjusting parameters for the active camera.

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
#include "../Camera/Camera.h"

class GMTrack : public GraphicsMessage
{
public:
	GMTrack(Entity * entity) 
		: GraphicsMessage(GM_TRACK), entity(entity)
	{		
	}
	virtual void Process()
	{
		Graphics.ActiveCamera()->entityToTrack = entity;
	};
private:
	Entity * entity;
};

class GMSetCamera : public GraphicsMessage 
{
public:
	GMSetCamera(int target, Vector3f vec3fValue) 
		: GraphicsMessage(GM_SET_CAMERA), target(target), vec3fValue(vec3fValue)
	{};
	GMSetCamera(int target, float fValue)
		: GraphicsMessage(GM_SET_CAMERA), target(target), fValue(fValue){}
	virtual void Process()
	{
		Camera * camera = Graphics.ActiveCamera();
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

private:
	int target;
	float fValue;
	Vector3f vec3fValue;
};