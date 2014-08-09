/// Emil Hedemalm
/// 2014-07-30
/// Dronely drone.

#include "TIFSDroneProperty.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Random/Random.h"

TIFSDroneProperty::TIFSDroneProperty(Entity * owner)
: EntityProperty("TIFSDroneProperty", TIFSProperty::DRONE, owner) 
{

	currentHP = maxHP = 5000;
	active = true;
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
		Physics.QueueMessage(new PMSetEntity(owner, PT_ACCELERATION, toDestination));

		float distance2 = toDestination.LengthSquared();
		if (distance2 < 10)
		{
			// Destination reached.
			destination = Vector3f();
		}
	}
}


/// o-o
Entity * target;

