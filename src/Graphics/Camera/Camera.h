// Emil Hedemalm
// 2013-07-19
/// Camera class, required for rendering.

#ifndef CAMERA_H
#define CAMERA_H

#include "Direction.h"
#include "MathLib.h"
#include "PhysicsLib.h"
#include <String/AEString.h>

/// Definitions for eased access in navigation for all states.
#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)

namespace TrackingMode {
enum trackingMode {
	FROM_BEHIND, THIRD_PERSON,
};};

class Entity;

class Camera {
public:
	/// Default constructor, sets some variables
	Camera();
	// Prints data, including position, matrices, etc.
	void PrintData() const;

	/// Name of the camera, can be nice to know.
	String name;

	/// Defines viewing distance to active object. Should be near 0 if first-person, larger if 3rd-person.
	float distanceFromCentreOfMovement;
	/// Center of Orientation in the world.
	Vector3f position;
	/// Defines where we are looking relative to z = -1
	Vector3f rotation;
	/// Fly-speed in units (meters) per second.
	float flySpeed;
	float rotationSpeed;
	/// For when doing adjustments after all regular matrix operations?
	float elevation;
	/// Current travel speed
	Vector3f velocity;
	/// Current rotational speed
	Vector3f rotationVelocity;
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
	void SetRatio(float widthRatio, float heightRatio);
	/** Sets width/height ratio using screen size (integers for amount of pixels in length). */
	void SetRatio(int widthRatio, int heightRatio);

	/// Returns the most recently updated projection matrix for this camera
	const Matrix4d Projection() { return projectionMatrix; };
	/// Returns the most recently updated view matrix for this camera
	const Matrix4d View() { return viewMatrix; };
	/// Returns a copy of the frustum
	Frustum GetFrustum() const {return frustum; };

	/// To be called from render/physics-thread. Moves the camera using it's given enabled directions and velocities.
	void ProcessMovement(float timeSinceLastUpdate);
	/// Last time processMovement was called.
	long long lastMovement;

	///////////////////////////////////////////////////////////////////
	// Global value functions!
	///////////////////////////////////////////////////////////////////
	/** Returns the actual position of the camera in the world,
		taking into consideration distance from center of movement
		(the private position) among others.
	*/
	Vector4f Position() const { return camPos; };
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

	/// For following camera's!
	Entity * entityToTrack;
	/// Tracking mode o-o;
	int trackingMode;

private:

	/// Ratio of the display device/context. Both should be at least 1.0, with the other scaling up as needed.
	float widthRatio, heightRatio;
	/// Real-world position-, looking at- and up-vector.
	Vector4f camPos, lookingAtVector, upVector, leftVector;
	Vector4f globalLeftVector, globalLookAtVector, globalUpVector;

	/// Projection matrix for this camera
	Matrix4d projectionMatrix;
	/// View matrix for this camera.
	Matrix4d viewMatrix;
	/// The rotation-part of the view-matrix!
	Matrix4d rotationMatrix;

	/// Frustum constrained by this camera.
	Frustum frustum;
	/// Updates base velocities depending on navigation booleans
	void UpdateNavigation();
	/// Boolean array for manual camera control (6 axis)
	bool navigation[6];		// For movement
	bool orientation[6];	// For rotation

	/// Default values
	static float defaultVelocity;
	static float defaultRotationSpeed;
};

/// Screen-to-world space functions, defined by input variables.
/** Returns a ray in 3D space using the given mouse and camera data.
	Mouse coordinates are assumed to be in screen-pixel space (i.e. 0.0 to 800.0 or similar)
*/
Ray GetRayFromScreenCoordinates(float& mouseX, float& mouseY, Camera& camera);


#endif
