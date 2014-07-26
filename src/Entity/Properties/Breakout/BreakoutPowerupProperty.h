/// Emil Hedemalm
/// 2014-07-24
/// Property for power-ups in a Breakout-type game.

#ifndef BREAKOUT_POWERUP_PROPERTY_H
#define BREAKOUT_POWERUP_PROPERTY_H

#include "Entity/EntityProperty.h"
#include "Time/Time.h"

namespace BreakoutPowerup {
	enum breakoutPowerups
	{
		WIDER_PADDLE,
		MORE_BALLS,
		FASTER_BALLS,
		SLOWER_BALLS,
	};
};

#include "Message/Message.h"


struct Powerup 
{
	int type;
	int durationInSeconds;
	Time startTime;
};


class BreakoutPowerupMessage : public Message
{
public:
	BreakoutPowerupMessage(Powerup power);
	Powerup power;
};

class BreakoutPowerupProperty : public EntityProperty
{
public:

	BreakoutPowerupProperty(Entity * owner);

	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);


	void RandomizeType();
	void AssignOwnerModelAndTextureBasedOnType();



	/// p=p;;
	Powerup power;

private:
	// Set to true after the powerup is activated.
	bool sleeping;
};

#endif

