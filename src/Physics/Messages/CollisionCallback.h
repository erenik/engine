// Emil Hedemalm
// 2013-06-15

#ifndef COLLISSION_CALLBACK_H
#define COLLISSION_CALLBACK_H

#include "Message/Message.h"
#include "MathLib.h"

class Entity;

/** Message for passing data~
	Do note that the "data" member should be deleted after processing the message as it is
	Created on a per-message basis!
*/
class CollisionCallback : public Message
{
public:
	CollisionCallback();
	CollisionCallback(Collision c);
	//CollisionCallback(Entity* one, Entity* two);
	virtual ~CollisionCallback();

	// Extra variables
	Entity* one;
	Entity* two;
	Vector3f collissionPoint;
	Vector3f impactNormal;
	float impactVelocity;

};

#endif

