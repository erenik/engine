/// Emil Hedemalm
/// 2014-08-01
/// Binds the Entity- and custom MORPG character objects together nicely.

#include "MORPGCharacterProperty.h"

#include "MORPG/Character/Character.h"

#include "Input/InputManager.h"

#include "Physics/PhysicsManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"

#include "Model.h"

MORPGCharacterProperty::MORPGCharacterProperty(Entity * characterEntity, Character * associatedWithCharacter)
: EntityProperty("MORPGCharacterProperty", 0, characterEntity), ch(associatedWithCharacter)
{
	inputFocus = false;
	lastRight = 0.f;
}

/// Time passed in seconds..! Will steer if inputFocus is true.
void MORPGCharacterProperty::Process(int timeInMs)
{
	if (inputFocus)
		ProcessInput();
}


/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
	and then passed on as messages to the character with inputFocus turned on.
*/
void MORPGCharacterProperty::ProcessInput()
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

	float movementSpeed = 2.f;
	forward *= movementSpeed;

	float rotationSpeed = 1.2f;
	right *= rotationSpeed;

	Vector3f acc;
	acc.z = forward;

//	Vector3f rot;
//	rot.y = right;

	if (lastAcc != acc)
	{
		// Set speed.
		Physics.QueueMessage(new PMSetEntity(owner, PT_RELATIVE_VELOCITY, acc));
		lastAcc = acc;

		if (acc.MaxPart() == 0)
			Physics.QueueMessage(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.5f));
		else
			Physics.QueueMessage(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.99f));
	}
	if (right != lastRight)
	{
		// Rotate int Y..
		Quaternion q = Quaternion(Vector3f(0,1,0), right);
		Physics.QueueMessage(new PMSetEntity(owner, PT_ROTATIONAL_VELOCITY, q));
		lastRight = right;
	}

	
	if (owner->cameraFocus)
	{
		/// Make sure the camera is rotating around the center of the entity.
		float height = 1.7f;
		if (owner->cameraFocus->relativePosition.y != height)
		{
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_RELATIVE_POSITION_Y, height - owner->model->centerOfModel.y));
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
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_YAW, -cameraRight));
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
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_ROTATION_SPEED_PITCH, -cameraUp)); 
			pastCameraUp = cameraUp;
		}


		float cameraZoom = 0.f;
		float cameraZoomMultiplier = 1.f;
#define CONSTANT_ZOOM_SPEED 1.f
#define ZOOM_MULTIPLIER_SPEED 1.3f
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
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED, cameraZoom));
			Graphics.QueueMessage(new GMSetCamera(owner->cameraFocus, CT_DISTANCE_FROM_CENTER_OF_MOVEMENT_SPEED_MULTIPLIER, cameraZoomMultiplier));
			pastCameraZoom = cameraZoom;
		}
	}
}
