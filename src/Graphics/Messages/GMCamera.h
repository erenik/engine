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
	virtual void Process();
private:
	Entity * entity;
};

class GMSetCamera : public GraphicsMessage 
{
public:
	/// For setting global/main camera to track. If window is NULL the main window will be selected.
	GMSetCamera(Camera * cameraToTrack, Window * inWindow = NULL);
	GMSetCamera(int target, Vector3f vec3fValue);
	GMSetCamera(int target, float fValue);
	virtual void Process();

private:
	int target;
	float fValue;
	Vector3f vec3fValue;
	Camera * camera;
	Window * window;
};