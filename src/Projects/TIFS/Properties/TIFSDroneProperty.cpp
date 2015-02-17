/// Emil Hedemalm
/// 2014-07-30
/// Dronely drone.

#include "TIFSDroneProperty.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Random/Random.h"

#include "Entity/EntityPropertyState.h"
#include "Physics/Messages/CollisionCallback.h"
#include "StateManager.h"

TIFSDroneProperty::TIFSDroneProperty(Entity * owner)
: EntityProperty("TIFSDroneProperty", TIFSProperty::DRONE, owner) 
{
	currentHP = maxHP = 5000;
	isActive = true;
	acceleration = 1.0;
}

int TIFSDroneProperty::ID()
{
	return TIFSProperty::DRONE;
}


/// Setup physics here.
void TIFSDroneProperty::OnSpawn()
{
	Physics.QueueMessage(new PMSetEntity(owner, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
	// Ignore gravity until it's destroyed or something.
	Physics.QueueMessage(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 0));

	// Setup linear damping for smoother movement.
	Physics.QueueMessage(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.9f));
}

Random droneMovement;

/// Time passed in seconds..!
void TIFSDroneProperty::Process(int timeInMs)
{
	if (!isActive)
		return;
	// Eh... walk to nearby turret?
	// Got a destination?
	if (!destination.MaxPart())
	{
		// Get one!
		destination = owner->position + Vector3f(droneMovement.Randf(50.f)-25.f, 0, droneMovement.Randf(50.f)-25.f);
	}		
	if (destination.MaxPart())
	{
		// Move to it.
		Vector3f toDestination = destination - owner->position;
		Vector3f toDestNormed = toDestination.NormalizedCopy();
		Physics.QueueMessage(new PMSetEntity(owner, PT_ACCELERATION, toDestNormed * acceleration));

		float distance2 = toDestination.LengthSquared();
		if (distance2 < 10)
		{
			// Destination reached.
			destination = Vector3f();
		}
	}
}

void TIFSDroneProperty::ProcessMessage(Message * message)
{
	switch(message->type)
	{
		case MessageType::COLLISSION_CALLBACK:
		{
			CollisionCallback * cc = (CollisionCallback * )message;
			// Take damage.
			if (isActive)
			{
				isActive = false;
				// Die.

				// Stop acceleration.
				PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
				// Start falling.
				PhysicsQueue.Add(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 1.0f));
			}
			
			break;
		}
	}
}



/// o-o
Entity * target;

