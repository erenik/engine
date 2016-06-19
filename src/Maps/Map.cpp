/// Emil Hedemalm
/// 2014-06-15
/// Map class for grouping entities, lighting, events and scripts together.

#include "Map.h"

#include "../Entity/EntityManager.h"
extern EntityManager entityManager;

#include "Graphics/GraphicsManager.h"
extern GraphicsManager graphics;

#include "../Entity/CompactEntity.h"
#include "Entity/EntityProperty.h"
#include "Script/Script.h"
#include "Pathfinding/NavMesh.h"
#include <cstring>
#include "Physics/PhysicsManager.h"

#include "Model/ModelManager.h"
#include "TextureManager.h"

Map::Map()
{
	this->onEnter = NULL;
	this->onExit = NULL;
	numEvents = 0;
//	lighting.CreateDefaultSetup();
//	lighting.SetAmbient(1,1,1);
	mapDataOK = true;
	// Reset the file header data.
	memset(&header, 0, sizeof(MapFileHeader));
	// Flag it's data as OK again
	mapDataOK = true;
	creator = "Implement";
	navMesh = NULL;
	source = "GameEngine created";
	mapType = MAP_TYPE_3D;
	active = false;
}

Map::~Map(){
	navMesh = NULL;
}

/** Adds target entity to the map. */
bool Map::AddEntity(Entity * i_entity){
	assert(entities.Size() < MAX_ENTITIES_PER_MAP);
	assert(entities.Add(i_entity));
	return true;
}

/// Adds target event to the map.
bool Map::AddEvent(Script * event){
	assert(events.Size() < MAX_EVENTS_PER_MAP);
	for (int i = 0; i < events.Size(); ++i){
		if (events[i]->name == event->name){
			std::cout<<"\nINFO: An event with the given name "<<event->name<<" already exists.";
			return false;
		}
	}
	assert(events.Add(event));
	return true;
}

/** Removes target entity from the map. If the map is active the entity will also be de-registered from graphics/physics/etc.? */
bool Map::RemoveEntity(Entity * entity)
{
	entities.Remove(entity);
	if (active)
	{
		PhysicsMan.QueueMessage(new PMUnregisterEntity(entity));
		GraphicsMan.QueueMessage(new GMUnregisterEntity(entity));
	}
	return true;
}

/** Removes target entities from the map. Returns number of failed removals. */
int Map::RemoveEntities(Entities entitySelection){
	int failed = entities.Size();
	int previousMax = entities.Size();
	for (int j = 0; j < entitySelection.Size(); ++j){
		assert(RemoveEntity(entitySelection[j]));
		--failed;
	}
	return failed;
}
/// Deletes all entities from the map.
void Map::RemoveAllEntities(){
	while(entities.Size())
		RemoveEntity(entities[0]);
}

/// Calculates AABB for all entities. 
AABB Map::CalcAABB()
{
	AABB aabb;
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		aabb.Expand(*entity->aabb);
	}
	return aabb;
}

Entity * Map::GetEntity(String byName){
	for (int i = 0; i < entities.Size(); ++i){
		if (entities[i]->name == byName)
			return entities[i];
	}
	return NULL;
}

/** Returns a list of entities in the map. */
List<Entity*> Map::GetEntities(){
	return entities;
}
List<Script*> Map::GetEvents(){
	return events;
}

/// Fetches last error string, resetting it upon use.
String Map::GetLastErrorString()
{
	String s = lastErrorString;
	lastErrorString = String();
	return s;
}

/// Deletes all entities within the map and re-loads/re-creates them. The entities however are NOT queued to be rendered or participate in physics straight away.
void Map::LoadFromCompactData()
{
	std::cout<<"\nMap::LoadFromCompactData for map: "<<name;

	if (NumEntities() > 0){
		std::cout<<"ERROR: Map has "<<NumEntities()<<" remaining entities. Delete these before reloading from compact map data!";
		RemoveAllEntities();
		assert(NumEntities() == 0);
	//	return;
	}
	/// TODO: Make function of creating stuff from compact format
	int entitiesToCreate = cEntities.Size();
	if (entitiesToCreate == 0){
		std::cout<<"\nNo entities in map compact data, skipping LoadFromCompactData";
		return;
	}
	assert(entitiesToCreate > 0);
	std::cout<<"\nCreating entities from compact file format...";
	/// Convert cEntities to regular entities, etc.
	for (int i = 0; i < entitiesToCreate; ++i)
	{
		CompactEntity * cEntity = cEntities[i];
		/// Get model and texture...
		Model * model = ModelMan.GetModel(cEntity->model);
		Texture * texture = TexMan.GetTextureBySource(cEntity->diffuseMap);
		if (!model){
			std::cout<<"\nWARNING: Unable to locate model, skipping entity.";
			continue;
		}
		/// Ask entity manager to create them :P
		Entity * newEntity = EntityMan.CreateEntity(cEntity->name, model, texture);
		newEntity->LoadCompactEntityData(cEntity);
		/// Add them to the map ^^
		AddEntity(newEntity);
	}
}


void Map::RemoveAllEvents(){
	events.ClearAndDelete();
}
/* Entities Map::GetEntities(){
	return Entities(entities.GetArray(), entities.Size());
}
*/

// Process called each game loop by the stateManager
void Map::Process(int timePassedInMs)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (entity->sharedProperties)
			continue;
		for (int j = 0; j < entity->properties.Size(); ++j)
		{
			entity->properties[j]->Process(timePassedInMs);
		}
	}
}

/// Parse model and texture dependencies if they were not included in the file upon loading!
bool Map::ParseDependencies()
{
	modelsRequired.Clear();
	texturesRequired.Clear();
	for (int i = 0; i < cEntities.Size(); ++i)
	{
		/// Models
		List<String> modelDependencies = cEntities[i]->GetModelDependencies();
		for (int j = 0; j < modelDependencies.Size(); ++j){
			if (!modelsRequired.Exists(modelDependencies[j]))
				modelsRequired.Add(modelDependencies[j]);
		}
		/// Textures
		List<String> textureDependencies = cEntities[i]->GetTextureDependencies();
		for (int j = 0; j < textureDependencies.Size(); ++j){
			if (!texturesRequired.Exists(textureDependencies[j]))
				texturesRequired.Add(textureDependencies[j]);
		}
	}
	return true;
}

/// Render?
void Map::Render(GraphicsState & graphicsState){
	std::cout<<"\nWARNING: Map::Render() called! This should only be called in appropriately defined subclasses!";
}
