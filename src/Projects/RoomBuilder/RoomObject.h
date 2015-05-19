/// Emil Hedemalm
/// 2015-05-10
/// Any object in the room.

#ifndef ROOM_OBJECT_H
#define ROOM_OBJECT_H

#include "String/AEString.h"
#include "Time/Time.h"
#include "MathLib.h"

class Wall;

/// To be subclassed.
class RoomObject
{
public:
	/// o.o
	enum typesEnum
	{
		NO_TYPE, BAD_TYPE = NO_TYPE,
		FLOOR,
		CEILING,
		WALL,
		DOOR,
		WINDOW,
		APPLIANCE, // Electrical appliances.
		FURNITURE, // Plain ole' wood or stuff.
	};
	int type;
	
	String name; // Although not needed.
	Time creationDate; // o.o 
	Time lastEdit;

	// Size in meters. For static models this is extracted upon creation. For dynamic cube-based ones, this will correspond to Entity scale.
	Vector3f size; 
	Vector3f position; // Where in the room.
	Vector3f rotation; // Rotation in the room. 
	
	String modelSource; // Model source.
	String diffuseSource; // Diffuse texture source.
	String emissiveSource; // For glowy-glowy.
	
	/// Wall it is attached to, if attachable.
	Wall * wall;
	
	/** Template types. Copies of RoomObject instances instead of held in own separate class. 
		Loaded upon startup.
	*/
	static List<RoomObject*> types;
	/// Loads from directory. Tries to parse all .ro files within.
	bool LoadTypesFromDir(String dir);
	bool LoadTypes(String fromFile);
	
	RoomObject();
	virtual ~RoomObject();
	/// Creates entity to represent self. Overload as needed.
	virtual void Create();
	/// Deletes all entities created associated with this object. To be called only when editing this object.
	void Delete();
	void SetVisibility(bool val);
	/// Checks if target entity belongs to this object. Used for raycasting moving operations.
	bool BelongsTo(Entity * entity);
	
		
private:
	/// If just one is enough.
	Entity * entity;
	/// If several are needed to represent adequately (like tiled wall-segments).
	List<Entity*> entities;
};

#endif
