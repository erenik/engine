/// Emil Hedemalm
/// 2014-07-23
/// Property for the bricks in a Breakout-type game.

#include "BreakoutBrickProperty.h"
//#include "Maps/MapManager.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Random/Random.h"

Random brickRandom;

BreakoutBrickProperty::BreakoutBrickProperty(Entity * owner)
	: EntityProperty("BreakoutBrickProperty", ID(), owner), hitsNeeded(1), score(1), hitsTaken(0),
	sleeping(false)
{
}

/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
int BreakoutBrickProperty::ID()
{
	return EntityPropertyID::MINI_GAME_2 + 1;
}


/// Time passed in seconds..!
void BreakoutBrickProperty::Process(int timeInMs)
{
}

/// If reacting to collisions...
void BreakoutBrickProperty::OnCollision(Collision & data)
{
}

// Maybe spawn some power-ups?
void BreakoutBrickProperty::OnBroken()
{
	if (sleeping)
		return;
	float r = brickRandom.Randf();
	if (r > 0.9f)
	{
		MesMan.QueueMessages("SpawnPowerup:"+String::ToString(owner->position[0])+","+String::ToString(owner->position[1]));
		sleeping = true;
	}
}
