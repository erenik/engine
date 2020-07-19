/// Emil Hedemalm
/// 2015-02-18
/// Tools for setting up and managing input for a default scene editor camera (3D), via en Entity.

#include "EditorCamera.h"

#include "InputState.h"
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

#include "Entity/EntityManager.h"
#include "Maps/MapManager.h"

/// Creates an entity featuring the EditorCameraProperty. If entity argument is provided, link to the entity is given as well.
Camera * CreateEditorCamera(EntitySharedPtr* entityPointer /*= NULL*/)
{
	EntitySharedPtr entity = EntityMan.CreateEntity("EditorCameraEntity", NULL, NULL);
	/// Set up default settings.

	EditorCameraProperty * ecp = new EditorCameraProperty(entity);
	entity->properties.Add(ecp);
	Camera * camera = ecp->CreateCamera();
	/// Set input- and camera focus..?
	GraphicsQueue.Add(new GMSetCamera(camera));

	// No need to add it to the map.
	MapMan.AddEntity(entity);

	/// o.o
	InputMan.SetInputFocus(entity);

	// Return data.
	if (entityPointer)
		*entityPointer = entity;
	return camera;
}


EditorCameraProperty::EditorCameraProperty(EntitySharedPtr owner)
: EntityProperty("EditorCameraProperty", EntityPropertyID::EDITOR_CAMERA, owner)
{
	inputFocusEnabled = true;
	lastRight = 0.f;
	autoFly = false;
	editorCamera = NULL;
	movementSpeed = 5.f;
}

/// Upon being unregistered from rendering.
void EditorCameraProperty::OnUnregistrationFromGraphics()
{
	if (editorCamera)
		GraphicsQueue.Add(new GMDeleteCamera(editorCamera));
}


/// Time passed in seconds..! Will steer if inputFocus is true.
void EditorCameraProperty::Process(int timeInMs)
{
	if (inputFocus)
		ProcessInput(timeInMs * 0.001f);
}


void EditorCameraProperty::ProcessMessage(Message * message)
{
	switch(message->type)
	{
		case MessageType::RAYCAST:
		{
			Raycast * raycast = (Raycast*) message;
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
				std::shared_ptr<Entity> entity = contact.entity;
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
void EditorCameraProperty::ToggleAutorun()
{
}

Camera * EditorCameraProperty::CreateCamera()
{
	editorCamera = CameraMan.NewCamera("EditorCamera", true);
	owner->cameraFocus = editorCamera;
	// Set defaults? No?
	return editorCamera;
}

/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
	and then passed on as messages to the character with inputFocus turned on.
*/
void EditorCameraProperty::ProcessInput(float timeInSeconds)
{
	float forward = 0.f;
	Vector3f moveVec;
	// Should probably check some lexicon of key-bindings here too. or?
	if (InputMan.KeyPressed(KEY::W))
		moveVec.z -= 1.f;
	if (InputMan.KeyPressed(KEY::S))
		moveVec.z += 1.f;
	float right = 0.f;
	if (InputMan.KeyPressed(KEY::A))
		moveVec.x -= 1.f;
	if (InputMan.KeyPressed(KEY::D))
		moveVec.x += 1.f;
	float up = 0.f;
	if (InputMan.KeyPressed(KEY::E))
		moveVec.y = 1.f;
	else if (InputMan.KeyPressed(KEY::Q))
		moveVec.y = -1.f;

	/// Adjust speed.
	bool speedAdjusted = false;
	if (InputMan.KeyPressed(KEY::PLUS))
	{
		movementSpeed += 0.2f * timeInSeconds;
		movementSpeed *= pow(2.0f, timeInSeconds);
		speedAdjusted = true;
	}
	else if (InputMan.KeyPressed(KEY::MINUS))
	{
		movementSpeed -= 0.2f * timeInSeconds;
		movementSpeed /= pow(2.0f, timeInSeconds);
		speedAdjusted = true;
	}
	if (speedAdjusted)
	{
		ClampFloat(movementSpeed, 0, 10000.f);
		std::cout<<"\nMovement speed of EditorCamera set to "<<movementSpeed;
	}
	if (InputMan.KeyPressed(KEY::SPACE))
	{
		if (!jumping)
		{
		}
	}

	moveVec *= movementSpeed;

	float rotationSpeed = 1.2f;
	right *= rotationSpeed;

	Vector3f acc;
	acc[2] = forward;

//	Vector3f rot;
//	rot[1] = right;
	// 
	// Auto-running,.
	if (autoFly)
	{
		if (lastAcc != acc)
		{
		}
		if (right != lastRight)
		{
			// Rotate int Y..
			GraphicsQueue.Add(new GMSetCamera(editorCamera, CT_ROTATION_SPEED_YAW, right));
		}
	}
			
	/// o-o cameraaaa focsuuuuuu!
	if (owner->cameraFocus)
	{
		
		// Check mouse position.
		UpdateTargetsByCursorPosition();

		// Free-form running (relative to camera)
		if (!autoFly)
		{
			// Just set velocity again?
			UpdateVelocity(moveVec);
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
			cameraRight -= 1.f;
		if (InputMan.KeyPressed(KEY::RIGHT))
			cameraRight += 1.f;

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
			cameraUp -= 1.f;
		if (InputMan.KeyPressed(KEY::DOWN))
			cameraUp += 1.f;
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

void EditorCameraProperty::UpdateVelocity(ConstVec3fr newVelocity)
{
	static Vector3f lastVelocity;
	if (lastVelocity == newVelocity)
	{
		return;
	}
	lastVelocity = newVelocity;
	Vector3f normalizedVelocity = newVelocity.NormalizedCopy();

	// And set it!
	GraphicsQueue.Add(new GMSetCamera(editorCamera, CT_VELOCITY, newVelocity)); 
	// Walking?
}

// Checks mouse position and casts a ray. Will return all entities along the ray, starting with the closest one.
void EditorCameraProperty::UpdateTargetsByCursorPosition()
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


