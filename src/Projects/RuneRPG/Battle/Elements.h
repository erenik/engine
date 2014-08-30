/// Emil Hedemalm
/// 2014-08-26
/// Elements o-o

#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "String/AEString.h"

namespace Element 
{
	enum elements
	{
		BAD_ELEMENT = -1,
		MAGIC, // Non-elemental magic.
		FIRE, 
		WATER, 
		EARTH,
		AIR,
		LIFE,
		DEATH,
		CHAOS,
		BALANCE,
		NUM_ELEMENTS,
		PHYSICAL, // Although not technically an element, it's stored here to preserve space but also make it easier to manage.
	};
};

int GetElementByString(String str);
String GetElementString(int element);

#endif

