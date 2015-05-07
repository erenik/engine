/// Emil Hedemalm
/// 2014-08-06
/** A property which sets up movement and camera controls
	for an arbitrary first-person playable entity.
	
	Exact details how to sub-class this will be updated later on.
	Inspiration may also be taken to create your own tailored variant.
*/

#include "InputState.h"
#include "FirstPersonPlayerProperty.h"

#include "Input/InputManager.h"

#include "StateManager.h"

#include "Message/Message.h"

#include "Physics/Messages/PhysicsMessage.h"
//#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/Shapes/Ray.h"
#include "PhysicsLib/Intersection.h"
#include "Physics/Messages/CollisionCallback.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"

#include "Model/Model.h"

#include "Window/AppWindow.h"

FirstPersonPlayerProperty::FirstPersonPlayerProperty(String propertyName, int id, Entity * owner)
: EntityProperty(propertyName, id, owner)
{
	inputFocus = false;
	lastRight = 0.f;
	autorun = false;
	primaryTarget = NULL;
	// 2 frictions, one when moving, other when (trying to) stand still.
	movementSpeed = 7.f;
	frictionOnStop = 0.5f;
	frictionOnRun = 0.1f;
	jumpSpeed = 9.f;
	// Minimum delay before collision callback messages reset the state.
	jumpCooldownMs = 500;
	jumping = true;
	lastJump = Time::Now();
	
	// Set physics properties by default?
	if (owner->physics)
	{
		owner->physics->requireGroundForLocalAcceleration = true;
	}
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
		case MessageType::COLLISSION_CALLBACK:
		{
			Time now = Time::Now();
			CollisionCallback * cc = (CollisionCallback*) message;
			// Enable jumping again after impact.
			if (AbsoluteValue(cc->impactNormal.y) > 0.9f)
			{
				/// If was in auto-run, starting running again.
				if ((now - lastJump).Milliseconds() > jumpCooldownMs)
				{
					jumping = false;
					if (autorun)
					{
						assert(movementSpeed == movementSpeed);
						PhysicsQueue.Add(new PMSetEntity(owner, PT_RELATIVE_ACCELERATION, Vector3f(0, 0, movementSpeed)));
					}
				}
			}
			break;	
		}

		case MessageType::RAYCAST:
		{
			if (!this->raycast)
				return;
			Raycast * raycastMessage = (Raycast*) message;
			List<Intersection> contacts = raycastMessage->isecs;
			targets.Clear();

			Ray ray = raycastMessage->ray;
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
	// Toggle flag.
	autorun = !autorun;
	// Set facing-rotational behaviour
	PhysicsQueue.Add(new PMSetEntity(owner, PT_FACE_VELOCITY_DIRECTION, !autorun));	
	// Disable regular walking acc.
	if (autorun)
	{
		// Set relative velocity. It will solve the issue of direction by using the current rotation :)
		PhysicsQueue.Add(new PMSetEntity(owner, PT_RELATIVE_ACCELERATION, Vector3f(0, 0, movementSpeed)));
		PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, Vector3f())); 
	}
	else 
		PhysicsQueue.Add(new PMSetEntity(owner, PT_RELATIVE_ACCELERATION, Vector3f()));
}



/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
	and then passed on as messages to the character with inputFocus turned on.
*/
void FirstPersonPlayerProperty::ProcessInput()
{
	// Skip if CTLR is pressed, should be some other binding then.
	if (InputMan.KeyPressed(KEY::CTRL) || InputMan.KeyPressed(KEY::ALT))
		return;
	forward = 0.f;
	// Should probably check some lexicon of key-bindings here too. or?
	if (InputMan.KeyPressed(KEY::W))
		forward -= 1.f;
	if (InputMan.KeyPressed(KEY::S))
		forward += 1.f;
	right = 0.f;
	if (InputMan.KeyPressed(KEY::A))
		right -= 1.f;
	if (InputMan.KeyPressed(KEY::D))
		right += 1.f;

	/// o.o
	if (InputMan.KeyPressedThisFrame(KEY::R))
	{
		ToggleAutorun();
	}

	if (InputMan.KeyPressed(KEY::SPACE))
	{
		if (!jumping)
		{
			// Jump!
			PhysicsQueue.Add(new PMApplyImpulse(owner, Vector3f(0,jumpSpeed,0), Vector3f()));
			lastJump = Time::Now();
			jumping = true;
			PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
			/// Cancel auto run as well.
			PhysicsQueue.Add(new PMSetEntity(owner, PT_RELATIVE_ACCELERATION, Vector3f()));
		}
	}

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
		if (raycast)
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
		if (InputMan.KeyPressed(KEY::LEFT))
			cameraRight += 1.f;
		if (InputMan.KeyPressed(KEY::RIGHT))
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
		if (InputMan.KeyPressed(KEY::UP))
			cameraUp += 1.f;
		if (InputMan.KeyPressed(KEY::DOWN))
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
		if (InputMan.KeyPressed(KEY::PG_DOWN))
		{
			cameraZoomMultiplier *= ZOOM_MULTIPLIER_SPEED;
			cameraZoom = CONSTANT_ZOOM_SPEED;
		}
		if (InputMan.KeyPressed(KEY::PG_UP))
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
		if (InputMan.KeyPressed(KEY::LEFT))
			cameraTurn += 1.f;
		if (InputMan.KeyPressed(KEY::RIGHT))
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
	if (jumping)
		return;
	if (lastVelocity == newVelocity)
	{
		return;
	}
	// If bad, like when new entity/camera and pointers have not had the time to update yet..?
	if (newVelocity.x != newVelocity.x)
		return;
	assert(newVelocity.x == newVelocity.x);

	lastVelocity = newVelocity;
	Vector3f normalizedVelocity = newVelocity.NormalizedCopy();

	// And set it!
	PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, newVelocity)); 
	// Walking?
	if (newVelocity.LengthSquared())
		PhysicsQueue.Add(new PMSetEntity(owner, PT_FRICTION, frictionOnRun));
	else 
		PhysicsQueue.Add(new PMSetEntity(owner, PT_FRICTION, frictionOnStop));
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
	AppWindow * activeWindow = ActiveWindow();
	if (activeWindow != MainWindow())
		return;
	// Try to get ray.
	if (!activeWindow->GetRayFromScreenCoordinates(inputState->mousePosition, ray))
		return;

	// Do ray cast within the physics system
	PMRaycast * raycast = new PMRaycast(ray);
	raycast->relevantEntity = owner;
	raycast->msg = "UpdateTargetsByCursorPosition";
	PhysicsQueue.Add(raycast);

	/// React to message of it later on.
}


