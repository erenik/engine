/// Emil Hedemalm
/// 2014-07-27
/// A character.

#ifndef CHARACTER_H
#define CHARACTER_H

#include "MathLib.h"
#include "String/AEString.h"
#include "List/List.h"
#include <utility>

class Item;

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
	// Determines how it is to play them. Used actively in the MORPGCharacterProperty. Default while testing: 2. meters per second.
	float movementSpeed;
	
	/// List of skills, sorted.
	List<std::pair<int,char>> skills;
	List<std::pair<int,char>> availableSkills; // Skills available for current level/class combination, includes both skill # and level of the skill.
	List<std::pair<int,int>> skillEXP; // Array of all skills being trained.
	int skillTraining; // Integer which skill is currently being trained.
	
	/// Skills currently being evaluated each frame.
	List<int> activeSkills;
	List<std::pair<int,int>> skillCooldownsMs; // Cooldown of skills, in milliseconds. If not in array, skill should be executable.
	
	int money; // Money in total on character.
	List<std::pair<Item*, int>> inventory; // Total amount of items.
};

#endif
