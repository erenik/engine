/// Emil Hedemalm
/// 2014-03-05
/// Camera class, required for rendering.

#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include <cstring>

float Camera::defaultRotationSpeed = 0.09f;
float Camera::defaultVelocity = 1.0f;

Camera::Camera(){
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
	memset(navigation, 0, sizeof(bool)*6);
	memset(orientation, 0, sizeof(bool)*6);
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
};

// Prints data, including position, matrices, etc.
void Camera::PrintData() const {
	std::cout<<"\nCamera::PrintData for camera: "<<name;
	std::cout<<"\n - ViewMatrix: "<<viewMatrix;
	std::cout<<"\n - Orthogonal: "<< ((projectionType == ORTHOGONAL)? "yes" : "no");
	std::cout<<"\n - Position: "<<position;
	std::cout<<"\n - Nearplane: "<<nearPlane;
	std::cout<<"\n - Farplane: "<<farPlane;
}

/// Begin moving in the specified direction
void Camera::Begin(int direction){
	navigation[direction] = true;
	UpdateNavigation();
}
/// Stop moving in the specified direction
void Camera::End(int direction){
	navigation[direction] = false;
	UpdateNavigation();
}
/// Begin rotating in the specified direction
void Camera::BeginRotate(int direction){
	orientation[direction] = true;
	UpdateNavigation();
}
/// Stop rotating in the specified direction
void Camera::EndRotate(int direction){
	orientation[direction] = false;
	UpdateNavigation();
}

/// Updates the frustum
void Camera::Update(){
	/// Load identity matrices before we re-calculate them
	projectionMatrix.LoadIdentity();
	viewMatrix.LoadIdentity();
	rotationMatrix.LoadIdentity();

	/// Begin by updating projection matrix as that is unlikely to vary with foci
	switch(projectionType){
		// OLD: Perspective-distortions when adjusting width and height!
		case PROJECTION_3D:
			projectionMatrix.InitProjectionMatrix(-zoom * widthRatio, zoom * widthRatio, -zoom * heightRatio, zoom * heightRatio, nearPlane, farPlane);
			break;
		case ORTHOGONAL:
			projectionMatrix.InitOrthoProjectionMatrix(-zoom * widthRatio, zoom * widthRatio, -zoom * heightRatio, zoom * heightRatio, nearPlane, farPlane);
			break;

	}


	/// 3rd Person "static-world-rotation" camera
	if (entityToTrack){
		// Move camera before before main scenegraph rendering begins
		// Rotate it
		if (trackingMode == TrackingMode::FROM_BEHIND){
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
				Clamp(dotProduct, 0.0f, 10000000.0f);
				dotProduct *= 0.08f;
				/// Should now have speed in our vector of interest, so use that as velocity.
				float distanceMultiplier = pow(dotProduct, 0.08f);
				Clamp(distanceMultiplier, 1.0f, 10.0f);
				distance *= distanceMultiplier;
			}
			viewMatrix.translate(0, 0, distance);
			
			Vector3f rotation = -entityToTrack->rotation;
			/// Rotate more, so that we view the entity from the front instead, if camera is in reverse-mode.
			if (revert)
			{
				rotation.y += PI;
			}
			rotation += offsetRotation;

			rotationMatrix.multiply(Matrix4d().InitRotationMatrix(rotation.x, 1, 0, 0));
			rotationMatrix.multiply(Matrix4d().InitRotationMatrix(rotation.y, 0, 1, 0));

			viewMatrix.multiply(rotationMatrix);
				/*
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(-this->entityToTrack->rotation.x, 1, 0, 0));
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(-this->entityToTrack->rotation.y, 0, 1, 0));
			*/
		}
		else if (trackingMode == TrackingMode::THIRD_PERSON){
			// First translate the camera relative to the viewing rotation-"origo"
			viewMatrix.translate(0, 0, this->distanceFromCentreOfMovement);

			rotationMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
			rotationMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));

			viewMatrix.multiply(rotationMatrix);
			/*
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
			viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));
			*/
		}

		// Then translate the camera to it's position. (i.e. translate the world until camera is at a good position).
		Matrix4d translationMatrix = Matrix4d().translate(-this->entityToTrack->position - relativePosition);
		viewMatrix.multiply(translationMatrix);
		/// If from behind, adjust it slightly afterward too!
		if (trackingMode == TrackingMode::FROM_BEHIND){
			viewMatrix.multiply(Matrix4d::InitTranslationMatrix(Vector3f(0, elevation, 0)));
		}
	}
	/// Regular free-fly camera
	else {
		// Move camera before before main scenegraph rendering begins
		// First translate the camera relative to the viewing rotation-"origo"
		viewMatrix.translate(0, 0, this->distanceFromCentreOfMovement);
		/*
		// Rotate it
		viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
		viewMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));
		*/
		rotationMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.x, 1, 0, 0));
		rotationMatrix.multiply(Matrix4d().InitRotationMatrix(this->rotation.y, 0, 1, 0));

		viewMatrix.multiply(rotationMatrix);


		// Then translate the camera to it's position. (i.e. translate the world until camera is at a good position).
		Matrix4d translationMatrix = Matrix4d().translate(this->position + relativePosition);
		viewMatrix.multiply(translationMatrix);
	}



	/// Some new temporary variables for updating the frustum
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
	leftVector = rotationMatrix.InvertedCopy().product(Vector4f(-1,0,0,1)).NormalizedCopy();
	lookingAtVector = rotationMatrix.InvertedCopy().product(Vector4f(0,0,-1,1)).NormalizedCopy();
	upVector = rotationMatrix.InvertedCopy().product(Vector4f(0,1,0,1)).NormalizedCopy();


	// Update frustum
	frustum.SetCamInternals(-zoom * widthRatio, zoom * widthRatio, -zoom * heightRatio, zoom * heightRatio, nearPlane, farPlane);
	frustum.SetCamPos(Vector3f(camPos), Vector3f(lookingAtVector), Vector3f(upVector));
}

/** Sets width/height ratio (commonly known as Aspect ratio).
	Ratios should be 1.0 and above, and will be scaled down as needed.
*/
void Camera::SetRatio(float width, float height){
	float smallest = (float)height;
	if (height > width)
		smallest = (float)width;
	this->widthRatio = (float) width / smallest;
	this->heightRatio = (float) height / smallest;
}
/** Sets width/height ratio (commonly known as Aspect ratio) using screen size.
*/
void Camera::SetRatio(int width, int height){
	float smallest = (float)height;
	if (height > width)
		smallest = (float)width;
	this->widthRatio = (float) width / smallest;
	this->heightRatio = (float) height / smallest;
}

/// To be called from render/physics-thread. Moves the camera using it's given enabled directions and velocities.
void Camera::ProcessMovement(float timeSinceLastUpdate)
{
	if (lastMovement == 0){
		lastMovement = Timer::GetCurrentTimeMs();
		return;
	}
	float timeDiff = timeSinceLastUpdate;
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
	
	/// Update matrices n stuff
	Update();
}

/// Updates base velocities depending on navigation booleans
void Camera::UpdateNavigation(){
	/// Navigation
	if (navigation[Direction::UP] && !navigation[Direction::DOWN])
		this->velocity.y = -this->defaultVelocity * this->flySpeed;
	else if (navigation[Direction::DOWN] && !navigation[Direction::UP])
		this->velocity.y = this->defaultVelocity * this->flySpeed;
	else
		this->velocity.y = 0;
	if (navigation[Direction::FORWARD] && !navigation[Direction::BACKWARD])
		this->velocity.z = this->defaultVelocity * this->flySpeed;
	else if (navigation[Direction::BACKWARD] && !navigation[Direction::FORWARD])
		this->velocity.z = -this->defaultVelocity * this->flySpeed;
	else
		this->velocity.z = 0;
	if (navigation[Direction::LEFT] && !navigation[Direction::RIGHT])
		this->velocity.x = this->defaultVelocity * this->flySpeed;
	else if (navigation[Direction::RIGHT] && !navigation[Direction::LEFT])
		this->velocity.x = -this->defaultVelocity * this->flySpeed;
	else
		this->velocity.x = 0;

	/// Rotation
	if (orientation[Direction::UP] && !orientation[Direction::DOWN])
		this->rotationVelocity.x = this->defaultRotationSpeed * this->rotationSpeed;
	else if (orientation[Direction::DOWN] && !orientation[Direction::UP])
		this->rotationVelocity.x = -this->defaultRotationSpeed * this->rotationSpeed;
	else
		this->rotationVelocity.x = 0;
	if (orientation[Direction::LEFT] && !orientation[Direction::RIGHT])
		this->rotationVelocity.y = this->defaultRotationSpeed * this->rotationSpeed;
	else if (orientation[Direction::RIGHT] && !orientation[Direction::LEFT])
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
Ray Camera::GetRayFromScreenCoordinates(int mouseX, int mouseY) const
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

	// Get relative positions of where we clicketiclicked, from 0.0 to 1.0 (0,0 in lower left corner)
	float clientAreaWidth = (float)Graphics.width;
	float clientAreaHeight = (float)Graphics.height;
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
