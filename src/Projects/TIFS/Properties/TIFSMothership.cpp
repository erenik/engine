/// Emil Hedemalm
/// 2015-02-23
/// o.o

#include "TIFSMothership.h"
#include "../TIFS.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Random/Random.h"

#include "Entity/EntityPropertyState.h"
#include "Physics/Messages/CollisionCallback.h"
#include "StateManager.h"

TIFSMothershipProperty::TIFSMothershipProperty(Entity * owner)
: EntityProperty("TIFSMothershipProperty", TIFSProperty::DRONE, owner) 
{
	currentHP = maxHP = 5000;
	isActive = true;
	acceleration = 1.0;
	state = RANDOM_ROAM;
	timeInStateMs = 0;
	timeSinceLastVelUpdate = 0;
}

int TIFSMothershipProperty::ID()
{
	return TIFSProperty::MOTHERSHIP;
}


/// Setup physics here.
void TIFSMothershipProperty::OnSpawn()
{
//	Physics.QueueMessage(new PMSetEntity(owner, PT_PHYSICS_TYPE, PhysicsType::KINEMATIC));
	// Ignore gravity until it's destroyed or something.
//	Physics.QueueMessage(new PMSetEntity(owner, PT_GRAVITY_MULTIPLIER, 0));

	// Setup linear damping for smoother movement.
//	Physics.QueueMessage(new PMSetEntity(owner, PT_LINEAR_DAMPING, 0.9f));
}

Random mothershipMovement;

/// Time passed in seconds..!
void TIFSMothershipProperty::Process(int timeInMs)
{
}

void TIFSMothershipProperty::Damage(int amount)
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
	}
}


void TIFSMothershipProperty::UpdateDestination()
{
}

void TIFSMothershipProperty::UpdateVelocity()
{
}

void Mothership::ProcessMessage(Message * message)
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
					Damage(proj->damage);
				}
				// Take some collision damage only?
			
			}
			
			break;
		}
	}
}

void Mothership::SetState(int newState)
{
	state = newState;
	timeInStateMs = 0;
	UpdateDestination();
}

