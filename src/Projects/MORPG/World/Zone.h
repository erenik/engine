/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#ifndef ZONE_H
#define ZONE_H

#include "String/AEString.h"
#include "MathLib.h"
#include "Maps/Map.h"
#include "Building.h"
#include "Matrix/Matrix.h"

class Nation;
class Character;
class CompactEntity;

class Zone;
class Room;
class Entrance;

class Zone : public Map
{
public:
	Zone();
	virtual ~Zone();
	void Nullify();

	/// Usually the most important building. Use a command to add the entity to a map, as that is not done in this function.
	Entity * CreateWorldMapRepresentation();
	BuildingSlot * GetFreeBuildingSlot();
	/// Places room into the zone, adding it to the grid and list.
	bool Place(Room * room);

	// Registers all entities for display and makes the world-map camera active.
	void MakeActive();

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

	/// Per-zone camera. Maybe too much? 
	Camera * camera;

	/// Number of inhabitants. This will be used as a base for how many interactable characters are actually visible and interactable in-game.
	int numInhabitants;
	/// Current characters within this zone.
	List<Character*> characters;
	/// Characters which regard this zone as their home, and stay here most of the time (for NPCs, anyway).
	List<Character*> inhabitants;
	/// Neighbour-zones.
	List<Zone*> neighbours;

	/// Slots where buildings could be placed.
	List<BuildingSlot*> buildingSlots;

	/// "Rooms" in the Zone-grid. Used to internally divide the zone when generating it, possibly used for pathfinding optimization too?
	List<Room*> rooms;
	/// XY Grid linked to the rooms.
	Matrix<Room*> roomMatrix;
	/// Meters per grid element. Default 1.0 (for in-door zones). Use higher values of e.g. 3.0 or upward for outdoor larger zones.
	float roomGridSize;
	/// Entrances to this zone.
	List<Entrance*> entrances;

	/// Buildings within this zone, as buildings may be present even in non-settlements... e.g. ruins... or?
	List<Building*> buildings;

	/// Who has control over this zone? Anyone?
	Nation * controllingNation;
	int controllingNationID;

	/// o.o 0 = water-line, 1 = land, 0.1 to 0.9 = Beach?, 2 = hills, 3+ = mountains
	float elevation;

	/// Just based on elevation.
	bool isWater;
	bool isHighAltitude;

	/// Based on neighbours.
	/// Land types.
	bool isCostal;
	bool isIsland;
	bool isMountain; // Surrounded by lower zones.
	bool isPit;	// Surrounded by taller zones.
	/// Water types.
	bool isLagoon; // All but one entry to this zone is land-based.

	/// If it has pre-built buildings and inhabitants. The settlement need not cover much at all of the zone's area.
	bool hasSettlement;

protected:
	/// All entities which comprise the "base" along which we will walk and physicall interact with on the most part. 
//	List<CompactEntity*> compactBaseEntities;


private:
};

#endif



