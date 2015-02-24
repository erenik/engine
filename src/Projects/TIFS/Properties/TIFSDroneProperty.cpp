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

#include "Physics/PhysicsProperty.h"

TIFSDroneProperty::TIFSDroneProperty(Entity * owner)
: EntityProperty("TIFSDroneProperty", TIFSProperty::DRONE, owner) 
{
	type = HOVER_DRONE;
}

int TIFSDroneProperty::ID()
{
	return TIFSProperty::DRONE;
}


/// Setup physics here.
void TIFSDroneProperty::OnSpawn()
{
	PhysicsProperty * pp = new PhysicsProperty();
	owner->physics = pp;
	pp->type = PhysicsType::DYNAMIC;
	pp->collisionCategory = CC_DRONE;
	pp->collisionFilter = CC_LASER | CC_ENVIRON;
	pp->collissionCallback = true;

	isActive = true;
	acceleration = 1.0;
	state = DESCENDING_ONTO_CITY;
	timeInStateMs = 0;
	timeSinceLastVelUpdate = 0;

	// Setup initial linear damping for the free-fall - nearly none.
	pp->SetLinearDamping(0.98f);

	switch(type)
	{
		case HOVER_DRONE:
			currentHP = maxHP = 100;
			break;
		case FLYING_DRONE: 
			pp->faceVelocityDirection = true;
			currentHP = maxHP = 700;
			break;
	}

}

Random droneMovement;

/// Time passed in seconds..!
void TIFSDroneProperty::Process(int timeInMs)
{
	if (!isActive)
		return;
	timeInStateMs += timeInMs;

	switch(type)
	{
	case HOVER_DRONE:
		ProcessHoverDrone(timeInMs);
		break;
	case FLYING_DRONE:
		ProcessFlyingDrone(timeInMs);
		break;
	}
	timeSinceLastVelUpdate += timeInMs;
	if (timeSinceLastVelUpdate > 200)
		UpdateVelocity();
	if (destination.MaxPart() == 0)
		UpdateDestination();
}

void Drone::ProcessHoverDrone(int timeInMs)
{
	switch(state)
	{
	case DESCENDING_ONTO_CITY:
	{
		// Change state once we approach ground.
		if (owner->position.y < 300.f)
		{
			// Setup linear damping to break our fall.
			QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.5f));
			QueuePhysics(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 0.5f));
			if (owner->position.y < 150.f)
			{
				state = RANDOM_ROAM;
				++tifs->dronesArrived;
				QueuePhysics(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.3f));
				QueuePhysics(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 0.f));
			}
		}
		break;
	}
	case RANDOM_ROAM:
		if (timeInStateMs > 10000)
		{
			SetState(CHASE_PLAYER);
		}
		break;
	case ATTACK_TURRET:
	{
		/// Not dead yet? Move along.
		if (timeInStateMs > 20000)
		{
			SetState(RANDOM_ROAM);
		}
		break;
	}
	case CHASE_PLAYER:
		if (timeInStateMs > 10000)
		{
			SetState(RANDOM_ROAM);
		}
		break;
	default:
		if (timeInStateMs > 10000)
		{
			SetState((state + 1) % DRONE_STATES);
		}
		break;
	}	
}

void TIFSDroneProperty::ProcessFlyingDrone(int timeInMs)
{
	// Start flying.
}


void TIFSDroneProperty::Damage(int amount)
{
	currentHP -= amount;
	if (currentHP < 0)
	{
		// Die.
		// Stop acceleration.
		PhysicsQueue.Add(new PMSetEntity(owner, PT_ACCELERATION, Vector3f()));
		// Start falling.
		PhysicsQueue.Add(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 1.0f));
		isActive = false;
		switch(type)
		{
		case HOVER_DRONE:
			++tifs->dronesDestroyed;
			break;
		case FLYING_DRONE:
			++tifs->flyingDronesDestroyed;
			break;
		}
	}
	else 
	{
		// Change state to chase whatever hurt it?
	}
}


void TIFSDroneProperty::UpdateDestination()
{
	bool circleTarget = false;
	Vector3f targetPosition;
	switch(state)
	{
		case DESCENDING_ONTO_CITY:
		{
			// Descend to around 100 units above ground, then change state.
			destination = owner->position * Vector3f(1,0,1) + Vector3f(0,100,0);
			break;
		}
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
					Damage(100000);
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

