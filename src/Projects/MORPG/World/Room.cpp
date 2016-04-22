/// Emil Hedemalm
/// 2015-01-20
/// A room o.o

#include "Room.h"

EntryPoint::EntryPoint()
{
	entrance = NULL;
}

EntryPoint::EntryPoint(String name, Vector3i direction, Vector3i position)
: name(name), direction(direction), position(position)
{
	entrance = NULL;
}

Room::Room()
{
	model = NULL;
	scale = Vector3f(1,1,1);
}	

/// Returns a new room, at least or preferably 1x1 in size, with exactly 2 entry points in the desired directions.
Room * Room::NewEntrance(Vector3i position, Vector3i direction)
{
	// Later on, add so it can load from pre-loaded room types.
	Room * room = new Room();
	room->entryPoints.Add(EntryPoint("Entrance entry point", direction, position),
		EntryPoint("Entrance entry point", -direction, position));
	room->points.AddItem(Vector3i(0,0,0));
	return room;
}

EntryPoint * Room::GetEntryPoint(Vector3i direction)
{
	for (int i = 0; i < entryPoints.Size(); ++i)
	{
		EntryPoint & point = entryPoints[i];
		if (point.direction.DotProduct(direction) > 0)
			return & point;
	}
	return NULL;
}

List<Vector3i> Room::GetAbsPoints()
{
	List<Vector3i> absPoints;
	for (int i = 0; i < points.Size(); ++i)
	{
		Vector3i locPoint = points[i];
		// Rotated.
		Vector3i rotated = Matrix4f::InitRotationMatrixY(rotation.y).Product(Vector4f(locPoint));
		Vector3i absSpot = position + rotated;
		absPoints.Add(absSpot);
	}
	return absPoints;
}
