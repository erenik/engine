// Emil Hedemalm
// 2013-06-15

#include "CollisionCallback.h"
#include "Message/MessageTypes.h"
#include "Physics/Collision/Collision.h"

CollisionCallback::CollisionCallback() : Message(MessageType::COLLISSION_CALLBACK) 
{
	data = NULL;
};

CollisionCallback::CollisionCallback(Collision c)
: Message(MessageType::COLLISSION_CALLBACK), one(c.one), two(c.two), impactVelocity(c.collisionVelocity)
{
	data = NULL;
}

CollisionCallback::~CollisionCallback() 
{ 
	if (data){
		delete (Collision*)data;
		data = NULL;
	}
};