/// Emil Hedemalm
/// 2015-01-20
/// Generator for zone contents and its internal parts.

#include "ZoneGenerator.h"
#include "Room.h"

#include "Model/ModelManager.h"

/// Generates contents for target zone. Neighbours are and type are taken into consideration to some extent?
void ZoneGenerator::GenerateZone(Zone * forZone)
{
	zone = forZone;
	CreateGrid();
	PlaceEntrances();
	AddMainRooms();
	ConnectRooms();
	PlaceBuildingSlots();
}

void ZoneGenerator::CreateGrid()
{
	/// Default 10x10x5 grid-size? Allowing for whatever is wanted and some elevation in between levels, or stuff.
	zone->roomMatrix.defaultValue = NULL;
	zone->roomMatrix.Allocate(Vector3i(30,5,30));
}

void ZoneGenerator::PlaceEntrances()
{
	Vector3i matrixSize = zone->roomMatrix.Size();
	// Check neighbours.
	for (int i = 0; i < zone->neighbours.Size(); ++i)
	{
		Zone * neighbour = zone->neighbours[i];
		// Add entrances.
		Vector3i direction = neighbour->position - zone->position;
		Entrance * entrance = new Entrance();
		entrance->name = "Entrance to neighbour zone";
		entrance->to = Entrance::ZONE;
		entrance->zone = neighbour;
		entrance->direction = direction;
		// Some default positions.
		Vector3i pos;
		if (direction.x > 0)
			pos = Vector3i(matrixSize.x - 1, 0, matrixSize.z * 0.5);
		else if (direction.x < 0)
			pos = Vector3i(0, 0, matrixSize.z * 0.5);
		if (direction.z > 0)
			pos = Vector3i(matrixSize.x * 0.5, 0, matrixSize.z - 1);
		else if (direction.z < 0)
			pos = Vector3i(matrixSize.x * 0.5, 0, 0);
		entrance->position = pos;
		/// Check that there isn't already an entrance with the same position.
		bool bad = false;
		for (int j = 0; j < zone->entrances.Size(); ++j)
		{
			Entrance * e2 = zone->entrances[j];
			if (e2->position == pos)
			{
				bad = true;
				break;
			}
		}
		if (bad)
		{
			assert(false);
			delete entrance;
			continue;
		}
		zone->entrances.Add(entrance);
	}
	// Create rooms for these entrances, make the player activate the transportation 
	// to the other entrance upon stepping into it, or crossing it's center or something?
	for (int i = 0; i < zone->entrances.Size(); ++i)
	{
		Entrance * entrance = zone->entrances[i];
		// Create a room for it.
		Room * room = Room::NewEntrance(entrance->position, entrance->direction);
		assert(room);
		if (!room)
			continue;
		// Place it somewhere, depending on its direction.
		// Get the room entry point which fits the direction.
		EntryPoint * point = room->GetEntryPoint(entrance->direction);
		point->entrance = entrance;
		point->position = entrance->position;
		point->name = "Entry point to entrance/gateway/teleport.";
		// Set position to the entrance..
		room->position = entrance->position;
		// ..and place the room into the zone.
		assert(zone->Place(room));
	}
}

void ZoneGenerator::AddMainRooms()
{
	Vector3i matrixSize = zone->roomMatrix.Size();
	/// 1 big fat room, dynamically generated, all of it, probably.
	AddMaximizedRoom();
}

void ZoneGenerator::AddMaximizedRoom()
{
	Vector3i min, max;
	List<EntryPoint> entryPointsToAdd;
	Vector3f averagePosition;
	for (int i = 0; i < zone->entrances.Size(); ++i)
	{
		Entrance * entrance = zone->entrances[i];
		Vector3i pos = entrance->position;
		if (i == 0)
			min = max = pos;
		else {
			if (pos.x < min.x)
				min.x = pos.x;
			else if (pos.y < min.y)
				min.y = pos.y;
			if (pos.x > max.x)
				max.x = pos.x;
			else if (pos.y > max.y)
				max.y = pos.y;
		}	
		EntryPoint newPoint;
		newPoint.name = "MaximizedRoom EntryPoint";
		newPoint.position = entrance->position - entrance->direction;
		newPoint.direction = entrance->direction;
		entryPointsToAdd.Add(newPoint);
		averagePosition += newPoint.position;
	}
	// Maximize...
	min = Vector3i(1,1,0);
	max = Vector3i(zone->roomMatrix.Size().x, zone->roomMatrix.Size().y, 0) - Vector3i(1,1,0);
	averagePosition = (min + max) * 0.5;
	// Min max ascertained. Create room.
	Vector3f size = max - min; // Add one, so that it includes the edges.
	Room * room = new Room();
	// Set model.
	room->model = ModelMan.GetModel("plane.obj");
	// Set scale (before multiplication due to grid-size outside).
	room->scale = size;
	room->size = size;
	room->position = averagePosition;
	for (int x = min.x; x < max.x; ++x)
	{
		for (int y = min.y; y < max.y; ++y)
		{
			Vector3f pointPosition = Vector3i(x,y,0);
			room->points.Add(pointPosition - room->position);
		}
	} 
	assert(room);
	room->entryPoints = entryPointsToAdd; // Add entry points.
	// ..and place the room into the zone.
	assert(zone->Place(room));
}

void ZoneGenerator::ConnectRooms()
{
	// Connect entry points. Ensure no empty entry points are found, or that they are locked somehow?
}

void ZoneGenerator::PlaceBuildingSlots()
{
	for (int i = 0; i < zone->rooms.Size(); ++i)
	{
		Room * room = zone->rooms[i];
		// If already has building-slots, skip it.
		if (room->buildingSlots.Size())
			continue;
		// Check whether we should add building-slots here.
		if (room->points.Size() < 4) // Skip small rooms.
			continue;
		// Just randomly place some rooms inside..? Fuck.
		List<Vector3i> points = room->GetAbsPoints();
		int buildingsToPlace = 100;
		bool good;
		for (int b = 0; b < buildingsToPlace; ++b)
		{
			Vector3f point;
			int largestSizeShouldPlace;
			int attempts = 0;
			good = false;
			while(true)
			{
				++attempts;
				point = points[rand() % points.Size()];
				Vector3f closestToOtherBuildings(100,100,100);
				for (int j = 0; j < room->buildingSlots.Size(); ++j)
				{
					BuildingSlot * otherSlot = room->buildingSlots[j];
					Vector3f toOtherBuilding = (otherSlot->position - point).Abs();
					toOtherBuilding -= otherSlot->size * 0.5f;
					closestToOtherBuildings = Vector3f::Minimum(closestToOtherBuildings, toOtherBuilding);
				}
				largestSizeShouldPlace = closestToOtherBuildings.MinPart();
				if (largestSizeShouldPlace < 2.f)
				{
					if (attempts > 100)
						break;
					continue;
				}
				good = true;
				break;
			}
			if (!good)
				break;
			largestSizeShouldPlace = min(min(largestSizeShouldPlace,room->size.x), room->size.y);
			int mod = largestSizeShouldPlace - 1;
			if (mod < 2)
				continue;
			BuildingSlot * slot = new BuildingSlot();
			slot->position = point;
			slot->size = Vector3f(rand() % mod + 1, 1, rand() % mod + 1);
			// Check that we have enough points around? lol
			room->buildingSlots.Add(slot);
			// Mark the building in the zone-grid?
			zone->buildingSlots.Add(slot);
		}
		std::cout<<"\n"<<room->buildingSlots.Size()<<" building slots placed.";
	}
}

