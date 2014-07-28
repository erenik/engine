/// Emil Hedemalm
/// 2014-07-17
/// Property for the paddles (players) in a simple Pong-game.
/// Contains the logic for a simple AI which can be toggled.

#include "BreakoutPaddleProperty.h"
#include "BreakoutBallProperty.h"

#include "Maps/MapManager.h"
#include "Entity/Entity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Messages/PhysicsMessage.h"

BreakoutPaddleProperty::BreakoutPaddleProperty(Entity * owner, Vector2f lookAt, float aiSpeed)
	: EntityProperty("BreakoutPaddleProperty", ID(), owner), aiSpeed(aiSpeed), lookAt(lookAt)
{
	score = 0;
	lastUserInput = Time::Now();
	initialScale = Vector3f(1,1,1);
}

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int BreakoutPaddleProperty::ID()
{
	return EntityPropertyID::MINI_GAME_2 + 0;
}

/// Time passed in seconds..!
void BreakoutPaddleProperty::Process(int timeInMs)
{

	bool powerupsAdjusted = false;
	for (int i = 0; i < powerups.Size(); ++i)
	{
		Powerup & power = powerups[i];
		int secondsElapsed = (Time::Now() - power.startTime).Seconds();
		if (secondsElapsed > power.durationInSeconds)
		{
			powerups.RemoveIndex(i);
			--i;
			powerupsAdjusted = true;
		}
	}
	// Update!
	if (powerupsAdjusted)
	{
		OnPowerupsUpdated();
	}

	List<Entity*> entities = MapMan.GetEntities();

	Time now = Time::Now();
	int secondsSinceLastInput = (now - lastUserInput).Seconds();
	if (secondsSinceLastInput < 1)
	{
		StopMovement();
		std::cout<<"\nStoppping ai movment.";
		return;
	}

	bool moved = false;
	Entity * closestBall = 0;
	float closestDistance = 1000000.f;
	/// Check distance to ball. Fetch closest/most dangerous one!
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * ball = entities[i];
		if (!ball->physics)
			continue;
		BreakoutBallProperty * bbp = (BreakoutBallProperty*) ball->GetProperty("BreakoutBallProperty");
		if (!bbp)
			continue;
		if (bbp->sleeping)
			continue;

		// Check distance.
		Vector3f ballToPaddle = owner->position - ball->position;
		// Ignore balls going away.
		if (ball->physics->velocity.DotProduct(lookAt) > 0)
			continue;
		float distance = AbsoluteValue(ballToPaddle.y);
		if (!ball->physics)
			continue;
		if (distance > 500.f)
			continue;
		if (distance < closestDistance)
		{
			closestBall = ball;
			closestDistance = distance;
		}
	}
	if (closestBall)
	{
		/// Ball on the wya here?!
		if (lookAt.DotProduct(closestBall->physics->velocity) < 0)
		{
			// Head towards it!
			Vector3f ballToPaddle = owner->position - closestBall->position;
			Vector3f toMove = -ballToPaddle;
			toMove.Normalize();
			toMove *= aiSpeed;
			toMove.x += closestBall->physics->velocity.x * 0.5f;
			toMove.y = toMove.z = 0;
			Physics.QueueMessage(new PMSetEntity(PT_VELOCITY, owner, Vector3f(toMove)));
			moved = true;
		}
	}
	if (!moved)
		StopMovement();
	
}

/// Adds the power up, and sets its starting time. Calls OnPowerupsUpdated afterward.
void BreakoutPaddleProperty::AddPowerup(Powerup power)
{
	power.startTime = Time::Now();
	powerups.Add(power);
	OnPowerupsUpdated();
}


// Call after the list changes.
void BreakoutPaddleProperty::OnPowerupsUpdated()
{
	float width = 1.f;
	float speed = 1.f;
	for (int i = 0; i < powerups.Size(); ++i)
	{
		Powerup & power = powerups[i];
		switch(power.type)
		{
			case BreakoutPowerup::WIDER_PADDLE:
				width += 0.5f;
				break;
			case BreakoutPowerup::FASTER_BALLS:
				speed *= 1.5f;
				break;
			case BreakoutPowerup::SLOWER_BALLS:
				speed *= .5f;
				break;
		}
	}
	// Update scale.
	Vector3f scale = initialScale;
	scale.x *= width;
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, owner, scale));

	Physics.QueueMessage(new PMSet(SIMULATION_SPEED, speed));
}
	

void BreakoutPaddleProperty::StopMovement()
{
	Physics.QueueMessage(new PMSetEntity(PT_VELOCITY, owner, Vector3f()));
}
