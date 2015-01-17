/// Emil Hedemalm
/// 2014-07-27
/// A character.

#include "Character.h"

Character::Character()
{
	movementSpeed = 2.f;
}

/// o.o
bool Character::WriteTo(std::fstream & file)
{
	name.WriteTo(file);
	return true;	
}

bool Character::ReadFrom(std::fstream & file)
{
	name.ReadFrom(file);
	return true;
}


