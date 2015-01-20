/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#include "Zone.h"

#include "Entity/Entity.h"
#include "Entity/CompactEntity.h"
#include "Entity/EntityManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Messages/GMCamera.h"

#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Room.h"

#include "WorldMap.h"

Zone::Zone()
{
	Nullify();
}

void Zone::Nullify()
{
	isWater = false;
	isMountain = false;
	elevation = 1.f;
	hasSettlement = false;
	inhabitants = 0;
	roomGridSize = 1.f;
	camera = NULL;
}

Zone::~Zone()
{
	buildingSlots.ClearAndDelete();
	buildings.ClearAndDelete();
	entrances.ClearAndDelete();
	rooms.ClearAndDelete();
}

/// Usually the most important building.
Entity * Zone::CreateWorldMapRepresentation()
{
	// Get the coolest house in town.
	if (buildings.Size() == 0)
		return NULL;
	Building * building = buildings[0];
	Entity * entity = EntityMan.CreateEntity(name, building->model, building->texture);
	if (entity == NULL){
	    std::cout<<"\nERROR: MapManager::CreateEntity:Unable to create entity, returning.";
        return NULL;
	}
	entity->position = position;
	entity->RecalculateMatrix();
	return entity;
}

BuildingSlot * Zone::GetFreeBuildingSlot()
{
	for (int i = 0; i < buildingSlots.Size(); ++i)
	{
		BuildingSlot * slot = buildingSlots[i];
		if (slot->building == NULL)
			return slot;
	}
	return NULL;
}

/// Places room into the zone, adding it to the grid and list.
bool Zone::Place(Room * room)
{
	// Get elements it would occupy.
	List<Vector3i> spotsToOccupy = room->GetAbsPoints();
	for (int i = 0; i < spotsToOccupy.Size(); ++i)
	{
		Vector3i spot = spotsToOccupy[i];
		if (roomMatrix.At(spot))
			return false;
	}
	for (int i = 0; i < spotsToOccupy.Size(); ++i)
	{
		Vector3i spot = spotsToOccupy[i];
		roomMatrix.Set(spot, room);
	}
	rooms.Add(room);
	return true;
}

// Registers all entities for display and makes the world-map camera active.
void Zone::MakeActive()
{
	// Unregister all current entities from graphics rendering.
	GraphicsMan.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
	// Create camera if needed.
	if (!camera){
		camera = CameraMan.NewCamera();
		// One-time setup of camera.
		camera->movementType = CAMERA_MOVEMENT_ABSOLUTE;
		camera->absForward = Vector3f(0,0,-1);
		camera->absRight = Vector3f(1,0,0);
		camera->absUp = Vector3f(0,1,0);
		// Create the default camera-position too.
		Camera * resetCamera = CameraMan.NewCamera();
		camera->resetCamera = resetCamera;
	}
	GraphicsMan.QueueMessage(new GMSetCamera(camera));
	// o.o
	if (entities.Size() == 0)
	{
		CreateEntities();
		// Update the reset-camera's position and stuffs! o.o
		Vector3f position = worldMap.FromWorldToWorldMap(roomMatrix.Size() * 0.5f, 0.f);
		camera->resetCamera->position = position + Vector3f(0, 20.f, 20.f);	 
		camera->resetCamera->rotation = Vector3f(0.9f, 0, 0);
		camera->resetCamera->flySpeed = roomMatrix.Size().Length() * 0.1f;
		// And reset it.
		camera->Reset();
	}
	GraphicsMan.QueueMessage(new GMRegisterEntities(entities));
}


/// Takes all models this zone is composed of and creates it for you. Will also create all characters within (hopefully including you!)
void Zone::CreateEntities()
{
	// Create all entities as the rooms and character data specify them at the moment!
	for (int i = 0; i < rooms.Size(); ++i)
	{
		Room * room = rooms[i];
		if (room->model)
		{
			Entity * entity = EntityMan.CreateEntity("Room", room->model, TexMan.GenerateTexture(Vector4f(1,1,1,1)));
			Vector3f worldPos = worldMap.FromWorldToWorldMap(room->position);
			entity->position = worldPos * roomGridSize;
			entity->scale = worldMap.FromWorldToWorldMap(room->scale);
			entity->RecalculateMatrix();
			entities.Add(entity);
		}
		else 
		{
			// Default, just create a bunch of planes.
			List<Vector3i> points = room->GetAbsPoints();
			for (int j = 0; j < points.Size(); ++j)
			{
				Entity * entity = EntityMan.CreateEntity("Room placeholder plate", ModelMan.GetModel("Plane.obj"), TexMan.GenerateTexture(Vector4f(1,1,1,1)));
				Vector3f rawPosition = points[j];
				Vector3f worldPos = worldMap.FromWorldToWorldMap(rawPosition);
				entity->position = worldPos * roomGridSize + Vector3f(0.5f,0,0.5f); // Offset so that it is inside the grid correctly.
				entity->RecalculateMatrix();
				entities.Add(entity);
			}
		}
		// Visualize the room's entry-points.
		for (int j = 0; j < room->entryPoints.Size(); ++j)
		{
			EntryPoint & point = room->entryPoints[j];
			Vector3f dir = point.direction;
			Texture * tex = TexMan.GenerateTexture(Vector4f(dir.Abs(), 1.f));
			Entity * entry = EntityMan.CreateEntity("Entry point "+String(j), ModelMan.GetModel("sphere.obj"), tex);
			Vector3f worldPos = worldMap.FromWorldToWorldMap(point.position);
			// Plus some offset.
			Vector3f offset = worldMap.FromWorldToWorldMap(point.direction) * 0.2f;
			entry->position = worldPos + offset + Vector3f(0.5f,0.5f,0.5f);
			entry->scale = Vector3f(1,1,1) * 0.2f;
			entry->RecalculateMatrix();
			entities.Add(entry);
		}
	}
	// Deletes and re-creates entities as needed.
//	LoadFromCompactData();
}

/// If this zone is to be painted on a map, what color would it be?
Vector4f Zone::GetColor()
{
	/// o.o 0 = water-line, 1 = land, 0.1 to 0.9 = Beach?, 2 = hills, 3+ = mountains
	if (elevation <= -1.f)
	{
		return Vector3f(0,0, elevation * 0.5f + 1.f);
	}
	/// Shallow waters
	else if (elevation < 0.f)
	{
		// Lagoons?
		if (elevation > -0.1f)
			return Vector3f(0,1,1);
		// Regular waters.
		return Vector3f(0.2f, 0.5f, 1.f) * (1 + elevation) + Vector3f(0,0,1);
	}
	// Beach.
	else if (elevation < 0.5f)
	{
		return Vector3f(1.f, 1.f, 0.f);
	}
	// Grass to hills?
	else if (elevation < 2.f)
	{
		return Vector3f(0,1.f,0.f);
	}
	/// Hills to Mountains o.o
	else if (elevation < 3.f)
		return Vector3f(1,1,1) * 0.5f;
	// Mountains and snowy peaks!
	else 
	{
		return Vector3f(1,1,1);
	}
	return Vector3f(0,1,0);
}

bool Zone::IsWater()
{
	return isWater;
};

bool Zone::IsMountain()
{
	return isMountain;
}

void Zone::SetWater(bool w)
{ 
	isWater = w;
	elevation = -2.f;
};

void Zone::SetMountain(bool m)
{
	isMountain = m;
	elevation = 3.f;
}



bool Zone::WriteTo(std::fstream & file)
{
	name.WriteTo(file);
	position.WriteTo(file);
	file.write((char*) &elevation, sizeof(float));
//	std::cout<<"\nWrite zone "<<name<<" with elevation "<<elevation;
	return true;
}
bool Zone::ReadFrom(std::fstream & file)
{
	name.ReadFrom(file);
	position.ReadFrom(file);
	file.read((char*) &elevation, sizeof(float));
//	std::cout<<"\nRead zone "<<name<<" with elevation "<<elevation;
	return true;
}

