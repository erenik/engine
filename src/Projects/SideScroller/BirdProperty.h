/// Emil Hedemalm
/// 2015-05-08
/// Yar

#ifndef BIRD_PROPERTY_H
#define BIRD_PROPERTY_H

#include "Entity/EntityProperty.h"

class BirdProperty : public EntityProperty
{
public:
	BirdProperty(Entity * owner);
	virtual void OnCollision(Collision & data);	
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);
	virtual void OnCollisionCallback(CollisionCallback * cc);

	/// Setups physics, collision settings, etc.
	void SetupForFlight();

private:
	bool collided;

};

#endif
