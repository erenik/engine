/// Emil Hedemalm
/// 2014-03-05
/// Camera class, required for rendering.

#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Window/Window.h"
#include <cstring>


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
Camera * CameraManager::NewCamera()
{
	Camera * camera = new Camera();
	cameras.Add(camera);
	return camera;
}
// Called from render/physics thread. updates movement/position of all cameras.
void CameraManager::Process()
{
	static int64 lastUpdate = 0;
	int64 cTime = Timer::GetCurrentTimeMs();
	int timeDiff = cTime - lastUpdate;
	timeDiff = timeDiff % 1000;
	lastUpdate = cTime;
	for (int i = 0; i < cameras.Size(); ++i)
	{
		Camera * camera = cameras[i];
		camera->ProcessMovement(timeDiff);
	}
}

Camera * CameraManager::DefaultCamera()
{
	if (!defaultCamera)
		defaultCamera = new Camera();
	cameras.Add(defaultCamera);
	return defaultCamera;
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
	entityToTrack = NULL;
	trackingMode = TrackingMode::FROM_BEHIND;
	nearPlane = -0.1f;
	farPlane = -100000.0f;
	zoom = 0.1f;
	distanceFromCentreOfMovement = 0.0f;
	position = Vector3f(10, -10, -20);
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
	/// Load identity matrices before we re-calculate them
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
		float halfWidth = windowWorkingArea.x * 0.5f;
		float halfHeight = windowWorkingArea.y * 0.5f;
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
	if (entityToTrack){
		// Move camera before before main scenegraph rendering begins
		// Rotate it
		if (trackingMode == TrackingMode::FROM_BEHIND)
		{
			float distance;
			// First translate the camera relative to the viewing rotation-"origo"
			/*
			if (entityToTrack->physics)
				distance = -this->entityToTrack->physics->physicalRadius*2;
			else
				distance = -this->entityToTrack->radius*2;
*/
			distance = -distanceFromCentreOfMovement;
			
			/// Check if we got zis speed option set.
			if (scaleDistanceWithVelocity){
				// Scale it a bit more depending on current speed!
				/// Fetch dot product of entity velocity and our look-direction, since that is what we care about.
				float dotProduct = this->entityToTrack->Velocity().DotProduct(this->LookingAt());
				ClampFloat(dotProduct, 0.0f, 10000000.0f);
				dotProduct *= 0.08f;
				/// Should now have speed in our vector of interest, so use that as velocity.
				float distanceMultiplier = pow(dotProduct, 0.08f);
				ClampFloat(distanceMultiplier, 1.0f, 10.0f);
				distance *= distanceMultiplier;
			}
			viewMatrix.Translate(0, 0, distance);
			
			Vector3f rotation = Vector3f(); //
			rotation = -entityToTrack->rotation;

			/// Need to.. 
			/// Rotate more, so that we view the entity from the front instead, if camera is in reverse-mode.
			if (revert)
			{
				rotation.y += PI;
			}
			rotation += offsetRotation;

			/// o-o
			if (entityToTrack->physics && entityToTrack->physics->useQuaternions)
			{
				if (inheritEntityRotation)
				{
					this->orientation = - entityToTrack->physics->orientation;
					/// Apply offset-rotation?
					useQuaternions = true;

					rotationMatrix = orientation.Matrix();
				}
			}
			else 
			{
				useQuaternions = false;
			/// Hmm..
//			rotationMatrix = entityToTrack->rotationMatrix;
				rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(rotation.x, 1, 0, 0));
				rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(rotation.y, 0, 1, 0));
			}



			

			viewMatrix.Multiply(rotationMatrix);
				/*
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(-this->entityToTrack->rotation.x, 1, 0, 0));
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(-this->entityToTrack->rotation.y, 0, 1, 0));
			*/
		}
		else if (trackingMode == TrackingMode::THIRD_PERSON)
		{
			// First translate the camera relative to the viewing rotation-"origo"
			viewMatrix.Translate(0, 0, this->distanceFromCentreOfMovement);

			rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
			rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));

			viewMatrix.Multiply(rotationMatrix);
			/*
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));
			*/
		}

		// Take the relative position and multiply it with the local axes of the entity we're tracking?
		Vector3f offset;
		if (inheritEntityRotation)
		{
			Vector3f up = entityToTrack->rotationMatrix.GetColumn(1);
			Vector3f right = entityToTrack->rotationMatrix.GetColumn(0);
			Vector3f forward = entityToTrack->rotationMatrix.GetColumn(2);
			offset = relativePosition.x * right + relativePosition.y * up + relativePosition.z * forward;
		}
		else 
			offset = relativePosition;

		// Then translate the camera to it's position. (i.e. translate the world until camera is at a good position).
		Matrix4d translationMatrix = Matrix4d().Translate(-this->entityToTrack->position - offset);
		viewMatrix.Multiply(translationMatrix);
		/// If from behind, adjust it slightly afterward too!
		if (trackingMode == TrackingMode::FROM_BEHIND){
			viewMatrix.Multiply(Matrix4d::InitTranslationMatrix(Vector3f(0, elevation, 0)));
		}
	}
	/// Regular free-fly camera
	else {
		// Move camera before before main scenegraph rendering begins
		// First translate the camera relative to the viewing rotation-"origo"
		viewMatrix.Translate(0, 0, this->distanceFromCentreOfMovement);
		/*
		// Rotate it
		viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
		viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));
		*/
		rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
		rotationMatrix.Multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));

		viewMatrix.Multiply(rotationMatrix);


		// Then translate the camera to it's position. (i.e. translate the world until camera is at a good position).
		Matrix4d translationMatrix = Matrix4d().Translate(this->position + relativePosition);
		viewMatrix.Multiply(translationMatrix);
	}



	/// Some new temporary variables for updating the frustum
	float sample = viewMatrix.GetColumn(0).x;
	assert(AbsoluteValue(sample) < 100000.f);
	//	Vector4f camPos, lookingAtVector, upVector;
	camPos = viewMatrix.InvertedCopy() * Vector4d(0,0,0,1);	// THIS

	// Calculate camLook and camUp Vectors
	Vector4d moveVec = Vector4d(0, 0, -1, 1);
/*	Matrix4d rotationMatrix;

	moveVec = rotationMatrix.InitRotationMatrix(this->rotation.x, 1, 0, 0).InvertedCopy() * moveVec;
	moveVec = rotationMatrix.InitRotationMatrix(this->rotation.y, 0, 1, 0).InvertedCopy() * moveVec;
	lookingAtVector = moveVec;
	lookingAtVector.Normalize3();	// Normalize!

	moveVec = Vector4d(0, 0, -1, 1);
	moveVec = rotationMatrix.InitRotationMatrix(this->rotation.x - PI/2, 1, 0, 0).InvertedCopy() * moveVec;
	moveVec = rotationMatrix.InitRotationMatrix(this->rotation.y, 0, 1, 0).InvertedCopy() * moveVec;
	upVector = moveVec;
	upVector.Normalize3();		// Normalize!

	leftVector = upVector.CrossProduct(lookingAtVector);
	leftVector.Normalize3();
*/
	/// Extract global coordinates
	leftVector = rotationMatrix.InvertedCopy().Product(Vector4f(-1,0,0,1)).NormalizedCopy();
	lookingAtVector = rotationMatrix.InvertedCopy().Product(Vector4f(0,0,-1,1)).NormalizedCopy();
	upVector = rotationMatrix.InvertedCopy().Product(Vector4f(0,1,0,1)).NormalizedCopy();


	// Update frustum
	frustum.SetCamInternals(-zoom * widthRatio, zoom * widthRatio, -zoom * heightRatio, zoom * heightRatio, nearPlane, farPlane);
	frustum.SetCamPos(Vector3f(camPos), Vector3f(lookingAtVector), Vector3f(upVector));
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
void Camera::ProcessMovement(int timeInMs)
{
	if (lastMovement == 0){
		lastMovement = Timer::GetCurrentTimeMs();
		return;
	}
	float timeDiff = timeInMs;
	timeDiff *= 0.001f;
	Vector3f deltaP = velocity * timeDiff;
	/// We might want to calculate the position Diff using local camera co-ordinates..!
	Vector3f rightVec = this->lookingAtVector.CrossProduct(upVector);
	Vector3f totalPosDiff = - deltaP.z * this->lookingAtVector +
		deltaP.y * this->upVector +
		deltaP.x * rightVec;

	if (scaleSpeedWithZoom)
	{
		totalPosDiff *= zoom;
	}
	position += totalPosDiff;	
}

/// Updates base velocities depending on navigationControls booleans
void Camera::UpdateNavigation()
{
	/// Navigation
	if (navigationControls[Direction::UP] && !navigationControls[Direction::DOWN])
		this->velocity.y = -this->defaultVelocity * this->flySpeed;
	else if (navigationControls[Direction::DOWN] && !navigationControls[Direction::UP])
		this->velocity.y = this->defaultVelocity * this->flySpeed;
	else
		this->velocity.y = 0;
	if (navigationControls[Direction::FORWARD] && !navigationControls[Direction::BACKWARD])
		this->velocity.z = this->defaultVelocity * this->flySpeed;
	else if (navigationControls[Direction::BACKWARD] && !navigationControls[Direction::FORWARD])
		this->velocity.z = -this->defaultVelocity * this->flySpeed;
	else
		this->velocity.z = 0;
	if (navigationControls[Direction::LEFT] && !navigationControls[Direction::RIGHT])
		this->velocity.x = this->defaultVelocity * this->flySpeed;
	else if (navigationControls[Direction::RIGHT] && !navigationControls[Direction::LEFT])
		this->velocity.x = -this->defaultVelocity * this->flySpeed;
	else
		this->velocity.x = 0;

	/// Rotation
	if (orientationControls[Direction::UP] && !orientationControls[Direction::DOWN])
		this->rotationVelocity.x = this->defaultRotationSpeed * this->rotationSpeed;
	else if (orientationControls[Direction::DOWN] && !orientationControls[Direction::UP])
		this->rotationVelocity.x = -this->defaultRotationSpeed * this->rotationSpeed;
	else
		this->rotationVelocity.x = 0;
	if (orientationControls[Direction::LEFT] && !orientationControls[Direction::RIGHT])
		this->rotationVelocity.y = this->defaultRotationSpeed * this->rotationSpeed;
	else if (orientationControls[Direction::RIGHT] && !orientationControls[Direction::LEFT])
		this->rotationVelocity.y = -this->defaultRotationSpeed * this->rotationSpeed;
	else
		this->rotationVelocity.y = 0;
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
	float clientAreaWidth = (float)windowSize.x;
	float clientAreaHeight = (float)windowSize.y;
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
