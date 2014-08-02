/// Emil Hedemalm
/// 2014-05-18
/// Messages for adjusting parameters for the active camera.

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
#include "../Camera/Camera.h"

// A sub-space of the main graphics targets as defined in GraphicsMessages.h
enum cameraTargets
{
	CT_CAMERA_TO_TRACK = GT_CAMERA_TARGET_0, 

	// How to handle all variables provided.
	CT_TRACKING_MODE,

	// Setting global camera
	// Setting camera position and orientation.
	CT_ROTATION,
	CT_POSITION,
	CT_OFFSET_ROTATION,

	/** Relative position of the camera compared to the entity. 
		If the camera is inheriting the entity's rotation, this relative position will be re-calculated using the entity's local orientation axes.
	*/
	CT_RELATIVE_POSITION,
	CT_RELATIVE_POSITION_Y,
	CT_DISTANCE_FROM_CENTER_OF_MOVEMENT,
	/// Speed by with the DfCoM changes.
	CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED_MULTIPLIER,
	CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED,
	/// 
	CT_ROTATION_SPEED_YAW,
	CT_ROTATION_SPEED_PITCH,
	/// o-o
	CT_ZOOM_SPEED,

	CT_ENTITY_TO_TRACK,

	// When true, the camera inherits rotation from the entity it is tracking. Default enabled, but may be adjusted via Camera::defaultInheritEntityRotation
	CT_INHERIT_ROTATION, 
//	GLOBAL_POSITION_OFFSET, // Global position offset, in contrast to the CT_RELATIVE_POSITION which takes into consideration the entity's current orientation.
};

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
	GMSetCamera(Camera * camera, int target, Vector3f vec3fValue);
	GMSetCamera(Camera * camera, int target, int iValue);
	GMSetCamera(Camera * camera, int target, float fValue);
	GMSetCamera(Camera * camera, int target, bool bValue);
	GMSetCamera(Camera * camera, int target, Entity * entity);
	virtual void Process();

private:
	int target;
	int iValue;
	float fValue;
	bool bValue;
	Vector3f vec3fValue;
	Camera * camera;
	Window * window;
	Entity * entity;
};