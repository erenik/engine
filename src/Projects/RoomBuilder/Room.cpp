/// Emil Hedemalm
/// 2015-05-10
/// A room.

#include "Room.h"
#include "RoomBuilder.h"

/// Constructor. Sets up default settings.
Room::Room()
{
	size = Vector3f(5.f, 4.f, 2.f);
}

Room::~Room()
{
	objects.ClearAndDelete();
}


/// Call after changing any contents inside and it will re-create the entire room for display.
void Room::Create()
{
	/// Remove old stuff.
	entities.Clear();
	MapMan.DeleteAllEntities();
	
	for (int i = 0; i < objects.Size(); ++i)
	{
		RoomObject * object = objects[i];
		object->Create();
	}
}

/// Adjust based on where camera is.
void Room::SetCeilingVisibility(bool vis)
{
	for (int i = 0; i < objects.Size(); ++i)
	{
		RoomObject * obj = objects[i];
		if (obj->type == RoomObject::CEILING)
			SetVisibility(vis);
	}	
}

/// Returns a list of saved rooms by name.
List<String> Room::Rooms()
{
	
	
}

/// Saves into an own file based on creation date.
bool Room::Save()
{
	
}

/// Loads target room.
bool Room::Load(String room)
{
	
	
}

/// File-stream read/write.
bool Room::WriteTo(std::fstream & fileStream)
{
	
	return true;
}

bool Room::ReadFrom(std::fstream & fileStream)
{
	return true;
}
