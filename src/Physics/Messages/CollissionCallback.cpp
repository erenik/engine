// Emil Hedemalm
// 2013-06-15

#include "CollissionCallback.h"
#include "Message/MessageTypes.h"
#include "Physics/Collission/Collission.h"

CollissionCallback::CollissionCallback() : Message(MessageType::COLLISSION_CALLBACK) {
};
CollissionCallback::~CollissionCallback() { 
	if (data){
		delete (Collission*)data;
		data = NULL;
	}
};