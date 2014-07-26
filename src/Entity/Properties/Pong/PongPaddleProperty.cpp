/// Emil Hedemalm
/// 2014-07-17
/// Property for the paddles (players) in a simple Pong-game.
/// Contains the logic for a simple AI which can be toggled.

#include "PongPaddleProperty.h"
#include "PongBallProperty.h"

#include "Maps/MapManager.h"
#include "Entity/Entity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Messages/PhysicsMessage.h"

PongPaddleProperty::PongPaddleProperty(Entity * owner, Vector2f lookAt, float aiSpeed)
	: EntityProperty("PongPaddleProperty", owner), aiSpeed(aiSpeed), lookAt(lookAt)
{
	score = 0;
	lastUserInput = Time::Now();
}

/// Time passed in seconds..!
void PongPaddleProperty::Process(int timeInMs)
{
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
		PongBallProperty * pbp = (PongBallProperty*) ball->GetProperty("PongBallProperty");
		if (!pbp)
			continue;
		if (pbp->sleeping)
			continue;

		// Check distance.
		Vector3f ballToPaddle = owner->position - ball->position;
		// Ignore balls going away.
		if (ball->physics->velocity.DotProduct(lookAt) > 0)
			continue;
		float distance = AbsoluteValue(ballToPaddle.x);
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
			toMove.y += closestBall->physics->velocity.y * 0.5f;
			toMove.x = toMove.z = 0;
			Physics.QueueMessage(new PMSetEntity(VELOCITY, owner, Vector3f(toMove)));
			moved = true;
		}
	}
	if (!moved)
		StopMovement();
	
}


void PongPaddleProperty::StopMovement()
{
	Physics.QueueMessage(new PMSetEntity(VELOCITY, owner, Vector3f()));
}