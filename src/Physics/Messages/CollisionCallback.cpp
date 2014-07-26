// Emil Hedemalm
// 2013-06-15

#include "CollisionCallback.h"
#include "Message/MessageTypes.h"
#include "Physics/Collision/Collision.h"

CollisionCallback::CollisionCallback() : Message(MessageType::COLLISSION_CALLBACK) {
};
CollisionCallback::~CollisionCallback() { 
	if (data){
		delete (Collision*)data;
		data = NULL;
	}
};