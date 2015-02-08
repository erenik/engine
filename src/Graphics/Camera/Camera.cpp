/// Emil Hedemalm
/// 2014-03-05
/// Camera class, required for rendering.

#include "Graphics/Camera/Camera.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Window/Window.h"
#include "Viewport.h"
#include <cstring>

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"

#include "PhysicsLib/Shapes/Ray.h"

CameraManager::CameraManager()
{
	defaultCamera = NULL;
}
CameraManager::~CameraManager()
{
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * camera = cameras[i];
		delete camera;
	}
	cameras.Clear();
}

CameraManager * CameraManager::cameraManager = NULL;
void CameraManager::Allocate()
{
	assert(cameraManager == NULL);
	cameraManager = new CameraManager();
}
void CameraManager::Deallocate()
{
	assert(cameraManager);
	delete cameraManager;
	cameraManager = NULL;
}
CameraManager * CameraManager::Instance()
{
	assert(cameraManager);
	return cameraManager;
}
Camera * CameraManager::NewCamera(String name)
{
	Camera * camera = new Camera();
	camera->name = name;
	cameras.Add(camera);
	return camera;
}
// Called from render/physics thread. updates movement/position of all cameras.
void CameraManager::Process()
{
	static int64 lastUpdate = 0;
	int64 cTime = Timer::GetCurrentTimeMs();
	int timeDiff = cTime - lastUpdate;
	timeDiff = timeDiff % 200;
	lastUpdate = cTime;
	float timeInSeconds = timeDiff * 0.001f;
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * camera = cameras[i];
		camera->ProcessMovement(timeInSeconds);
	}
}

/// Returns the currently active camera. This assumes a main camera being used in the main window.
Camera * CameraManager::ActiveCamera()
{
	Camera * renderingCamera = GraphicsMan.ActiveCamera();
	if (renderingCamera)
		return renderingCamera;
	Window * window = MainWindow();
	if (!window)
		return NULL;
	Viewport * vp = window->MainViewport();
	if (!vp)
		return NULL;
	return vp->camera;
}

/** Makes active the next camera (compared to the current one) by queueinga message to the graphics-manager.
	Assumes only 1 active camera is being used for 1 main viewport or window.
*/
Camera * CameraManager::NextCamera()
{
	Camera * current = Graphics.ActiveCamera();
	int previousIndex = -1;
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * c = cameras[i];
		if (c == current)
			previousIndex = i;
	}
	// Set next one.
	int nextIndex = previousIndex + 1;
	if (nextIndex >= cameras.Size())
		nextIndex = 0;
	if (previousIndex < 0)
		previousIndex = 0;
	Graphics.QueueMessage(new GMSetCamera(cameras[nextIndex]));
	return cameras[nextIndex];
}

/** Makes active the previous camera (compared to the current one) by queueinga message to the graphics-manager.
	Assumes only 1 active camera is being used for 1 main viewport or window.
*/
Camera * CameraManager::PreviousCamera()
{
	Camera * current = Graphics.ActiveCamera();
	int previousIndex = -1;
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * c = cameras[i];
		if (c == current)
			previousIndex = i;
	}
	// Set next one.
	int nextIndex = previousIndex - 1;
	Camera * newCam = NULL;
	if (nextIndex < 0)
		newCam = cameras.Last();
	else 
		newCam = cameras[nextIndex];
	Graphics.QueueMessage(new GMSetCamera(newCam));
	return newCam;
}


Camera * CameraManager::DefaultCamera()
{
	if (!defaultCamera)
		defaultCamera = new Camera();
	cameras.Add(defaultCamera);
	return defaultCamera;
}

/// Lists all cameras to standard output
void CameraManager::ListCameras()
{
	std::cout<<"\n"<<cameras.Size()<<" active cameras";
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * camera = cameras[i];
		std::cout<<"\nCamera "<<camera->name<<", pos "<<camera->position;
	}
}




float Camera::defaultRotationSpeed = 0.09f;
float Camera::defaultVelocity = 1.0f;
bool Camera::defaultInheritEntityRotation = true;

Camera::Camera()
{
	Nullify();
};

Camera::~Camera()
{

}

/// Resets everything.
void Camera::Nullify()
{
	// Tracking vars
	smoothness = 0.2;
	minTrackingDistance = 1.5f;
	maxTrackingDistance = 5.f;

	movementType = CAMERA_MOVEMENT_RELATIVE;
	resetCamera = NULL;
	entityToTrack = NULL;
	trackingMode = TrackingMode::FROM_BEHIND;
	nearPlane = -0.1f;
	farPlane = -100000.0f;
	zoom = 0.1f;
	distanceFromCentreOfMovement = 0.0f;
	position = Vector3f(-10, 10, 20);
	rotation = Vector3f(PI*0.25f, PI*0.125f, 0);
	flySpeed = 1.0f;
	rotationSpeed = 0.1f;
	memset(navigationControls, 0, sizeof(bool)*6);
	memset(orientationControls, 0, sizeof(bool)*6);
	/// Ratio of the display device/context. Both should be at least 1.0, with the other scaling up as needed.
	widthRatio = 1.0f;
	heightRatio = 1.0f;
	// Default the vectors
	upVector = Vector3f(0,1,0);
	lookingAtVector = Vector3f(0,0,-1);
	projectionType = PROJECTION_3D;
	/// wat.
	elevation = 0.0f;
	revert = false;
	scaleDistanceWithVelocity = false;
	scaleSpeedWithZoom = false;

	adjustProjectionMatrixToWindow = false;
	windowToTrack = NULL;

	useQuaternions = false;
	inheritEntityRotation = defaultInheritEntityRotation;

	dfcomSpeedMultiplier = 1.f;
	dfcomSpeed = 0.f;
}


// Prints data, including position, matrices, etc.
void Camera::PrintData() const {
	std::cout<<"\nCamera::PrintData for camera: "<<name;
	std::cout<<"\n - ViewMatrix: "<<viewMatrix;
	std::cout<<"\n - Orthogonal: "<< ((projectionType == ORTHOGONAL)? "yes" : "no");
	std::cout<<"\n - Position: "<<position;
	std::cout<<"\n - Nearplane: "<<nearPlane;
	std::cout<<"\n - Farplane: "<<farPlane;
}

/// Resets values to some pre-defined.. values.
void Camera::Reset()
{
	if (!resetCamera)
		return;
	position = resetCamera->position;
	rotation = resetCamera->rotation;
	flySpeed = resetCamera->flySpeed;
}


/// For de-coupling bindings to any relevant entities.
void Camera::OnLoseCameraFocus()
{
	if (entityToTrack)
		entityToTrack->cameraFocus = NULL;
}
/// For coupling bindings to relevant entities.
void Camera::OnGainCameraFocus()
{
	if (entityToTrack)
		entityToTrack->cameraFocus = this;
}


/** When called, will set the adjustProjectionMatrixToWindow variable to true and adjust the projection matrix
	To map each pixel in the screen to one unit in-game.
	Regular projection matrices use width and height ratios based on > 1.0f, e.g. 1.666 and 1.0
*/
void Camera::AdjustProjectionMatrixToWindow(Window * window)
{
	adjustProjectionMatrixToWindow = true;
	windowToTrack = window;
}

/// Begin moving in the specified direction
void Camera::Begin(int direction)
{
	navigationControls[direction] = true;
	UpdateNavigation();
}
/// Stop moving in the specified direction
void Camera::End(int direction)
{
	navigationControls[direction] = false;
	UpdateNavigation();
}
/// Begin rotating in the specified direction
void Camera::BeginRotate(int direction)
{
	orientationControls[direction] = true;
	UpdateNavigation();
}
/// Stop rotating in the specified direction
void Camera::EndRotate(int direction){
	orientationControls[direction] = false;
	UpdateNavigation();
}

/// Updates the frustum
void Camera::Update()
{
	Time now = Time::Now();
	float milliseconds = (now - lastUpdate).Milliseconds();
	float timeInSeconds = milliseconds * 0.001f;
	
	/// Load identity matrices before we re-calculate them... hmm
	projectionMatrix.LoadIdentity();
	viewMatrix.LoadIdentity();
	rotationMatrix.LoadIdentity();

	// Calculate values to use for projection matrix.
	float left, right, bottom, top;
	// This should probably only be used with orthogonal projection cameras...
	if (adjustProjectionMatrixToWindow)
	{
		Vector2i windowWorkingArea = windowToTrack->WorkingArea();
		if (windowWorkingArea.Length() <= 0 || windowWorkingArea.Length() > 100000)
		{
		std::cout<<"lall";
		}
		float halfWidth = windowWorkingArea[0] * 0.5f;
		float halfHeight = windowWorkingArea[1] * 0.5f;
		right = halfWidth;
		left = -right;
		top = halfHeight;
		bottom = -top;
	}
	else {
		left = -widthRatio;
		right = widthRatio;
		bottom = -heightRatio;
		top = heightRatio;
	};

	left *= zoom;
	right *= zoom;
	bottom *= zoom;
	top *= zoom;

	/// Nothing to create matrix out of...
	if (left == right || bottom == top &&
		left == 0 || bottom == 0)
		return;

	/// Begin by updating projection matrix as that is unlikely to vary with foci
	switch(projectionType)
	{	
		// OLD: Perspective-distortions when adjusting width and height!
		case PROJECTION_3D:
			projectionMatrix.InitProjectionMatrix(left, right, bottom, top, nearPlane, farPlane);
			break;
		case ORTHOGONAL:
			projectionMatrix.InitOrthoProjectionMatrix(left, right, bottom, top, nearPlane, farPlane);
			break;
	}
	frustum.SetProjection(projectionType);


	/// 3rd Person "static-world-rotation" camera
	if (entityToTrack)
	{
		Track();
	}
	else 
		positionWithOffsets = this->position;

	
	/// Apply any after-effects here, such as 'shake', etc.

	/// Calculate matrices.
	/// Add a switch case if new/other techniques are requested -> move code from the TrackBehind, etc. to there.
	viewMatrix = CalculateDefaultEditorMatrices(distanceFromCenterOfMovement, rotation, positionWithOffsets);


	/// Some new temporary variables for updating the frustum
	float sample = viewMatrix.GetColumn(0)[0];
	assert(AbsoluteValue(sample) < 100000.f);
	//	Vector4f camPos, lookingAtVector, upVector;
	camPos = viewMatrix.InvertedCopy() * Vector4d(0,0,0,1);	// THIS

	// Calculate camLook and camUp Vectors
	Vector4d moveVec = Vector4d(0, 0, -1, 1);

	/// Extract global coordinates
	leftVector = rotationMatrix.InvertedCopy().Product(Vector4f(-1,0,0,1)).NormalizedCopy();
	lookingAtVector = rotationMatrix.InvertedCopy().Product(Vector4f(0,0,-1,1)).NormalizedCopy();
	upVector = rotationMatrix.InvertedCopy().Product(Vector4f(0,1,0,1)).NormalizedCopy();

	// Update frustum
	frustum.SetCamInternals(left, right, bottom, top, nearPlane, farPlane);
	frustum.SetCamPos(Vector3f(camPos), Vector3f(lookingAtVector), Vector3f(upVector));

	lastUpdate = now;
}

void Camera::Track()
{
	switch(trackingMode)
	{
		case TrackingMode::THIRD_PERSON:
		case TrackingMode::FOLLOW_AND_LOOK_AT:
		{
			// Get data
			Vector3f entityPosition = entityToTrack->position;
			
			/// The actual position of the camera at the moment.
			Vector3f currentCamPosition = this->Position();
			// Look at it from our new position!
			// Find rotation yaw needed.
			Vector3f toEntity = entityPosition - currentCamPosition;
			Vector2f toEntityXZ(toEntity[0], toEntity[2]);
			toEntityXZ.Normalize();
			float yawNeeded = atan2(toEntityXZ[1], toEntityXZ[0]);
			yawNeeded += PI * 0.5f;

			float xzDistance = Vector2f(toEntity[0],toEntity[2]).Length();
			Vector2f toEntityXY(xzDistance, toEntity[1]);
			toEntityXY.Normalize();
			float pitchNeeded = -atan2(toEntityXY[1], toEntityXY[0]);
		
			// Set rotations accordingly.
			rotation.x = pitchNeeded;
			rotation.y = yawNeeded;
			
			// Far away? get closer?
			// First update toEntity since it will be using the offsetted position from earlier, we want the flat XZ diff.
			toEntity = entityToTrack->position - position;
			toEntity.y = 0;
			float dist = toEntity.Length();
			float relevantTrackingDistance = dist;
			ClampFloat(relevantTrackingDistance, minTrackingDistance, maxTrackingDistance);
			float currentDiff = dist - relevantTrackingDistance;
			float newDiff = currentDiff * (1 - smoothness);
			float newDist = newDiff + relevantTrackingDistance;
			// Reduce the diff.
			position = entityPosition - toEntity.NormalizedCopy() * newDist;

			/// Copy over so calculation works below...
			positionWithOffsets = position + trackingPositionOffset;
			break;
		}
		case TrackingMode::FROM_BEHIND:
		case TrackingMode::FIRST_PERSON:
		{
			// Get data
			Vector3f entityPosition = entityToTrack->position;
			position = entityPosition;
			rotation = entityToTrack->rotation;
			positionWithOffsets = position;
			break;
		}
		case TrackingMode::ADD_POSITION:
		{
			positionWithOffsets = this->position;
			positionWithOffsets += entityToTrack->position;
			break;
		}
	}
}

/** Sets width/height ratio (commonly known as Aspect ratio).
	Ratios should be 1.0 and above, and will be scaled down as needed.
*/
void Camera::SetRatio(float width, float height)
{
	float smallest = (float)height;
	if (height > width)
		smallest = (float)width;
	this->widthRatio = (float) width / smallest;
	this->heightRatio = (float) height / smallest;
}
/** Sets width/height ratio (commonly known as Aspect ratio) using screen size.
*/
void Camera::SetRatio(int width, int height)
{
	float smallest = (float)height;
	if (height > width)
		smallest = (float)width;
	this->widthRatio = (float) width / smallest;
	this->heightRatio = (float) height / smallest;
}

/// To be called from render/physics-thread. Moves the camera using it's given enabled directions and velocities.
void Camera::ProcessMovement(float timeInSeconds)
{
	if (lastMovement == 0){
		lastMovement = Timer::GetCurrentTimeMs();
		return;
	}
	if (velocity.MaxPart() == 0 && rotationalVelocityEuler.MaxPart() == 0 && dfcomSpeedMultiplier == 1.f)
		return;
	Vector3f deltaP = velocity * timeInSeconds;
	/// We might want to calculate the position Diff using local camera co-ordinates..!
	Vector3f rightVec = this->lookingAtVector.CrossProduct(upVector);
	Vector3f totalPosDiff;
	
	Vector3f forward, up, right;
	if (movementType == CAMERA_MOVEMENT_RELATIVE)
	{
		forward = - lookingAtVector;
		up = upVector;
		right = rightVec;
	}
	else
	{
		forward = absForward;
		up = absUp;
		right = absRight;
	}

	totalPosDiff = deltaP[2] * forward +
		deltaP[1] * up +
		deltaP[0] * right;
	
	if (scaleSpeedWithZoom)
	{
		totalPosDiff *= zoom;
	}
	position += totalPosDiff;	

	/// Apply rotation and stuff.

	/// Apply Y-rotation, global axis.
	if (rotationalVelocityEuler.MaxPart())
	{
		rotationEuler += rotationalVelocityEuler * timeInSeconds;
		// Re-calculate the quaternion defining this rotation (if possible).
		Quaternion pitch(Vector3f(1,0,0), rotationEuler[0]);
		Quaternion yaw(Vector3f(0,1,0), rotationEuler[1]);
		Quaternion roll(Vector3f(0,0,1), rotationEuler[2]);

		orientationEuler = yaw * pitch; // * roll;
		// And multiply it!
//		orientation = orientation * rotation;
	}
	/// Apply zoom, multiplicative style!
	if (dfcomSpeedMultiplier != 1.f)
	{
		distanceFromCenterOfMovement *= pow(dfcomSpeedMultiplier, timeInSeconds);			
	}
	// Apply zoom, constant style!
	if (dfcomSpeed)
	{
		distanceFromCenterOfMovement += dfcomSpeed * timeInSeconds;
	}
}

/// Updates base velocities depending on navigationControls booleans
void Camera::UpdateNavigation()
{
	/// Navigation
	if (navigationControls[Direction::UP] && !navigationControls[Direction::DOWN])
		this->velocity[1] = this->defaultVelocity * this->flySpeed;
	else if (navigationControls[Direction::DOWN] && !navigationControls[Direction::UP])
		this->velocity[1] = -this->defaultVelocity * this->flySpeed;
	else
		this->velocity[1] = 0;
	if (navigationControls[Direction::FORWARD] && !navigationControls[Direction::BACKWARD])
		this->velocity[2] = this->defaultVelocity * this->flySpeed;
	else if (navigationControls[Direction::BACKWARD] && !navigationControls[Direction::FORWARD])
		this->velocity[2] = -this->defaultVelocity * this->flySpeed;
	else
		this->velocity[2] = 0;
	if (navigationControls[Direction::LEFT] && !navigationControls[Direction::RIGHT])
		this->velocity[0] = -this->defaultVelocity * this->flySpeed;
	else if (navigationControls[Direction::RIGHT] && !navigationControls[Direction::LEFT])
		this->velocity[0] = this->defaultVelocity * this->flySpeed;
	else
		this->velocity[0] = 0;

	/// Rotation
	if (orientationControls[Direction::UP] && !orientationControls[Direction::DOWN])
		this->rotationVelocity[0] = this->defaultRotationSpeed * this->rotationSpeed;
	else if (orientationControls[Direction::DOWN] && !orientationControls[Direction::UP])
		this->rotationVelocity[0] = -this->defaultRotationSpeed * this->rotationSpeed;
	else
		this->rotationVelocity[0] = 0;
	if (orientationControls[Direction::LEFT] && !orientationControls[Direction::RIGHT])
		this->rotationVelocity[1] = this->defaultRotationSpeed * this->rotationSpeed;
	else if (orientationControls[Direction::RIGHT] && !orientationControls[Direction::LEFT])
		this->rotationVelocity[1] = -this->defaultRotationSpeed * this->rotationSpeed;
	else
		this->rotationVelocity[1] = 0;
};


/// Returns the width of the camera's nearplane.
float Camera::Width() const {
    float width = widthRatio * zoom;
    std::cout<<"\nWidthRatio: "<<widthRatio<<" zoom: "<<zoom<<" width:"<<width;
    return width;
}
/// Returns the height of the camera's nearplane.
float Camera::Height() const {
    float height = heightRatio * zoom;
    std::cout<<"\nHeightRatio: "<<heightRatio<<" zoom: "<<zoom<<" height:"<<height;
    return height;
}

/// Screen-to-world space functions, defined by input variables.
/** Returns a ray in 3D space using the given mouse and camera data.
	Mouse coordinates are assumed to be in screen-pixel space (i.e. 0.0 to 800.0 or similar)
*/
Ray Camera::GetRayFromScreenCoordinates(Window * window, int mouseX, int mouseY) const
{
//	std::cout<<"\nGetRayFromScreenCoordinates\n===============================";
//	std::cout<<"\nMouseX: "<<mouseX<<" MouseY: "<<mouseY;
//    std::cout<<"\nCamera: "<<camera.name;
	const Camera & camera = (*this);
	/// Calculate near plane coordinates
	Vector3f camPosition = camera.Position();
	Vector3f camLookingAt = camera.LookingAt();
	Vector4f nearPlaneCenter = camPosition - camLookingAt * camera.nearPlane;
	Vector4f nearPlaneUpVec = camera.UpVector();
#define upVector nearPlaneUpVec
	Vector4f nearPlaneRightVec = -camera.LeftVector();
#define rightVector nearPlaneRightVec

 //   std::cout<<"\nCamPosition: "<<camPosition<<" LookingAt: "<<camLookingAt;

	/// Lower left corner
	Vector4f nearPlaneLLCorner = nearPlaneCenter - nearPlaneUpVec * camera.GetFrustum().top
		- nearPlaneRightVec * camera.GetFrustum().right;

 //   std::cout<<"\nNearPlaneCenter: "<<nearPlaneCenter<<" NearPlaneLowerLeftcorner: "<<nearPlaneLLCorner;

	Vector2i windowSize = window->WorkingArea();
	// Get relative positions of where we clicketiclicked, from 0.0 to 1.0 (0,0 in lower left corner)
	float clientAreaWidth = (float)windowSize[0];
	float clientAreaHeight = (float)windowSize[1];
	float relativeX = mouseX / clientAreaWidth,
		  relativeY = mouseY / clientAreaHeight;

    float zoom = camera.zoom;
    Frustum frustum = camera.GetFrustum();

 //   relativeX -= 0.5f;
 //   relativeY -= 0.5f;

    Vector4f startPoint = nearPlaneLLCorner + nearPlaneUpVec * relativeY * camera.GetFrustum().top * 2
                        + nearPlaneRightVec * relativeX * camera.GetFrustum().right * 2;
	//Vector4f startPoint = nearPlaneCenter + nearPlaneUpVec * relativeY * frustum.top * 2;

//	nearPlaneLLCorner + nearPlaneUpVec * relativeY * camera.GetFrustum().top * 2
//						+ nearPlaneRightVec * relativeX * camera.GetFrustum().right * 2;

	Vector4f clickDirection;
	if (camera.projectionType == Camera::PROJECTION_3D)
		clickDirection = startPoint - camera.Position();
	/// Straight direction if orthogonal, always.
	else if (camera.projectionType == Camera::ORTHOGONAL)
		clickDirection = camera.LookingAt();
	clickDirection.Normalize3();
	Vector4f endPoint = startPoint - clickDirection * camera.farPlane;

    /// Adjust start point so it looks correct on-screen.
   // startPoint += relativeY * nearPlaneUpVec * frustum.top/ zoom * 0.5f;
   // startPoint += relativeX * nearPlaneRightVec * frustum.right / zoom * 0.5f;

	Ray result;
	result.start = startPoint;
	result.direction = clickDirection;
	return result;
}

#define Maximum(a,b) (a > b? a : b)


/** Calculates a view transform based on the notion of having 
	a distance from center of movement (as in 3D-modelling programs), 
	a rotation aroud the same point, and translate the point based on position and relative position summed up.
*/
Matrix4d Camera::CalculateDefaultEditorMatrices(float distanceFromCenterOfMovement, Vector2f rotationXY, ConstVec3fr worldSpacePosition)
{
	Matrix4d viewMatrix, rotationMatrix;
	// Move camera before before main scenegraph rendering begins
	// First translate the camera relative to the viewing rotation-"origo"
	viewMatrix.Translate(0, 0, -distanceFromCentreOfMovement);
	/*
	*/
	rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(rotationXY[0], 1, 0, 0));
	rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(rotationXY[1], 0, 1, 0));

	/// o.o
	viewMatrix.Multiply(rotationMatrix);

	// Then translate the camera to it's position. (i.e. translate the world until camera is at a good position).
	Matrix4d translationMatrix = Matrix4d().Translate(-worldSpacePosition);
	viewMatrix.Multiply(translationMatrix);
	return viewMatrix;
}


