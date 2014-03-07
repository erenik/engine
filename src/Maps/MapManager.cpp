/// Emil Hedemalm
/// 2013-03-01

#include "MapManager.h"
#include "ModelManager.h"
#include "Entity/EntityManager.h"
#include "Entity/Entity.h"
#include "Entity/CompactEntity.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/GraphicsManager.h"
#include "Texture.h"
#include "TextureManager.h"
#include "2D/TileMap2D.h"
#include "OS/Sleep.h"
#include "Event/Event.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/Path.h"
#include "Pathfinding/NavMesh.h"
#include "Entity/EntityFlags.h"
#include "Graphics/Render/Renderable.h"
#include <cstring>

// Singleton initialization.
MapManager * MapManager::mapMan = NULL;
String MapManager::rootMapDir; // = "map/";

/// Definitions/Macros

#define ASSERT_ACTIVE_MAP(returnValue) {\
	assert(activeMap); \
	if (activeMap == NULL){ \
		std::cout<<"\nERROR: No active map."; \
		return returnValue; \
	} \
}


void MapManager::Allocate(){
	assert(mapMan == NULL);
	mapMan = new MapManager();
}
MapManager * MapManager::Instance(){
	assert(mapMan);
	return mapMan;
}
void MapManager::Deallocate(){
	assert(mapMan);
	delete(mapMan);
	mapMan = NULL;
}

MapManager::MapManager(){
	activeMap = NULL;
	defaultAddPhysics = true;
	processOnEnter = true;
}

MapManager::~MapManager(){

}

/// Sets default settings and loads the default map.
void MapManager::Initialize(){
	/// Attempts to load map from specified source file.
	Map * result = LoadMap("default.map");
	if (result){
		result->name = "EditorMap";
		MakeActiveByName("EditorMap");
		result->lighting.VerifyData();
	}
	else {
		std::cout<<"\nError loading default map in MapManager, creating fresh one.";
		result = new Map();
		result->name = "EditorMap";
		maps.Add(result);
		MakeActiveByName("EditorMap");
		result->lighting.VerifyData();
	}
};
/// Resets all data concering the map, deleting any entities and events within it.
void MapManager::ResetMap(Map & map){
	// Reset the compact data.
	memset(&map.header, 0, sizeof(MapFileHeader));
	// Remove all entities
	map.RemoveAllEntities();
	// Flag it's data as OK again
	map.mapDataOK = true;
}

/// Checks if a map with given name already exists.
bool MapManager::Exists(String mapName){
	for (int i = 0; i < maps.Size(); ++i){
		if(maps[i]->Name() == mapName)
			return true;
	}
	return false;
}

// GET MAP
Map * MapManager::GetMap(String byName){
	for (int i = 0; i < maps.Size(); ++i){
		if(maps[i]->Name() == byName)
			return maps[i];
	}
	// Try loading it not existing
	std::cout<<"\nUnable to find map by name, trying to load it.";
	Map * loadMap = LoadMap(byName);
	return loadMap;
}

Map * MapManager::GetMapBySource(String source){
	for (int i = 0; i < maps.Size(); ++i){
		if(maps[i]->source == source)
			return maps[i];
	}
	// Try loading it not existing
	std::cout<<"\nUnable to find map by name, trying to load it.";
	Map * loadMap = LoadMap(source);
	return loadMap;
}


/** Creates a map with specified name. Returns NULL upon failure.
*/
Map * MapManager::CreateMap(String mapName){
	/// First check for duplicates
	if (Exists(mapName))
		return NULL;
	Map * map = new Map();
	map->name = mapName;
	maps.Add(map);
	return map;
}

/** Creates a map with specified name. Returns NULL upon failure.
*/
TileMap2D * MapManager::CreateMap2D(String mapName){
	std::cout<<"\nMapManager::CreateMap2D called: "<<mapName;
	/// First check for duplicates
	if (Exists(mapName))
		return NULL;
	TileMap2D * map = new TileMap2D();
	map->name = mapName;
	maps.Add(map);
	return map;
}


/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
	all depending on the provided entities belonging to it.
	Returns 1 upon success, 0 if an error occurred and -1 if the map isn't loaded yet.
*/
int MapManager::MakeActiveByName(String mapName){
	for (int i = 0; i < maps.Size(); ++i){
		if (!maps[i]->Name())
			continue;
		if (mapName == maps[i]->Name()){
			std::cout<<"\nFound map "<<mapName<<"! Loading now...";
			return MakeActive(maps[i]);
		}
	}
	return -1;
}
/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
	all depending on the provided entities belonging to it.
	Returns false upon failure.
*/
bool MapManager::MakeActiveByIndex(int mapIndex){
	return MakeActive(maps[mapIndex]);
}

bool MapManager::MakeActive(String mapName){
    for (int i = 0; i < maps.Size(); ++i){
        if (maps[i]->name == mapName){
            MakeActive(maps[i]);
            return true;
        }
    }
    return false;
}

/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
	all depending on the provided entities belonging to it.
	Returns false upon failure.
*/
bool MapManager::MakeActive(Map * map){
	/// Target map is already active one!
	if (activeMap == map)
		return true;

	std::cout<<"\nMapManager::MakeActive map: "<<(map ? map->name : "NULL");

	/// Make last map inactive first?
	if (this->activeMap){
		activeMap->lighting.VerifyData();

		activeMap->OnExit();
		/// Unregister all entities
		Graphics.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
		Graphics.QueueMessage(new GMClear(PARTICLE_SYSTEMS));
		Physics.QueueMessage(new PhysicsMessage(PM_CLEAR_ALL_ENTITIES));
		activeMap = NULL;
	}

	/// Set this as the active map! :3
	this->activeMap = map;
	// Now, if a null-map was queued, make it so!
	if (map == NULL)
		return false;

	Graphics.QueueMessage(new GMSetLighting(map->lighting));
	/// Initial check again.
	if (!map->Name()){
		std::cout<<"\nWARNING: Null map set!";
		assert(false && "WARNING: NULL map set (no-name)");
		return false;
	}

	/** Creates entities from the cEntity data, as these structures are the only ones guaranteed to remain upon
		switching active map!
	*/
	LoadFromCompactData(map);

	/// Register the entities with relevant managers!
	Entity * currentEntity;
	for (int i = 0; i < map->entities.Size(); ++i){
		currentEntity = map->entities[i];
		/// Register for rendering
		Graphics.QueueMessage(new GMRegisterEntity(currentEntity));
		/// Register for Physics.
		if (currentEntity->physics && !currentEntity->registeredForPhysics)
			Physics.QueueMessage(new PMRegisterEntity(currentEntity));
	}
//	std::cout<<"\nMapManager::MakeActive map loaded, calling map's OnEnter: "<<map->name;
	if (processOnEnter)
		activeMap->OnEnter();
	activeMap->lighting.VerifyData();
	return true;
}

/** "Selects" all entities whose name fulfills the requirements of the provided name by discarding the remaining entities.
	Arguments can be any of the following formats: name, *name, name* or *name*
	Where the asterix * acts as a wildcard character which can be a string of any length.
*/
List<Entity*> MapManager::SelectEntitiesByName(const char * i_name){
	List<Entity*> result;
	if (i_name == NULL)
		return result;
	List<Entity*> mapEntities = activeMap->GetEntities();
	std::cout<<"\nSelecting entities by name: "<<i_name;

	// Name to compare
	char name[256];
	strcpy(name, i_name);

	bool wildCardBeginning = false;
	bool wildCardEnd = false;
	if (name[0] == '*'){
		wildCardBeginning = true;
		/// If just an asterix.. lol
		if (strlen(i_name) == 1)
			return mapEntities;
	}
	if (name[strlen(name)-1] == '*'){
		wildCardEnd = true;
	}
	// Fix name by removing asterixes now!
	if (wildCardBeginning)
		strcpy(name, name+1);
	if (wildCardEnd)
		name[strlen(name)-1] = '\0';

	// Compare name with all entities in the active map
	for (int i = 0; i < activeMap->entities.Size(); ++i){
		Entity * entity = activeMap->entities[i];
		// Exact match
		if (!wildCardBeginning && !wildCardEnd &&
			(strcmp(name, entity->name) == 0)){
			result.Add(entity);
		}
		// Wildcard start
		else if (wildCardBeginning && !wildCardEnd){
			for (int i = 0; i <= (int)(strlen(entity->name) - strlen(name)); ++i){
				int stringComparisonResult = strcmp(name, entity->name + i);
				if (stringComparisonResult == 0){
					result.Add(entity);
					break;
				}
			}
		}
		// Wildcard end
		else if (!wildCardBeginning && wildCardEnd){
			int stringComparisonResult = strcmp(name, entity->name);
			if (stringComparisonResult == 0){
				result.Add(entity);
			}
		}
		// Double Wildcarduuuuu!
		else if (wildCardBeginning && wildCardEnd) {
			for (int i = 0; i <= (int)(strlen(entity->name) - strlen(name)); ++i){
				int stringComparisonResult = strncmp(name, entity->name + i, strlen(name));
				if (stringComparisonResult == 0){
					result.Add(entity);
					break;
				}
			}
		}
	}
	return result;
}


/// Prints list of enities to console
void MapManager::ListEntities(){
	std::cout<<"\nListing entities for map: "<<(activeMap->name? activeMap->name : "No-name");
	for (int i = 0; i < activeMap->entities.Size(); ++i){
		std::cout<<"\n"<<i<<". "
			<<activeMap->entities[i]->name
			<<" Pos: "<<activeMap->entities[i]->positionVector;
	}
}
/** Fills the provided selection with all available entities in the active map. */
int MapManager::GetEntities(List<Entity*> & targetEntities){
	if (!activeMap){
		std::cout<<"No valid map selected!";
		return 0;
	}
	targetEntities = activeMap->entities;
	return activeMap->NumEntities();
}

/** Returns a selection object containing all entities in the current map. */
List<Entity*> MapManager::GetEntities(){
	if (!activeMap){
		std::cout<<"No valid map selected!";
		return List<Entity*>();
	}
	return activeMap->entities;
}

/// Returns the first/best entity found via the provided selection ray.
Entity * MapManager::GetFirstEntity(Ray & selectionRay, Vector3f & intersectionPoint){
#define ray selectionRay
    List<Entity*> entities = GetEntities();
    Entity * closest = NULL;
    float closestRadius = 1000000000000.0f;
    float closestDistance = 10000000000000.0f;
    float closestTriangleCollissionDistance = 1000000000000.0f;

    List<Entity*> entitiesBehind, entitiesPwned;
    for (int i = 0; i < entities.Size(); ++i){
        Entity * entity = entities[i];

        /// First discard those entities which are behind the ray's origin.
        float entityRadius = entity->scaleVector.MaxPart() * entity->radius;
        float distanceEntityToRayStart = (entity->positionVector - selectionRay.start).Length() - entityRadius;

        /// Check if it's anywhere near as close as the closest-distance, if not discard it now.
        if (closest && distanceEntityToRayStart - entityRadius > closestDistance + closestRadius){
            entitiesPwned.Add(entity);
            continue;
        }

        Vector3f rayStartToEntity = entity->positionVector - selectionRay.start;
        if (rayStartToEntity.DotProduct(selectionRay.direction) < 0 && distanceEntityToRayStart > entity->radius * entity->scaleVector.MaxPart()){
            entitiesBehind.Add(entity);
            continue;
        }

        /// Then do a radial check to dismiss unrelated ones.
        float distanceProjectedOntoClickRay = selectionRay.direction.DotProduct(rayStartToEntity);
        Vector3f projectedPointOnVector = selectionRay.start + distanceProjectedOntoClickRay * selectionRay.direction;
        float distanceEntityToRay = (entity->positionVector - projectedPointOnVector).Length();
        /// Skip entities that aren't even close to the ray. (sphere not touching the ray).
        if (distanceEntityToRay > entity->radius * entity->scaleVector.MaxPart()){
            continue;
        }

        /// Check collission with any of the entity's faces.
        bool collissionFound = false;
        List<Triangle> triangles = entity->model->GetTris();
        Sphere sphere;
        sphere.radius = 0.1f;
        for (int i = 0; i < triangles.Size(); ++i){

            Triangle triangle = triangles[i];
#define tri triangle
            tri.Transform(entity->transformationMatrix);
            RenderOptions ro;
            ro.duration = 5.0f;
            ro.disableDepthTest = true;
            ro.color = Vector4f(1.0f,0,0,1);
        //    Graphics.QueueMessage(new GMRender(tri, &ro));

            ///  hypo = sqrt(k² + k²)
            /// distance to plane² = dotProduct²
            float distance;
            bool intersects = ray.Intersect(tri, &distance);
       //     std::cout<<"\nIntersects: "<<intersects;
            if (!intersects)
                continue;
      //      std::cout<<"\nDistance: "<<distance;

            /// Early out if the distance is greater than any previous one.
            if (distance > closestTriangleCollissionDistance)
                break;

            intersectionPoint = ray.start + ray.direction * distance;
            std::cout<<"\nIntersectionPoint: "<<intersectionPoint;
            collissionFound = true;
            closestTriangleCollissionDistance = distance;
        }
        if (!collissionFound)
            continue;

        closest = entity;
        closestDistance = distanceEntityToRayStart;
        closestRadius = entityRadius;
    }
/*
    std::cout<<"\n";
    std::cout<<"\n"<<entities.Size()<<" entities examined.. of which:";
    std::cout<<"\n  "<<entitiesBehind.Size()<<" entities behind ray origin/direction.";
    std::cout<<"\n  "<<entitiesPwned.Size()<<" entities pwned by sheer distance (since we already got a valid target).";
    std::cout<<"\n";
*/
    return closest;
}


/** Creates a duplicate entity, copying all relevant information (as possible). */
Entity * MapManager::CreateEntity(Entity * referenceEntity){
	assert(referenceEntity);
	Entity * entity = EntityMan.CreateEntity(referenceEntity->model, referenceEntity->GetTexture(DIFFUSE_MAP));
	entity->physics = new PhysicsProperty(*referenceEntity->physics);
	entity->scaleVector = referenceEntity->scaleVector;
	entity->rotationVector = referenceEntity->rotationVector;
	entity->positionVector = referenceEntity->positionVector;
	entity->recalculateMatrix();
	// Register it with the graphics manager straight away since it's the active map!
	Graphics.QueueMessage(new GMRegisterEntity(entity));
	// Go ahead and add physics too, most entities will have physics, so.
	if (defaultAddPhysics)
		Physics.QueueMessage(new PMRegisterEntity(entity));
	activeMap->AddEntity(entity);
	return entity;
}

/** Creates an entity with target model and texture and places it into the active map. */
Entity * MapManager::CreateEntity(Model * model, Texture * texture, Vector3f position){

	if (!model){
		std::cout<<"\nWARNING: Model invalid. Assigning default type.";
		model = ModelMan.GetModel(0);
	}
	else if (!texture){
		std::cout<<"\nWarning: Entity lacking texture, is this how it's sposed to be?";
	}
	std::cout<<"\nCreating entity with model "<<(model? model->Name() : String("None"))<<" and texture: "<<(texture? texture->name : String("None"));

	Entity * entity = EntityMan.CreateEntity(model, texture);
	if (entity == NULL){
	    std::cout<<"\nERROR: MapManager::CreateEntity:Unable to create entity, returning.";
        return NULL;
	}



	entity->positionVector = position;
	entity->recalculateMatrix();

	assert(activeMap && "Active Map not set in MapManager::CreateEntity!");
	bool addResult = activeMap->AddEntity(entity);
	if (addResult == false){
		EntityMan.DeleteEntity(entity);
		//	Abort creation.
		return NULL;
	}

	// Register it with the graphics manager straight away since it's the active map!
	Graphics.QueueMessage(new GMRegisterEntity(entity));
	// Go ahead and add physics too, most entities will have physics, so.
	if (defaultAddPhysics)
		Physics.QueueMessage(new PMRegisterEntity(entity));

	return entity;
}

#include "Event/Event.h"

/** Adds an event ~ */
bool MapManager::AddEvent(Event * event){
	assert(activeMap);
	return activeMap->AddEvent(event);
}

/** Creates an event, placing it in the list of the maplir. */
Event * MapManager::CreateEvent(){
	assert(false);
	return NULL;
}

/// Attempts to remove given event from ze map.
bool MapManager::DeleteEvent(Event * event){
	assert(activeMap);
	bool result = activeMap->events.Remove(event);
	if (result)
		delete event;
	return result;
}
/// Deletes all events.
void MapManager::DeleteEvents(){
	assert(activeMap);
	for (int i = 0; i < activeMap->events.Size(); ++i){
		DeleteEvent(activeMap->events[i]);
	}
}

/// Oy. Gets active map's events.
List<Event*> MapManager::GetEvents(){
	assert(activeMap);
	return activeMap->events;
}

/** Queries deletion of all entities in the active map. */
int MapManager::DeleteEntities(){
	assert(activeMap);
	int deleted = 0;
	List<Entity*> mapEntities = activeMap->GetEntities();
	activeMap->RemoveEntities(mapEntities);
	Graphics.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
	Physics.QueueMessage(new PhysicsMessage(PM_UNREGISTER_ALL_ENTITIES));
	for (int i = 0; i < mapEntities.Size(); ++i){
		mapEntities[i]->flaggedForDeletion = true;
	}
	return deleted;
}

/** Queries deletion of specified entity in active map. */
bool MapManager::DeleteEntity(Entity * entity){
	// Check that it isn't already flagged for deletion
	if (!entity && !entity->flaggedForDeletion){
		return false;
	}
	entity->flaggedForDeletion = true;
	std::cout<<"\nEntity flagged for deletion. ";
	if (entity->registeredForRendering)
		Graphics.QueueMessage(new GMUnregisterEntity(entity));
	if (entity->registeredForPhysics)
		Physics.QueueMessage(new PMUnregisterEntity(entity));
	// Remove entity from the map too...!
	activeMap->RemoveEntity(entity);
	// Delete any other extra bits and pieces needed.
	entity->Delete();
	return true;
}

/** Notifies the map that the entity has been registered from one or more services.
	This function will then check relevant variables and if fully unregistered will mark it as
	unused and queue it's deletion in the EntityManager.
*/
void MapManager::EntityUnregistered(Entity * entity){
	if(entity->flaggedForDeletion){
		if (entity->registeredForPhysics)
			return;
		else if (entity->registeredForRendering)
			return;
		/// Remove it from any maps it belonged to.
		for (int i = 0; i < maps.Size(); ++i)
			maps[i]->RemoveEntity(entity);
		/// Tell the entity manager to delete the entity.
		EntityMan.DeleteEntity(entity);
	}
}

/// Get current map lighting
Lighting MapManager::GetLighting(){
	Lighting currentLighting;
//	currentLighting.VerifyData();
	if (activeMap){
		currentLighting = activeMap->lighting;
	//	currentLighting.VerifyData();
	}
	return currentLighting;
}
/// Get current map lighting
void MapManager::SetLighting(Lighting lighting){
	if (activeMap)
		activeMap->lighting = lighting;
}

/// Lists the loaded maps to console
void MapManager::ListMaps(){

}

/// Attempts to save active map to file.
bool MapManager::SaveMap(String toFile){
	bool result = false;
	if (toFile.Length() == 0){
		std::cout<<"\nFilename null";
		return false;
	}
	if (!this->activeMap){
		std::cout<<"\nNo active map selected.";
		return false;
	}

	String filename = "";
//	if (!(toFile.Contains("map\\") || toFile.Contains("map/")))
//		filename = "map/";
	filename += toFile;
	if (!(filename.Contains(".map") || filename.Contains(".tmap"))){
		switch(activeMap->mapType){
			case Map::MAP_TYPE_3D:
				filename += ".map";
				break;
			case Map::TILE_MAP_2D:
				filename += ".tmap";
				break;
			default:
				assert(false);
				break;
		}
	}

	/// Call save function
	result = this->activeMap->Save(filename.c_str());
	if (!result)
		lastErrorString = this->activeMap->GetLastErrorString();
	// Return result
	return result;
}

/** Attempts to load map from specified source file to specified map slot.
	If map is NULL, a free slot will be selected. Returns a pointer to the loaded map or NULL if it failed.
*/
Map * MapManager::LoadMap(String fromFile, Map * targetMap){

	/// Fix path if lazy path.
/*	if (!fromFile.Contains(rootMapDir)){
		fromFile = rootMapDir + fromFile;
	}
*/
	if (!(fromFile.Contains(".map") || fromFile.Contains(".tmap"))){
		fromFile += ".map";
	}
	/// Check that it isn't already loaded!
	for (int i = 0; i < maps.Size(); ++i){
		Map * map = maps[i];
		String mapSource = map->source;
		assert(mapSource.Length() > 0);
		if (mapSource == fromFile || mapSource.Contains(fromFile)){
			return maps[i];
		}
	}
	bool result = false;
	if (fromFile.Length() == 0){
		std::cout<<"\nFilename null";
		return NULL;
	}

	Map * mapLoaded = NULL;
	// Use target map if it was provided
	if (targetMap)
		mapLoaded = targetMap;

	/// Allocate eeet
	if (fromFile.Contains(".tmap")){
		std::cout<<"\nCreating new TileMap2D..";
		mapLoaded = new TileMap2D();
	}
	else {
		std::cout<<"\nCreating new Map..";
		mapLoaded = new Map();
	}

	/// Call load function
	result = mapLoaded->Load(fromFile);
	mapLoaded->source = fromFile;
	if (!result){
		lastErrorString = mapLoaded->GetLastErrorString();
		delete mapLoaded;
		return NULL;
	}

	/// Add the fucking map to the list too. ..... >_>
	maps.Add(mapLoaded);

	/// =================================================================================================
	/// TODO: The below should be made into a separate function for preparing the map for making it active!
	/// =================================================================================================
	/// Parse model and texture dependencies
	mapLoaded->ParseDependencies();

	/// Load the parsed external dependencies.
	TexMan.LoadTextures(mapLoaded->texturesRequired);
	ModelMan.LoadModels(mapLoaded->modelsRequired);

	// Set map name!
	mapLoaded->name = fromFile;
	mapLoaded->source = fromFile;
	if (result)
		return mapLoaded;
	return NULL;
}

/// Attempts to load all map files from target directory.
void MapManager::LoadMapsFromDirectory(const char * directory){

}

/** Reloads active map from file, returning it upon success and NULL upon failure. */
Map * MapManager::ReloadFromFile(){
	Selection mapEntities = GetEntities();
	for (int i = 0; i < mapEntities.Size(); ++i){
		activeMap->RemoveEntity(mapEntities[i]);
	}
	// Load the map again from it's compact data. ^^
	LoadFromCompactData(activeMap);
	MakeActive(activeMap);
	return activeMap;
}

/// Navmeseeeesh. Returns false if fail (like if 0 entities or what?)
bool MapManager::CreateNavMesh(List<Entity*> entitiesToCreateFrom){
	ASSERT_ACTIVE_MAP(false);
	Graphics.PauseRendering();
	/// If we got any paths, clear them first.
	if (activeMap->paths)
		CLEAR_AND_DELETE(activeMap->paths);
	if (activeMap->navMesh == NULL)
		activeMap->navMesh = WaypointMan.CreateNavMesh(activeMap->name + " Navmesh");
	/// If it already had stuff, clear it.
	else {
		WaypointMan.MakeActive(activeMap->navMesh);
		WaypointMan.Clear();
	}
	NavMesh * nm = WaypointMan.GenerateNavMesh(entitiesToCreateFrom);
	activeMap->navMesh = nm;
	Graphics.ResumeRendering();
	return true;
}
/// Assign a navmesh to the current map, invalidating old paths that might have been linked to the old navmesh!
bool MapManager::AssignNavMesh(NavMesh * nm){
	ASSERT_ACTIVE_MAP(false);
	activeMap->paths.ClearAndDelete();
	activeMap->navMesh = nm;
}

const NavMesh * MapManager::GetNavMesh(){
	ASSERT_ACTIVE_MAP(NULL);
	return activeMap->navMesh;
}
/// Wosh.
void MapManager::AddPath(Path * path){
	assert(activeMap);
	/// Make sure we don't add anything twice, yo?
	for (int i = 0; i < activeMap->paths.Size(); ++i){
		if (activeMap->paths[i]->name == path->name){
			std::cout<<"\nWARNING: Path with given name "<<path->name<<" already exists. Overriting it's data!";
			Path * p = activeMap->paths[i];
			activeMap->paths.Remove(p);
			delete p;
			--i;
		}
	}

	activeMap->paths.Add(path);
}
Path * MapManager::GetPath(String byName){
	assert(activeMap);
	for (int i = 0; i < activeMap->paths.Size(); ++i){
		if (activeMap->paths[i]->name == byName)
			return activeMap->paths[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////////////

/// Loads data from compact versions, registering them in the various managers.
void MapManager::LoadFromCompactData(Map * map){
	std::cout<<"\nMapManager::LoadFromCompactData for map: "<<map->name;
	if (map->NumEntities() > 0){
		std::cout<<"\nEntities already exist in map. Skipping loading procedure from compact data.";
		return;
		std::cout<<"ERROR: Map has "<<map->NumEntities()<<" remaining entities. Delete these before reloading from compact map data!";
		assert(map->NumEntities() == 0);
		map->RemoveAllEntities();
	//	return;
	}
	/// TODO: Make function of creating stuff from compact format
	int entitiesToCreate = map->cEntities.Size();
	if (entitiesToCreate == 0){
		std::cout<<"\nNo entities in map compact data, skipping LoadFromCompactData";
		return;
	}
	assert(entitiesToCreate > 0);
	std::cout<<"\nCreating entities from compact file format...";
	/// Convert cEntities to regular entities, etc.
	for (int i = 0; i < entitiesToCreate; ++i){
		/// Get model and texture...
		Model * model = ModelMan.GetModel(map->cEntities[i]->model);
		Texture * texture = TexMan.GetTextureBySource(map->cEntities[i]->diffuseMap);
		if (!model){
			std::cout<<"\nWARNING: Unable to locate model, skipping entity.";
			continue;
		}
		/// Ask entity manager to create them :P
		Entity * newEntity = EntityMan.CreateEntity(model, texture);
		newEntity->LoadCompactEntityData(map->cEntities[i]);
		/// Add them to the map ^^
		map->AddEntity(newEntity);
	}

}


/// Clears all entities spawned by events.
void MapManager::ClearEventSpawnedEntities(){
	assert(activeMap);
	List<Entity*> entities = activeMap->GetEntities();
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
		if (entity->flags & SPAWNED_BY_EVENT){
			DeleteEntity(entity);
		}
	}
}

/// Removes all entities with the PLAYER_OWNED_FLAG
void MapManager::ClearPlayerEntities(){
	assert(activeMap);
	List<Entity*> entities = activeMap->GetEntities();
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];
		if (entity->flags & PLAYER_OWNED_ENTITY){
			DeleteEntity(entity);
		}
	}
}

/// Fetches last error string, resetting it upon use.
String MapManager::GetLastErrorString()
{
	String s = lastErrorString;
	lastErrorString = String();
	return s;
}
