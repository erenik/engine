/// Emil Hedemalm
/// 2014-07-27
/// A character.

#ifndef CHARACTER_H
#define CHARACTER_H

#include "MathLib.h"
#include "String/AEString.h"

/// Class used to store both npc- and player data! Any character should be playable, or at least jump-in and playable by e.g. GMs for testing purposes.
class Character 
{
public:
	Character();
	/// o.o
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);
	String name;
	Vector3f position;
	// Determines how it is to play them. Used actively in the MORPGCharacterProperty. Default while testing: 2.
	float movementSpeed;

};

#endif
