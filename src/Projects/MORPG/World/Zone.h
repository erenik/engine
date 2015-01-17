/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#ifndef ZONE_H
#define ZONE_H

#include "String/AEString.h"
#include "MathLib.h"
#include "Maps/Map.h"

class Nation;
class Building;
class Character;
class CompactEntity;

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

class Zone : public Map
{
public:
	Zone();

	void Nullify();

	/// Takes all models this zone is composed of and creates it for you. Will also create all characters within (hopefully including you!)
	void CreateEntities();

	String name;
	int id;

	/// Position on the grid/world-map.
	Vector3i position;

	/// If this zone is to be painted on a map, what color would it be?
	Vector4f GetColor();


	void SetWater(bool w);
	void SetMountain(bool m);

	bool IsWater();
	bool IsMountain();

	virtual bool WriteTo(std::fstream & file);
	virtual bool ReadFrom(std::fstream & file);

	/// Current characters within this zone.
	List<Character*> characters;
	/// Neighbour-zones.
	List<Zone*> neighbours;

	/// Slots where buildings could be placed.
	List<BuildingSlot*> buildingSlots;

	/// Buildings within this zone, as buildings may be present even in non-settlements... e.g. ruins... or?
	List<Building*> buildings;

	/// Who has control over this zone? Anyone?
	Nation * controllingNation;
	int controllingNationID;

	/// o.o 0 = water-line, 1 = land, 0.1 to 0.9 = Beach?, 2 = hills, 3+ = mountains
	float elevation;

protected:
	/// All entities which comprise the "base" along which we will walk and physicall interact with on the most part. 
//	List<CompactEntity*> compactBaseEntities;


private:
	bool isWater;
	bool isMountain;
};

#endif



