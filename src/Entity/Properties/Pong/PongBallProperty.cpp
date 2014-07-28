/// Emil Hedemalm
/// 2014-07-17
/// Property for the ball and other entities in a simple Pong-game.

#include "PongBallProperty.h"

#include "Entity/Entity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"

#include "Time/Time.h"

#include "Message/MessageManager.h"

PongBallProperty::PongBallProperty(Entity * owner, float defaultMinHorizontalVel)
	: EntityProperty("PongBallProperty", ID(), owner), defaultMinHorizontalVel(defaultMinHorizontalVel)
{
	minimumHorizontalVelocity = 0.f;
	velocityIncreasePerBounce = 0.f;
	maxYVel = 1000.f;
};

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int PongBallProperty::ID()
{
	return EntityPropertyID::MINI_GAME_1 + 1;
}

/// Time passed in seconds..!
void PongBallProperty::Process(int timeInMs)
{
	if (!owner->physics)
		return;
	// Ball still a long time? Re-spawn it! o-o
	if (owner->physics->velocity.MaxPart() == 0)
	{
		if (!sleeping)
		{
			static Time ballStoppedStartTime;
			if (ballStoppedStartTime.intervals == 0)
				ballStoppedStartTime = Time::Now();
			Time currentTime = Time::Now();
			if ((currentTime - ballStoppedStartTime).Seconds() > 1)
			{
				// Reset it.
			//	OnSpawn();
				// Give points to players too.
				MesMan.QueueMessages("PongBallGoal:"+String::ToString(owner->position.x));
				sleeping = true;
			}
		}
	}
}


/// Resets position to default.
void PongBallProperty::OnSpawn()
{
	// Set ball propertiiiies!
	minimumHorizontalVelocity = defaultMinHorizontalVel;
	sleeping = false;
}

