/// Emil Hedemalm
/// 2014-08-26
/// Elements o-o

#include "Elements.h"

int GetElementByString(String str)
{
	for (int i = 0; i < Element::NUM_ELEMENTS; ++i)
	{
		String elementName = GetElementString(i);
		if (elementName.Length() == 0)
			continue;
		if (str.Contains(elementName))
			return i;
	}	
	return -1;
}

String GetElementString(int element)
{
	switch(element)
	{
		case Element::MAGIC: return "Magic";
		case Element::FIRE: return "Fire";
		case Element::WATER: return "Water";
		case Element::EARTH: return "Earth";
		case Element::AIR: return "Air";
		case Element::BALANCE: return "Balance";
		case Element::CHAOS: return "Chaos";
		case Element::LIFE: return "Life";
		case Element::DEATH: return "Death";
	}
	return String();
}

