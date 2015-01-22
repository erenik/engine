// Emil Hedemalm
// 2013-06-15

#include "CollisionCallback.h"
#include "Message/MessageTypes.h"
#include "Physics/Collision/Collision.h"

CollisionCallback::CollisionCallback() : Message(MessageType::COLLISSION_CALLBACK) 
{
	data = NULL;
};

CollisionCallback::CollisionCallback(Entity * one, Entity * two)
: Message(MessageType::COLLISSION_CALLBACK), one(one), two(two)
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