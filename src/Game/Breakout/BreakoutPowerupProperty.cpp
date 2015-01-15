/// Emil Hedemalm
/// 2014-07-24
/// Property for power-ups in a Breakout-type game.

#include "BreakoutPowerupProperty.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Message/MessageManager.h"

#include "Random/Random.h"

Random powerupRandom;

BreakoutPowerupMessage::BreakoutPowerupMessage(Powerup power)
	: Message(MessageType::BREAKOUT_POWERUP), power(power)
{
}


BreakoutPowerupProperty::BreakoutPowerupProperty(Entity * owner)
	: EntityProperty("BreakoutPowerupProperty", ID(), owner)
{
	sleeping = false;
}

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int BreakoutPowerupProperty::ID()
{
	return EntityPropertyID::MINI_GAME_2 + 3;
}


/// If reacting to collisions...
void BreakoutPowerupProperty::OnCollision(Collision & data)
{
	if (sleeping)
		return;

	sleeping = true;

	// should only collide with the player.
	Graphics.QueueMessage(new GMUnregisterEntity(owner));
	Physics.QueueMessage(new PMUnregisterEntity(owner));

	float r = powerupRandom.Randf();

	if (r > 0.8f)
	{
		power.type = BreakoutPowerup::WIDER_PADDLE;
		power.durationInSeconds = 15;
	}
	else if (r > 0.6f)
	{
		power.type = BreakoutPowerup::FASTER_BALLS;
		power.durationInSeconds = 5;
	}
	else if (r > 0.4f)
	{
		power.type = BreakoutPowerup::SLOWER_BALLS;
		power.durationInSeconds = 5;
	}
	else 
		power.type = BreakoutPowerup::MORE_BALLS;
	
	BreakoutPowerupMessage * powerMessage = new BreakoutPowerupMessage(power);
	MesMan.QueueMessage(powerMessage);

}
