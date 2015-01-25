/// Emil Hedemalm
/// 2015-01-20
/// Part of a zone.

#include "Zone.h"

#ifndef ROOM_H
#define ROOM_H

class Entrance 
{
public:
	enum {
		NONE,
		ROOM, 
		ZONE,
	};
	String name;
	// See above, ROOM or ZONE.
	int to;
	// If ZONE, will link to other ZONE this connects to.
	Zone * zone;
	// If ROOM, will link to other room, acting as an in-zone teleporter?
	Room * room;
	/**	If ZONE, will link to the Entrance module located in the other zone, 
		so that the character may be placed at the correct position (specially true if multiple entrances link the 2 zones).
	*/
	Entrance * entrance;
	// In the grid, absolute direction.
	Vector3i direction;
	/// in the grid, absolute position.
	Vector3i position;
};

class EntryPoint 
{
public:
	EntryPoint();
	EntryPoint(String name, Vector3i direction, Vector3i position = Vector3i(0,0,0));
	String name;
	/// Entrance added here.
	Entrance * entrance;
	/// Local position, relative to room position.
	Vector3i position;
	/** Local direction to enter this point, implies destination room or zone direction. Entering into this point will use the refersed direction.
		If room it is located in was rotated, this direction may have to be rotated too.
	*/
	Vector3i direction;
};


class Room 
{
public:
	Room();
	/// Returns a new room, at least or preferably 1x1 in size, with exactly 2 entry points in the desired directions.
	static Room * NewEntrance(Vector3i position, Vector3i direction);
	EntryPoint * GetEntryPoint(Vector3i direction);
	List<Vector3i> GetAbsPoints();

	/// Position in the grid.
	Vector3f position;
	/// Scale
	Vector3f scale;
	/// If rotated.
	Vector3f rotation; 
	/// Local AABB size in X, Y and Z.
	Vector3i size;
	/// Local points this room occupies.
	List<Vector3f> points;
	/// To other rooms, or to another zone entirely?
	List<Entrance*> entrances;
	/// Points where other rooms may be connected or entrances may be placed.
	List<EntryPoint> entryPoints;
	// For pathfinding and stuff.
	List<Room*> neighbours;

	List<BuildingSlot*> buildingSlots;

	/// Model to be used.
	Model * model;

private:
	/// Loaded pre-defined rooms.
	List<Room*> types;
};

#endif
