/// Emil Hedemalm
/// 2014-07-17
/// Property for the ball and other entities in a simple Pong-game.

#include "BreakoutBallProperty.h"
#include "BreakoutBrickProperty.h"

#include "Graphics/GraphicsManager.h"
#include "Physics/PhysicsManager.h"

#include "Entity/Entity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"

#include "Time/Time.h"

#include "Message/MessageManager.h"

#include "Audio/AudioManager.h"

BreakoutBallProperty::BreakoutBallProperty(Entity * owner, float defaultMinHorizontalVel)
	: EntityProperty("BreakoutBallProperty", ID(), owner), defaultMinVerticalVel(defaultMinHorizontalVel)
{
	minimumVerticalVelocity = 0.f;
	velocityIncreasePerBounce = 0.f;
	maxYVel = 1000.f;
};

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int BreakoutBallProperty::ID()
{
	return EntityPropertyID::MINI_GAME_2 + 2;
}


/// Time passed in seconds..!
void BreakoutBallProperty::Process(int timeInMs)
{
	if (!owner->physics)
		return;
	// Ball still a long time? Re-spawn it! o-o
	if (owner->physics->velocity.MaxPart() == 0)
	{
		// There is no such thing as a sleeping ball...
		static Time ballStoppedStartTime;
		if (ballStoppedStartTime.intervals == 0)
			ballStoppedStartTime = Time::Now();
		Time currentTime = Time::Now();
		if ((currentTime - ballStoppedStartTime).Seconds() > 1)
		{
			// Reset it.
		//	OnSpawn();
			// Give points to players too.
			MesMan.QueueMessages("BreakoutBallGoal:"+String::ToString(owner->position[0]));
			sleeping = true;
			// Hide it from physics and graphics too!
			Graphics.QueueMessage(new GMUnregisterEntity(owner));
			Physics.QueueMessage(new PMUnregisterEntity(owner));
		}
	}
}


/// Resets position to default.
void BreakoutBallProperty::OnSpawn()
{
	// Set ball propertiiiies!
	minimumVerticalVelocity = defaultMinVerticalVel;
	sleeping = false;
}


void BreakoutBallProperty::OnCollision(Collision & c)
{
	// Skip unresolved collisions. Not of interest for ze ball.
	if (!c.resolved)
		return;
	Entity * other;
	if (c.one == owner)
		other = c.two;
	else
		other = c.one;

	// Play SFX!
	BreakoutBrickProperty * bbp = (BreakoutBrickProperty*) other->GetProperty("BreakoutBrickProperty");
	float sfxVol = 0.05f;
	if (other->GetProperty("BreakoutPaddleProperty"))
	{
		AudioMan.QueueMessage(new AMPlaySFX("Breakout/Collision1.ogg", sfxVol));
//		AudioMan.PlaySFX("Breakout/Collision1.ogg", sfxVol);
	}
	else if (bbp && !bbp->sleeping)
	{
		// Give points?
		AudioMan.QueueMessage(new AMPlaySFX("Breakout/Collision2.ogg", sfxVol));
		//AudioMan.PlaySFX("Breakout/Collision2.ogg", sfxVol);

		bbp->hitsTaken++;
		if (bbp->hitsTaken >= bbp->hitsNeeded)
		{
			// Unsubscribe from physics and graphics?
			Graphics.QueueMessage(new GMUnregisterEntity(bbp->owner));
			Physics.QueueMessage(new PMUnregisterEntity(bbp->owner));
			MesMan.QueueMessages("BrickBroken:"+String::ToString(bbp->score));
			bbp->sleeping = true;
			// Maybe spawn some power-ups?
			bbp->OnBroken();
		}

	}
	else 
	{
		AudioMan.QueueMessage(new AMPlaySFX("Breakout/Collision3.ogg", sfxVol));
	//	AudioMan.PlaySFX("Breakout/Collision3.ogg", sfxVol);
	}
}
