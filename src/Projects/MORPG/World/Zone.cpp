/// Emil Hedemalm
/// 2014-07-27
/// A single zone within the world.

#include "Zone.h"

#include "Entity/Entity.h"
#include "Entity/CompactEntity.h"
#include "Entity/EntityManager.h"

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
}

Zone::~Zone()
{
	buildingSlots.ClearAndDelete();
	buildings.ClearAndDelete();
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


/// Takes all models this zone is composed of and creates it for you. Will also create all characters within (hopefully including you!)
void Zone::CreateEntities()
{
	// Deletes and re-creates entities as needed.
	LoadFromCompactData();
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

