/// Emil Hedemalm
/// 2013-10-23
/// For keeping track of stuff between entities.

#ifndef CONTACT_H
#define CONTACT_H

class Entity;

#include "MathLib.h"

enum contactTypes{
	NULL_CONTACT,
	RESTING_CONTACT
};

/// Used for some collision systems, but also the Raycasting system within the Physics system.
struct Contact 
{
	Contact();
	Contact(Entity * one, Entity * two);
	/// Eased testing test.
	bool IsPartOf(Entity * entity);
	/// Disconnects the contact from both entities, depending on type.. and... stuff..
	void Disconnect();
	Entity * one, * two;
	/// Only resting contact used for now.
	int type;
	/// Position of the contact, if any.
	Vector3f position;
};

struct RestingContact : public Contact {
	RestingContact(Entity * one, Entity * two) : Contact(one,two){	
		type = RESTING_CONTACT;
	};
};

#endif