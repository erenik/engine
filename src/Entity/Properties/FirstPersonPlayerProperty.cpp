/// Emil Hedemalm
/// 2014-08-06
/** A property which sets up movement and camera controls
	for an arbitrary first-person playable entity.
	
	Exact details how to sub-class this will be updated later on.
	Inspiration may also be taken to create your own tailored variant.
*/

#include "FirstPersonPlayerProperty.h"

#include "Input/InputManager.h"

#include "StateManager.h"

#include "Message/Message.h"

#include "Physics/Messages/PhysicsMessage.h"
//#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/Shapes/Ray.h"
#include "PhysicsLib/Intersection.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"

#include "Model/Model.h"

#include "Window/Window.h"

FirstPersonPlayerProperty::FirstPersonPlayerProperty(String propertyName, int id, Entity * owner)
: EntityProperty(propertyName, id, owner)
{
	inputFocus = false;
	lastRight = 0.f;
	autorun = false;
	movementSpeed = 2.f;
	primaryTarget = NULL;
}

/// Time passed in seconds..! Will steer if inputFocus is true.
void FirstPersonPlayerProperty::Process(int timeInMs)
{
	if (inputFocus)
		ProcessInput();
}


void FirstPersonPlayerProperty::ProcessMessage(Message * message)
{
	switch(message->type)
	{
		case MessageType::RAYCAST:
		{
			Raycast * raycast = (Raycast*) message;
			bool done;
			List<Intersection> contacts = raycast->isecs;
			targets.Clear();

			Ray ray = raycast->ray;
			if (contacts.Size())
			{
				lastRaycastTargetPosition = ray.start + ray.direction * contacts[0].distance;
			}

			/// Move list of contacts to list of entities?
			for (int i = 0; i < contacts.Size(); ++i)
			{
				Intersection & contact = contacts[i];
				Entity * entity = contact.entity;
//				std::cout<<"\n contacts "<<i<<" "<<entity->name;
				targets.Add(entity);
			}
			// Set primary target to the first one from the raycast.
			if (targets.Size())
			{
				primaryTarget = targets[0];
		//		std::cout<<"\nTarget found: "<<targets[0]->name;	
			}
			else
				primaryTarget = NULL;

			break;	
		}
	}
}

/// o-o
void FirstPersonPlayerProperty::ToggleAutorun()
{
	autorun = !autorun;
	/// New state.. do stuffelistuff.
	if (autorun)
	{
		if (!owner->physics)
			return;
		// Do we have a velocity? If not disable autorun straight away.
		if (!owner->physics->velocity.MaxPart())
		{
			autorun = false;
			return;
		}
		PhysicsQueue.Add(new PMSetEntity(owner, PT_FACE_VELOCITY_DIRECTION, false));
		// Set relative velocity. It will solve the issue of direction by using the current rotation :)
		Vector3f velocity(0, 0, movementSpeed);
		PhysicsQueue.Add(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, velocity));
		/// Disable regular velocity.
		PhysicsQueue.Add(new PMSetEntity(owner, PT_VELOCITY, Vector3f())); 
		/// Set damping in case the regular velocity persist somehow.
		PhysicsQueue.Add(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.5f));
	}
	// Leaving autorun-mode.
	else 
	{
		PhysicsQueue.Add(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.9f));
		PhysicsQueue.Add(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, Vector3f()));
	}
}



/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
	and then passed on as messages to the character with inputFocus turned on.
*/
void FirstPersonPlayerProperty::ProcessInput()
{
	float forward = 0.f;
	// Should probably check some lexicon of key-bindings here too. or?
	if (Input.KeyPressed(KEY::W))
		forward -= 1.f;
	if (Input.KeyPressed(KEY::S))
		forward += 1.f;
	float right = 0.f;
	if (Input.KeyPressed(KEY::A))
		right -= 1.f;
	if (Input.KeyPressed(KEY::D))
		right += 1.f;

	forward *= movementSpeed;

	float rotationSpeed = 1.2f;
	right *= rotationSpeed;

	Vector3f acc;
	acc[2] = forward;

//	Vector3f rot;
//	rot[1] = right;

	// 
	// Auto-running,.
	if (autorun)
	{
		if (lastAcc != acc)
		{
		}
		if (right != lastRight)
		{
			// Rotate int Y..
			Quaternion q = Quaternion(Vector3f(0,1,0), right);
			PhysicsQueue.Add(new PMSetEntity(owner, PT_ROTATIONAL_VELOCITY, q));
			lastRight = right;
		}
	}
			
	/// o-o cameraaaa focsuuuuuu!
	if (owner->cameraFocus)
	{
		
		// Check mouse position.
		UpdateTargetsByCursorPosition();


		// Free-form running (relative to camera)
		if (!autorun)
		{
			/// Get camera transform.
			Camera * camera = owner->cameraFocus;
			if (!camera)
				return;
			Vector3f camLookAt = camera->LookingAt();
			Vector3f forwardVector = -forward * camLookAt;
			forwardVector.Normalize();
			Vector3f rightwardVector = -right * camera->LeftVector();
			rightwardVector.Normalize();
			Vector3f newVelocity = forwardVector + rightwardVector;
			// Remove Y-component.
			newVelocity[1] = 0;
			Vector3f normalizedVelocity = newVelocity.NormalizedCopy();
			// Multiply movement speed.
			newVelocity = normalizedVelocity * movementSpeed;
			UpdateVelocity(newVelocity);
		}


		/// Make sure the camera is rotating around the center of the entity. <- wat.
		float height = 1.7f;
		if (owner->cameraFocus->relativePosition[1] != height)
		{
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_RELATIVE_POSITION_Y, height));
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_TRACKING_POSITION_OFFSET, Vector3f(0,height,0)));
		}
		/// Camera Control, Booyakasha!
		float cameraRight = 0.f;
		if (Input.KeyPressed(KEY::LEFT))
			cameraRight += 1.f;
		if (Input.KeyPressed(KEY::RIGHT))
			cameraRight -= 1.f;

		// Set it! :D
		static float pastCameraRight = 0.f;
		if (cameraRight != pastCameraRight)
		{
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_YAW, -cameraRight));
			pastCameraRight = cameraRight;
		}

		/// Camera updown
		float cameraUp = 0.f;
		if (Input.KeyPressed(KEY::UP))
			cameraUp += 1.f;
		if (Input.KeyPressed(KEY::DOWN))
			cameraUp -= 1.f;
		static float pastCameraUp = 0.f;
		if (cameraUp != pastCameraUp)
		{
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_PITCH, -cameraUp)); 
			pastCameraUp = cameraUp;
		}


		float cameraZoom = 0.f;
		float cameraZoomMultiplier = 1.00f;
#define CONSTANT_ZOOM_SPEED 2.2f
#define ZOOM_MULTIPLIER_SPEED 1.5f
		if (Input.KeyPressed(KEY::PG_DOWN))
		{
			cameraZoomMultiplier *= ZOOM_MULTIPLIER_SPEED;
			cameraZoom = CONSTANT_ZOOM_SPEED;
		}
		if (Input.KeyPressed(KEY::PG_UP))
		{
			cameraZoomMultiplier /= ZOOM_MULTIPLIER_SPEED;
			cameraZoom = - CONSTANT_ZOOM_SPEED;
		}
		static float pastCameraZoom = 1.f;
		if (cameraZoom != pastCameraZoom)
		{
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED, cameraZoom));
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED_MULTIPLIER, cameraZoomMultiplier));
			pastCameraZoom = cameraZoom;
		}
		float cameraTurn = 0.f;
		if (Input.KeyPressed(KEY::LEFT))
			cameraTurn += 1.f;
		if (Input.KeyPressed(KEY::RIGHT))
			cameraTurn += -1;
		static float pastCameraTurn = 0.f;
		if (cameraTurn != pastCameraTurn)
		{
			cameraTurn *= 2.f;
			pastCameraTurn = cameraTurn;
			GraphicsQueue.Add(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_YAW, cameraTurn));
		}
	}
}

void FirstPersonPlayerProperty::UpdateVelocity(ConstVec3fr newVelocity)
{
	static Vector3f lastVelocity;
	if (lastVelocity == newVelocity)
	{
		return;
	}
	lastVelocity = newVelocity;
	Vector3f normalizedVelocity = newVelocity.NormalizedCopy();

	// And set it!
	PhysicsQueue.Add(new PMSetEntity(owner, PT_VELOCITY, newVelocity)); 
	// Depending on camera tracking-mode...	
	switch(owner->cameraFocus->trackingMode)
	{
		// Either rotate straight away.
		case TrackingMode::THIRD_PERSON:
		{				
			PhysicsQueue.Add(new PMSetEntity(owner, PT_FACE_VELOCITY_DIRECTION, true));
			if (newVelocity.MaxPart())
			{
				return;
				// Set our rotation toward this new destination too!
				float yaw = atan2(normalizedVelocity[2], normalizedVelocity[0]) + PI * 0.5f;
				PhysicsQueue.Add(new PMSetEntity(owner, PT_ROTATION_YAW, yaw));
			}
			break;
		}
		// Or possibly induce a rotational velocity.
		case TrackingMode::FIRST_PERSON:
		{
			// Disable .. stuff.
			PhysicsQueue.Add(new PMSetEntity(owner, PT_FACE_VELOCITY_DIRECTION, false));
			break;
		}
	}
}

// Checks mouse position and casts a ray. Will return all entities along the ray, starting with the closest one.
void FirstPersonPlayerProperty::UpdateTargetsByCursorPosition()
{
	Ray ray;
	Window * activeWindow = ActiveWindow();
	if (activeWindow != MainWindow())
		return;
	// Try to get ray.
		if (!activeWindow->GetRayFromScreenCoordinates(Input.mousePosition, ray))
		return;

	// Do ray cast within the physics system
	PMRaycast * raycast = new PMRaycast(ray);
	raycast->relevantEntity = owner;
	raycast->msg = "UpdateTargetsByCursorPosition";
	PhysicsQueue.Add(raycast);

	/// React to message of it later on.
}


