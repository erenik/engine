// Emil Hedemalm
// 2013-07-19
/// Camera class, required for rendering.

#ifndef CAMERA_H
#define CAMERA_H

#include "Direction.h"
#include "MathLib.h"
//#include "PhysicsLib.h"
#include <String/AEString.h>
#include "Time/Time.h"
#include "PhysicsLib/Shapes/Frustum.h"

class Window;

/// Definitions for eased access in navigation for all states.
#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)

namespace TrackingMode {
enum trackingMode {
	FROM_BEHIND, 
	FIRST_PERSON,
	THIRD_PERSON,
	FOLLOW_AND_LOOK_AT, // Should be smooth. MMORPG-style.
	ADD_POSITION, // Adds position onto camera position.. only.
};};

/// Setups for calculating camera matrices. (mainly the view matrix)
namespace CameraMatrixCalculation {
enum {
	DEFAULT_EDITOR,	/// Default editor o.o, see Camera::CalculateDefaultEditorMatrices

};
};

class Entity;
class Camera;

#define CameraMan (*CameraManager::Instance())

class CameraManager 
{
	CameraManager();
	~CameraManager();
	static CameraManager * cameraManager;
public:
	static void Allocate();
	static void Deallocate();
	static CameraManager * Instance();
	Camera * NewCamera(String name);
	// Called from render/physics thread. updates movement/position of all cameras.
	void Process();
	
	/// Returns the currently active camera. This assumes a main camera being used in the main window.
	Camera * ActiveCamera();

	/** Makes active the next camera (compared to the current one) by queueinga message to the graphics-manager.
		Assumes only 1 active camera is being used for 1 main viewport or window.
	*/
	Camera * NextCamera();
	/** Makes active the previous camera (compared to the current one) by queueinga message to the graphics-manager.
		Assumes only 1 active camera is being used for 1 main viewport or window.
	*/
	Camera * PreviousCamera();
	Camera * DefaultCamera();

	/// Lists all cameras to standard output
	void ListCameras();

	
private:
	Camera * defaultCamera;
	List<Camera*> cameras;
};

enum {
	CAMERA_MOVEMENT_RELATIVE, 
	CAMERA_MOVEMENT_ABSOLUTE
};

class Camera 
{
	friend class CameraManager;
	friend class GMSetCamera;
	/// Default constructor, sets some variables
	Camera();
	~Camera();
public:
	/// Resets everything.
	void Nullify();
	// Prints data, including position, matrices, etc.
	void PrintData() const;

	/// Recalculates projection matrix only.
	void UpdateProjectionMatrix();
	/// Updates all components of the view-matrix. Calls Track() for tracking modes.
	void UpdateViewMatrix(bool track = true);
	/// Extracts camera position, lookAt, etc. from the current matrices.
	void ExtractData();

	/// Resets values to some pre-defined values stored in the resetCamera member.
	void Reset();
	Camera * resetCamera;

	/// For de-coupling bindings to any relevant entities.
	void OnLoseCameraFocus();
	/// For coupling bindings to relevant entities.
	void OnGainCameraFocus();

	/** When called, will set the adjustProjectionMatrixToWindow variable to true and adjust the projection matrix
		To map each pixel in the screen to one unit in-game.
		Regular projection matrices use width and height ratios based on > 1.0f, e.g. 1.666 and 1.0
	*/
	void AdjustProjectionMatrixToWindow(Window * window);

	/// Name of the camera, can be nice to know.
	String name;

	/// Defines viewing distance to active object. Should be near 0 if first-person, larger if 3rd-person.
#define distanceFromCentreOfMovement distanceFromCenterOfMovement
	float distanceFromCenterOfMovement;
	/// Center of Orientation in the world.
	Vector3f position;
	/// Defines where we are looking relative to z = -1
	Vector3f rotation;
	/// Fly-speed in units (meters) per second.
	float flySpeed;
	float rotationSpeed;
	/// For when doing adjustments after all regular matrix operations?
	float elevation;
	float smoothness;
	
	// Default CAMERA_MOVEMENT_RELATIVE, see enum above.
	int movementType;
	// If movementType is CAMERA_MOVEMENT_ABSOLUTE, these vectors are used.
	Vector3f absRight, absUp, absForward;

	/// Current travel speed
	Vector3f velocity;
	/// Current rotational speed
	Vector3f rotationVelocity;
	/// Pitch, yaw, roll. Used to distinguish what kind of rotations are being used, and specifically/more importantly: in which order.
	Vector3f rotationalVelocityEuler;
	/** To distinguish it from the general "rotation" earlier which is.. idk.
		If tracking an entity, these angles will be in relation to the entity, while if not these wil be along the global axises.
	*/
	Vector3f rotationEuler;
	/// The quaternion as built up by the 3 rotations as specified by rotationEuler.
	Quaternion orientationEuler;

	/// To get a "speedy" effect when following entities.
	bool scaleDistanceWithVelocity;
	/// For faster scrolling when zoomed out e.g. in editors.
	bool scaleSpeedWithZoom;
	/// Reverts various attributes depending on camera type.
	bool revert;

	enum cameraTypes {
		NULL_TYPE, PROJECTION_3D, ORTHOGONAL,
	};
	int projectionType;

	// Active camera variables
	/** Multiplied to all edges in the projectionMatrix, the zoom regulates how much content will be visible. 
		A value of 1 is default, while a lower value will zoom in on smaller details, and higher values will allow more content to be visible.
	*/
	float zoom;		
	float farPlane;		// Defines distance of the farplane.
	float nearPlane;	// Defines distance to the nearplane. Should probably be quite low?
	/// Begin moving in the specified direction
	void Begin(int direction);
	/// Stop moving in the specified direction
	void End(int direction);
	/// Begin rotating in the specified direction
	void BeginRotate(int direction);
	/// Stop rotating in the specified direction
	void EndRotate(int direction);
	/// Updates the camera's Matrices, Frustum, and other variables according to new position/rotation, etc.
	void Update();

	/** Sets width/height ratio (commonly known as Aspect ratio). Values should span from 1.0 and upward. */
	void SetRatioF(float widthRatio, float heightRatio, bool force = false);
	/** Sets width/height ratio using screen size (integers for amount of pixels in length). */
	void SetRatioI(int widthRatio, int heightRatio);

	/// Returns the most recently updated projection matrix for this camera
	const Matrix4d Projection() { return projectionMatrix; };
	/// Returns the most recently updated view matrix for this camera
	const Matrix4d View() { return viewMatrix; };

	const Matrix4f ViewProjectionF();
	/// Returns a copy of the frustum
	Frustum GetFrustum() const {return frustum; };

	/** To be called from render/physics-thread. Moves the camera using it's given enabled directions and velocities.
		Returns true if it moved.
	*/
	bool ProcessMovement(float timeInSeconds);
	/// Last time processMovement was called.
	long long lastMovement;

	///////////////////////////////////////////////////////////////////
	// Global value functions!
	///////////////////////////////////////////////////////////////////
	/** Returns the actual position of the camera in the world,
		taking into consideration distance from center of movement
		(the private position) among others.
	*/
	Vector3f Position() const { return camPos; };
	/// Returns the normalized(xyz) vector of the direction the camera is facing.
	Vector4f LookingAt() const { return lookingAtVector; };
	/// Returns the normalized(xyz) vector of the up vector of the camera.
	Vector4f UpVector() const { return upVector; };
	/// Local left-coordinate of the camera. Note that this will not correspond to the global vector if tracking an entity or other unit.
	Vector4f LeftVector() const { return leftVector; };

    /// Returns the width of the camera's nearplane.
	float Width() const;
	/// Returns the height of the camera's nearplane.
	float Height() const;
	/*
	/// Global left-vector of the camera, with applied view-matrix.
	Vector4f GlobalLeftVector() const { return globalLeftVector; };
	/// Global lookAt-vector of the camera, with applied view-matrix.
	Vector4f GlobalLookAtVector() const { return globalLookAtVector; };
	/// Wosh. D:
	Vector4f GlobalUpVector() const { return globalUpVector; };
*/
	/// Returns the view matrix currently used by the camera.
	Matrix4d ViewMatrix4d() { return viewMatrix; };
	/// Returns the view matrix currently used by the camera.
	Matrix4f ViewMatrix4f() { return viewMatrix; };
	/// Returns the projection matrix currently used by the camera.
	Matrix4d ProjectionMatrix4d() { return projectionMatrix; };
	/// Returns the projection matrix currently used by the camera.
	Matrix4f ProjectionMatrix4f() { return projectionMatrix; };

	/// For following camera's!
	Entity * entityToTrack;
	/// Tracking mode o-o;
	int trackingMode;

	/// Screen-to-world space functions, defined by input variables.
	/** Returns a ray in 3D space using the given mouse and camera data.
		Mouse coordinates are assumed to be in screen-pixel space (i.e. 0 to 799 or similar)
	*/
	Ray GetRayFromScreenCoordinates(Window * window, int mouseX, int mouseY) const;

	/// When tracking entities, may add this to the position of the entity (E.g. making it track the head of a character).
	Vector3f trackingPositionOffset;


	Vector3f offsetRotation;
	Vector3f relativePosition;

	/// For when tracking the entity.
	float minTrackingDistance;
	float maxTrackingDistance;

private:
	/// cool.
	Time lastUpdate;

	// Tracks the entity based on given mode.
	void Track();

	/** Calculates a view transform based on the notion of having 
		a distance from center of movement (as in 3D-modelling programs), 
		a rotation aroud the same point, and translate the point based on position and relative position summed up.
	*/
	static bool CalculateDefaultEditorMatrices(float distanceFromCenterOfMovement, Vector2f rotationXY, const Vector3f & worldSpacePosition,
		Matrix4d & viewMatrix, Matrix4d & rotationMatrix);

	/// World position, modulated by tracking and follow- functions, before being used in CalculateDefaultEditorMatrices function.
	Vector3f positionWithOffsets;

	// Velocities! :D
	float dfcomSpeed, dfcomSpeedMultiplier;

	/// See the function with same name.
	bool adjustProjectionMatrixToWindow;
	Window * windowToTrack;

	/// Ratio of the display device/context. Both should be at least 1.0, with the other scaling up as needed.
	float widthRatio, heightRatio;
	/// Real-world position-, looking at- and up-vector.
	Vector3f camPos, lookingAtVector, upVector, leftVector;
	Vector3f globalLeftVector, globalLookAtVector, globalUpVector;

	/// Projection matrix for this camera
	Matrix4f projectionMatrix;
	/// View matrix for this camera.
	Matrix4f viewMatrix;
	/// The rotation-part of the view-matrix!
	Matrix4f rotationMatrix;

	/// If forced once, ratio is fixed.
	bool ratioFixed;

	/// Frustum constrained by this camera.
	Frustum frustum;
	/// Updates base velocities depending on navigation booleans
	void UpdateNavigation();

	/// Boolean array for manual camera control (6 axis)
	bool navigationControls[6];		// For movement
	bool orientationControls[6];	// For rotation

	/// Default values
	static float defaultVelocity;
	static float defaultRotationSpeed;


	// For setting defaults applied to all cameras. Adjust as needed in SetApplicationDefaults or set the property manually for each camera.
	static bool defaultInheritEntityRotation;
	/// If true, inherits entity rotation while tracking it.
	bool inheritEntityRotation;
	/// o=-o
	bool useQuaternions;
	/// o-o 
	Quaternion orientation;
};

/// Screen-to-world space functions, defined by input variables.
/** Returns a ray in 3D space using the given mouse and camera data.
	Mouse coordinates are assumed to be in screen-pixel space (i.e. 0 to 799 or similar)
*/
// Ray GetRayFromScreenCoordinates(int mouseX, int mouseY, Camera& camera);


#endif
