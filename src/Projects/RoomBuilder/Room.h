/// Emil Hedemalm
/// 2015-05-10
/// A room.

#ifndef ROOM_H
#define ROOM_H

#include "RoomObject.h"
#include "Game/GameVariable.h"
#include <fstream>

class Wall;
class Window;
class Door;

class Room 
{
public:
	String name; // Although not needed.
	Time creationDate; // o.o 
	Vector3f size; // Size in meters. This applies 
	Vector3f position; // o.o If in a house.
	Vector3f rotation; // Could be used later on to tweak rooms for sunlight conditions, etc.
	
	// Default padding distance between the rooms walls and until the center or outer parts of the walls, to account for insulation and wirings, or just thickness of wall material (wood, brick, etc.)
	float wallPadding;

	/// List of all objects. Used for read/write and just general organizing.
	List<RoomObject*> objects;
	
	/// Created/adjusted dynamically.
	List<Wall*> walls;
	List<Window*> windows;
	List<Door*> doors;

	/// Settings for this room. Contains default values for position of Windows, or colors, etc.
	List<GameVariable*> settings;
	
	/// Constructor. Sets up default settings.
	Room();
	virtual ~Room();
		
	/// Call after changing any contents inside and it will re-create the entire room for display.
	void Create();
	/// Adjust based on where camera is.
	void SetCeilingVisibility(bool vis);
	
	/// Returns a list of saved rooms by name.
	static List<String> Rooms();
	/// Saves into an own file based on creation date.
	bool Save();
	/// Loads target room.
	bool Load(String room);
	
	/// File-stream read/write.
	bool WriteTo(std::fstream & fileStream);
	bool ReadFrom(std::fstream & fileStream);
	
private:

	/// All entities created with Create().
	List<Entity*> entities;
};

#endif
