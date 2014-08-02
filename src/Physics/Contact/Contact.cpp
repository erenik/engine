/// Emil Hedemalm
/// 2013-10-23
/// For keeping track of stuff between entities.

#include "Contact.h"
#include "Physics/PhysicsProperty.h"
#include "Entity/Entity.h"

Contact::Contact(){
	one = two = NULL;
};

Contact::Contact(Entity * one, Entity * two): one(one), two(two){
	
};

 /// Eased testing test.
bool Contact::IsPartOf(Entity * entity){ 
	if (entity == one || entity == two) 
		return true; 
	return false;
};

/// Disconnects the contact from both entities, depending on type.. and... stuff..
void Contact::Disconnect(){
	/// 
	switch(type){
		case RESTING_CONTACT:
			one->physics->state &= ~PhysicsState::IN_REST;
			two->physics->state &= ~PhysicsState::IN_REST;
			break;
	}
	
	one->physics->contacts.Remove(this);
	two->physics->contacts.Remove(this);
	one = two = NULL;
}