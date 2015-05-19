/// Emil Hedemalm
/// 2015-05-10
/// Any object in the room.

#include "RoomObject.h"
#include "RoomBuilder.h"

/// o.o
bool RoomObject::LoadTypesFromDir(String dir)
{
	List<String> files;
	bool ok = GetFilesInDirectory(dir, files);
	if (!ok)
	{
		LogMain("Bad directory: "+dir, ERROR);
		return false;
	}
	for (int i = 0; i < files.Size(); ++i)
	{
		String path = dir + "/" + files[i];
		bool ok = LoadTypes(path);
		if (!ok)
		{
			return false;
		}
	}
	return success;
}

/// Loads from directory. Tries to parse all .ro files within.
bool RoomObject::LoadTypes(String fromFile)
{
	RoomObject * object = NULL;
	List<String> lines = File::GetLines(fromFile);
	if (lines.Size() == 0)
	{
		LogMain("No lines in file "+fromFile, ERROR);
		return false;
	}
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		List<String> spaceTokenized = line.Tokenize(" ");
		String arg, value;
		if (spaceTokenized.Size() > 1)
		{
			arg = spaceTokenized[0];
			value = spaceTokenized[1];
		}
		if (arg.Length() < 1)
			continue; 
#define ADD_TO_LIST_IF_NEEDEd if (object) types.AddItem(object); object = NULL;
		if (arg == "Object" || arg == "RoomObject")
		{
			ADD_TO_LIST_IF_NEEDEd
			object = new RoomObject();
		}
		else if (arg == "")
		
	}	
	return true;
}


RoomObject::RoomObject()
{
	type = NO_TYPE;
	creationDate = lastEdit = Time::Now();
	modelSource = "cube";
	diffuseSource = "0xAAFF";
	size = Vector3f(1,1,1);
}
			
/// Creates entity to represent self.
void RoomObject::Create()
{
	entity = EntityMan.CreateEntity("o.o", ModelMan.GetModel(modelSource), TexMan.GetTexture(diffuseSource));
	// Set position and other physical properties.
	entity->position = position;
	entity->rotation = rotation;
	entity->scale = size;
	// Add for rendering.. and physics so raycasting works.
	MapMan.AddEntity(entity);
	entities.AddItem(entity);
};

/// Deletes all entities created associated with this object. To be called only when editing this object.
void RoomObject::Delete()
{
	MapMan.DeleteEntities(entities);
	entities.Clear();
}

void RoomObject::SetVisibility(bool val)
{
	QueueGraphics(new GMSetEntity(entities, GT_VISIBILITY, val));
}

bool RoomObject::BelongsTo(Entity * e)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		if (entities[i] == e)
			return true;
	}
	return false;
}

void 

