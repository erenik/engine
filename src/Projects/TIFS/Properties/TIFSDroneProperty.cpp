/// Emil Hedemalm
/// 2014-07-30
/// Dronely drone.

#include "TIFSDroneProperty.h"
#include "../TIFS.h"
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
	state = RANDOM_ROAM;
	timeInStateMs = 0;
	timeSinceLastVelUpdate = 0;
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
	timeInStateMs += timeInMs;
	if (timeInStateMs > 10000)
	{
		SetState((state + 1) % DRONE_STATES);
	}
	else 
	{
		timeSinceLastVelUpdate += timeInMs;
		if (timeSinceLastVelUpdate > 200)
			UpdateVelocity();
		if (destination.MaxPart() == 0)
			UpdateDestination();
	}
}

void TIFSDroneProperty::UpdateDestination()
{
	bool circleTarget = false;
	Vector3f targetPosition;
	switch(state)
	{
		case RANDOM_ROAM:
			// Get one!
			destination = owner->position + Vector3f(droneMovement.Randf(50.f)-25.f, 0, droneMovement.Randf(50.f)-25.f);
			break;
		case CHASE_PLAYER:
		{
			// Get closest player.
			Entity * target = tifs->GetClosestDefender(owner->position);
			if (target)
			{
				destination = target->position + Vector3f(0,5,0);
				circleTarget = true;
				targetPosition = target->position;
			}
			else 
			{
				SetState(RANDOM_ROAM);
			}
			break;
		}
		case TIFSDroneProperty::ATTACK_TURRET:
		{
			// Get closest player.
			Turret * target = tifs->GetClosestActiveTurret(owner->position);
			if (target)
			{
				targetPosition = target->position;
				circleTarget = true;
			}
			/// No active turret found? Search player instead?
			else 
			{
				SetState(CHASE_PLAYER);
			}
			break;
		}
		default:
			assert(false);
	}
	/// o.o;
	if (targetPosition.MaxPart() && circleTarget)
	{
		float circleRadius = 3.f;
		destination = targetPosition + 
			(Vector3f(2,0,0) * (0.5f - rand() * oneDivRandMaxFloat) + 
			Vector3f(0,0,2) * (0.5f - rand() * oneDivRandMaxFloat) + 
			Vector3f(0,1.5f,0)) * circleRadius;
	}
	UpdateVelocity();
}

void TIFSDroneProperty::UpdateVelocity()
{
	timeSinceLastVelUpdate = 0;
	// Got a destination?
	if (destination.MaxPart())
	{
		// Move to it.
		Vector3f toDestination = destination - owner->position;
		Vector3f toDestNormed = toDestination.NormalizedCopy();

		float distance2 = toDestination.LengthSquared();
		/// Close enough? Break.
		if (distance2 > 100)
		{
			PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, toDestNormed * acceleration));
			// Set linear damping?
			PhysicsQueue.Add(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.1f));
		}
		else if (distance2 > 10)
		{
			PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, toDestNormed * acceleration));			
			/// Just add some more damping.
			PhysicsQueue.Add(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.2f));
		}
		else 
		{
			destination = Vector3f();
			PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
			/// Just add some more damping.
			PhysicsQueue.Add(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.4f));
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
				Entity * other = NULL;
				if (cc->one == owner)
				{
					other = cc->two;
				}
				else 
				{
					other = cc->one;
				}
				TIFSProjectile * proj = (TIFSProjectile*)other->GetProperty(TIFSProjectile::ID());
				if (proj)
				{
					// Die.
					// Stop acceleration.
					PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
					// Start falling.
					PhysicsQueue.Add(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 1.0f));
					isActive = false;
				}
				// Take some collision damage only?
			
			}
			
			break;
		}
	}
}



/// o-o
Entity * target;

void TIFSDroneProperty::SetState(int newState)
{
	state = newState;
	timeInStateMs = 0;
	UpdateDestination();
}

