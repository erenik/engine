/// Emil Hedemalm
/// 2015-05-06
/// Ramp prop

#ifndef RAMP_H
#define RAMP_H

#include "Entity/EntityProperty.h"

class Entity;
struct Collision;
class Message;
class CollisionCallback;

class RampProp : public EntityProperty 
{
public:
	RampProp(Entity * owner);
	virtual void OnCollision(Collision & data);	
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);
	virtual void OnCollisionCallback(CollisionCallback * cc);
	// Velocity Bonus?
	float speedBonus;
	/// Times before sleeping.
	int totalSpeedBonuses;
	int speedBonusesGiven;
protected:
	bool sleeping;
};

#endif

