/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#ifndef ZONE_H
#define ZONE_H

#include "String/AEString.h"
#include "MathLib.h"

class Nation;
class Building;
class Character;

struct BuildingSlot 
{
	/// 
	Character * owner;
	// If for sale, price >= 0 if not for sale, price < 0
	int price;

	/// Size dimensions in x and y.
	Vector2i size;
	/// 3D position or use grid position..?
	Vector3f position;
};

class Zone 
{
public:
	Zone();

	String name;
	int id;

	/// Position on the grid/world-map.
	Vector3i position;

	/// If this zone is to be painted on a map, what color would it be?
	Vector4f GetColor();

	bool IsWater();
	void SetWater(bool w);
	bool IsMountain();

	virtual bool WriteTo(std::fstream & file);
	virtual bool ReadFrom(std::fstream & file);

	/// Current characters within this zone.
	List<Character*> characters;


	/// Slots where buildings could be placed.
	List<BuildingSlot*> buildingSlots;

	/// Buildings within this zone, as buildings may be present even in non-settlements... e.g. ruins... or?
	List<Building*> buildings;

	/// Who has control over this zone? Anyone?
	Nation * controllingNation;
	int controllingNationID;

private:
	bool isWater;
	bool isMountain;
};

#endif



