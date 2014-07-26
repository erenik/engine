#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H

#include "Maps/Map.h"
#include "Util.h"
//#include "EntityManager.h"
#include <iostream>

#define MapMan		(*MapManager::Instance())

/// Remove windows defined CreateEvent..
#undef CreateEvent

class Model;
class Texture;
class TileMap2D;
class Script;
struct Ray;

/** Defines a class that keeps track of and separates different maps,
	where maps are basically just a grouping of entities, events, etc.
*/
class MapManager{
private:
	MapManager();
	static MapManager * mapMan;
public:
	static void Allocate();
	static MapManager * Instance();
	static void Deallocate();
	~MapManager();
	/// Sets default settings and loads the default map.
	void Initialize();
	/// Resets all data concering the map, deleting any entities and events within it.
	void ResetMap(Map & map);


	/** "Selects" all entities whose name fulfills the requirements of the provided name by discarding the remaining entities.
		Arguments can be any of the following formats: name, *name, name* or *name*
		Where the asterix * acts as a wildcard character which can be a string of any length.
	*/
	List<Entity*> SelectEntitiesByName(const char * name);
	// GET MAP
	Map * GetMap(String byName);
	Map * GetMapBySource(String source);
	/// Prints list of enities in active map to console
	void ListEntities();

	Entity * GetEntityByName(String name);
	/** Fills the provided selection with all available entities in the active map. */
	int GetEntities(List<Entity*> & entityList);
	/** Returns a selection object containing all entities in the current map. */
	List<Entity*> GetEntities();
    /// Returns the first/best entity found via the provided selection ray.
	Entity * GetFirstEntity(Ray & selectionRay, Vector3f & intersectionPoint);

	/** Creates a duplicate entity, copying all relevant information (as possible). */
	Entity * CreateEntity(Entity * entity);
	/** Creates an entity with target model and texture and places it into the active map. */
	Entity * CreateEntity(String name, Model * model, Texture * texture, Vector3f position = Vector3f());
	/// Adds target entity to the map, registering it for physics and graphicsState->
	bool AddEntity(Entity * entity);

	/** Adds an event ~ */
	bool AddEvent(Script * eventScript);
	/** Creates an event, placing it in the list of the maplir. */
	Script * CreateEvent();
	/** Attempts to remove given event from ze map. Also calls it's destructorrrr if it exists within the given map.
		In the case that the event does not exist in the active map, the function will return false and NO destructor will be called!
	*/
	bool DeleteEvent(Script * eventScript);
	/// Deletes all events.
	void DeleteEvents();
	/// Oy. Gets active map's events.
	List<Script*> GetEvents();

	// Deletes all entities in the active map.
	int DeleteAllEntities();
	// Delete specific entities.
	int DeleteEntities(List<Entity*> entities);
	/** Queries deletion of specified entity in active map. */
	bool DeleteEntity(Entity * entity);
	/** Notifies the map that the entity has been registered from one or more services.
		This function will then check relevant variables and if fully unregistered will mark it as
		unused and queue it's deletion in the EntityManager. */
	void EntityUnregistered(Entity * entity);

	/// Get current map lighting
	Lighting GetLighting();
	/// Get current map lighting
	void SetLighting(Lighting lighting);

	/// Checks if a map with given name already exists.
	bool Exists(String mapName);

	/** Creates a map with specified name. Returns NULL upon failure.
	*/
	Map * CreateMap(String mapName);
	/** Creates a map with specified name. Returns NULL upon failure.
	*/
	TileMap2D * CreateMap2D(String mapName);

	/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
		all depending on the provided entities belonging to it.
		Returns 1 upon success, 0 if an error occurred and -1 if the map isn't loaded yet.
	*/
	int MakeActiveByName(String mapName);
	/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
		all depending on the provided entities belonging to it.
		Returns false upon failure.
	*/
	bool MakeActiveByIndex(int mapIndex);

	/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
		all depending on the provided entities belonging to it.
		Returns false upon failure.
	*/
	bool MakeActive(String mapName);
	/** Makes target map active, queueing it's bufferization into the graphicsManager, physicsManager, etc.
		all depending on the provided entities belonging to it.
		Returns false upon failure.
	*/
	bool MakeActive(Map * map);

	/// Lists the loaded maps to console
	void ListMaps();
	/// Attempts to save active map to file.
	bool SaveMap(String toFile);
	/** Attempts to load map from specified source file to specified map slot.
		If map is NULL, a free slot will be selected.
		Returns a pointer to the new map, or NULL if it failed.
	*/
	Map * LoadMap(String fromSourceFile, Map * map = NULL);
	/// Attempts to load all map files from target directory.
	void LoadMapsFromDirectory(const char * directory);

	/** Reloads active map from file, returning it upon success and NULL upon failure. */
	Map * ReloadFromFile();

	Map * ActiveMap() { return activeMap; };

	/// Bleh.
	void SetDefaultAddPhysics(bool v){ defaultAddPhysics = v; };
	bool DefaultAddPhysics() const { return defaultAddPhysics; };

	/// Navmeseeeesh. Returns false if fail (like if 0 entities or what?)
	bool CreateNavMesh(List<Entity*> entitiesToCreateFrom);
	/// Assign a navmesh to the current map, invalidating old paths that might have been linked to the old navmesh!
	bool AssignNavMesh(NavMesh * nm);
	const NavMesh * GetNavMesh();
	/// Paths will be taken care of by the map for destruction, but you will have to create it yourself!
	void AddPath(Path * path);
	Path * GetPath(String byName);

	/// Clears all entities spawned by events (having the SPAWNED_BY_EVENT flag).
	void ClearEventSpawnedEntities();
	/// Removes all entities with the PLAYER_OWNED_FLAG
	void ClearPlayerEntities();

	/// Enable/Disables Map::OnEnter processing.
	bool processOnEnter;

	/// Fetches last error string, resetting it upon use.
	String GetLastErrorString();
	/// Root dir for fetching and saving maps
	static String rootMapDir;
private:
	String lastErrorString;

	/// Bleh.
	bool defaultAddPhysics;

	/// Loads data from compact versions, registering them in the various managers.
	void LoadFromCompactData(Map * map);

	/// Currently active map
	Map * activeMap;
	/// List with the maps :3
	List<Map*> maps;

};

#endif
