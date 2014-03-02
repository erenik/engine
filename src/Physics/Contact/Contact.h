/// Emil Hedemalm
/// 2013-10-23
/// For keeping track of stuff between entities.

#ifndef CONTACT_H
#define CONTACT_H

#include "Entity/Entity.h"

enum contactTypes{
	NULL_CONTACT,
	RESTING_CONTACT
};

struct Contact {
	Contact();
	Contact(Entity * one, Entity * two);
	/// Eased testing test.
	bool IsPartOf(Entity * entity);
	/// Disconnects the contact from both entities, depending on type.. and... stuff..
	void Disconnect();
	Entity * one, * two;
	/// Only resting contact used for now.
	int type;
};

struct RestingContact : public Contact {
	RestingContact(Entity * one, Entity * two) : Contact(one,two){	
		type = RESTING_CONTACT;
	};
};

#endif