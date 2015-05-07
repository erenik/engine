/// Emil Hedemalm
/// 2015-04-20
/// Player property.

#ifndef LUCHADOR_H
#define LUCHADOR_H

#include "Entity/EntityProperty.h"

class Entity;
struct Collision;
class Message;
class CollisionCallback;

class LuchadorProperty : public EntityProperty 
{
public:
	LuchadorProperty(Entity * owner);
	virtual void OnCollision(Collision & data);	
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);
	virtual void OnCollisionCallback(CollisionCallback * cc);

	void Run();
	/// Stops velocity and acceleration.
	void Stop();
	void BuyTaco();
protected:
	
	void UpdateVelocity();
	void SetSpeedBonusDueToTacos(float newBonus);

	bool sleeping;
	Time lastJump;
	/// o.o
	Time timeSinceLastTaco;
	/// Multiplicative., 1.0 default.
	float speedBonusDueToTacos;

	/// o.o after fall/run?
	bool autoRun;
	int state;
	enum {
		STOPPED,
		JUMPING,
		FALLING,
		RUNNING
	};
};

#endif
