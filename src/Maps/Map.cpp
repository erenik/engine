
#include "Map.h"

#include "../Entity/EntityManager.h"
extern EntityManager entityManager;

#include "Graphics/GraphicsManager.h"
extern GraphicsManager graphics;

#include "../Entity/CompactEntity.h"
#include "EntityStates/StateProperty.h"
#include "Event/Event.h"
#include "Pathfinding/NavMesh.h"
#include <cstring>

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
bool Map::AddEvent(Event * event){
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

/** Removes target entity from the map. */
bool Map::RemoveEntity(Entity * i_entity){
	entities.Remove(i_entity);
	return true;
}

/** Removes target entities from the map. Returns number of failed removals. */
int Map::RemoveEntities(Selection entitySelection){
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
List<Event*> Map::GetEvents(){
	return events;
}

/// Fetches last error string, resetting it upon use.
String Map::GetLastErrorString()
{
	String s = lastErrorString;
	lastErrorString = String();
	return s;
}

void Map::RemoveAllEvents(){
	events.ClearAndDelete();
}
/* Selection Map::GetEntities(){
	return Selection(entities.GetArray(), entities.Size());
}
*/

// Process called each game loop by the stateManager
void Map::Process(float timePassed){
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
		entity->name;
		if (entity->state)
			entity->state->Process(timePassed);
	}
/*
	// Process events too!
	for (int i = 0; i < events.Size(); ++i){
		if (events[i]->state == Event::BEGUN){
			events[i]->Process(timePassed);
			if (events[i]->state == Event::ENDING)
				events[i]->OnEnd();
		}
	}
*/
}

/// Parse model and texture dependencies if they were not included in the file upon loading!
bool Map::ParseDependencies(){
	modelsRequired.Clear();
	texturesRequired.Clear();
	for (int i = 0; i < cEntities.Size(); ++i){
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
