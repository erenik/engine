/// Emil Hedemalm
/// 2014-06-15
/// Map class for grouping entities, lighting, events and scripts together.

#ifndef MAP_H
#define MAP_H

#include "../Globals.h"
#include <Util.h>
#include "Entity/Entities.h"
#include "../Lighting.h"

//#include "..\EntityManager.h"
//#include "..\Entity.h"
class Entity;
class Script;

class CompactEntity;
struct CompactEvent;
class NavMesh;
class Path;

//#include "..\Script.h"

/// Map version flags
#define MV_HEADER
/// Define first and latest too
#define MV_FIRST			0x00000001
#define MV_SIMPLE_ENTITIES	0x00000001	// Contains entities

// + Paths, and some number-changes, like END_OF_FILE moved to a much higher index. 
// - removed blockSizes here too!
#define MV_PATHS			0x00000002	

#define MV_LATEST			MV_PATHS
/// Define current maximum map version
#define MV_CURRENT	MV_LATEST


/** File header to be used when verifying file version and validity.
	DO NOT CHANGE! This will have to remain constant throughout all versions for it to work properly.
*/
struct MapFileHeader{
    bool Write(std::fstream& toFile);
    bool Read(std::fstream& fromFile);
	int version;			/// Last two digits interpreted as minor version, prior ones as major version.
	int dateCreated;		/// Date created, since standard time~
	char mapName[MAX_PATH];	/// Unique (hopefully) mapname ^^
	int NumEntities();		/// Amount of entities in the map. Used for when reloading the maps!
};

/** Defines a map, containing entities, events, etc.
	A map is invalid if it has no name
*/
class Map {
	friend class MapManager;
public:
	Map();
	virtual ~Map();

	/// Declares the structs to be used to contain data related to the maps
	struct OnEnterAttributes;
	struct OnExitAttributes;
	struct Attributes;			// General attributes

	/// Evaluates
	virtual void OnEnter();	// Called once when entering the map
	// Called once when exiting the map
	virtual void OnExit();	
	// Process called each game loop by the stateManager. Time passed in seconds!
	void Process(float timePassed);

	/// Loads map data from file.
	virtual bool Load(const char * fromFile);
	/// Saves map data to file.
	virtual bool Save(const char * toFile);

	/// Resets events so that they can be re-played
	virtual bool ResetEvents();

	/// Returns lighting for this map.
	const Lighting * GetLighting() const {return &lighting; };
	/// Make sure you call this from the render-thread or pause rendering before manipulation!
	Lighting * GetLightingEditable() {return &lighting; };
	/// Render?
	virtual void Render(GraphicsState & graphicsState);

	/// Parse model and texture dependencies if they were not included in the file upon loading!
	bool ParseDependencies();
	/// Loads the map's data into memory, including models and textures, but does not bufferize it.
	bool LoadData();
	/// Buffers map data into graphics memory.
	bool BufferData();

	void SetName(String newName){ name = newName; };
	const String Name(){ return name; };
	const String Creator() { return creator;};
//	static const int MAX_NAME = 260;

	/// Entities is kind of obsolete now what with the utility list class..
///	Entities GetEntities();

	Entity * GetEntity(String byName);
	/** Returns a list of entities in the map. */
	List<Entity*> GetEntities();
	List<Script*> GetEvents();
	inline int NumEntities() { return entities.Size(); };

	/** Removes target entity from the map. If the map is active the entity will also be de-registered from graphics/physics/etc.? */
	virtual bool RemoveEntity(Entity * entity);	

	/// Fetches last error string, resetting it upon use.
	String GetLastErrorString();
protected:

	/// Deletes all entities within the map and re-loads/re-creates them. The entities however are NOT queued to be rendered or participate in physics straight away.
	void LoadFromCompactData();

	/// If this map is currently active and being rendered/interacted with?
	bool active;

	/// For querying reasons of failure.
	String lastErrorString;

	enum mapTypes {
		MAP_TYPE_NULL,
		MAP_TYPE_3D,
		TILE_MAP_2D,
	};
	/// For when making it active.
	int mapType;

	/// Navigational data for this map! Belongs here and not in the WaypoingManager... maybe.
	NavMesh * navMesh;
	/// Paths across the navmesh. Assume that each path has a unique name to identify itself?
	List<Path*> paths;

	/// Map lighting object, accessed via MapManager
	Lighting lighting;

	/// List of external dependencies that were not embedded in the file
	List<String> modelsRequired;
	List<String> texturesRequired;

	/// Flag for when saving/loading map data.
	bool mapDataOK;

	/** Adds target entity to the map. */
	virtual bool AddEntity(Entity * entity);
	/// Adds target event to the map.
	virtual bool AddEvent(Script * eventScript);
	/** Removes target entities from the map. Returns number of failed removals. */
	int RemoveEntities(Entities entities);
	/// Deletes all entities from the map.
	void RemoveAllEntities();

	void RemoveAllEvents();

	/// Reads active entity data block from file
	bool ReadEntities(std::fstream &file);
	/// Writes entity data block to file
	bool WriteEntities(std::fstream &file);

	/// Loads event data, assuming that there exists an event list which at least includes 1 event and a valid source for each event (they will load themselves)
	virtual bool LoadEvents();

	/// Loads embedded path-data, i.e. paths belonging solely to this map.
	bool ReadPaths(std::fstream &file);
	/// Saves embedded path-data, i.e. paths belonging solely to this map.
	bool WritePaths(std::fstream &file);

	/// File header to be used when verifying file version and validity.
	MapFileHeader header;
	/// Function to verify if the header was valid or not.
	bool VerifyHeader();

	/// Name of the map
	String name;
	String source;
	String creator;

	/// Contains onEnter attributes
	OnEnterAttributes * onEnter;
	OnExitAttributes * onExit;

	/// Number of entities and events respectively
	int numEvents;

	/// Maximum amount of entities the map will ever need to hold
	const static int MAX_ENTITIES_PER_MAP = 5000;
	const static int MAX_EVENTS_PER_MAP = 100;

	/// Compact entity list, as saved to file.
	List<CompactEntity*> cEntities;
	/// Compact event list, as saved to file
	CompactEvent * cEvent;

	/// List of currently loaded entities related to this map
	List<Entity*> entities;
//	Entity * entity[MAX_ENTITIES_PER_MAP];
	/// List of currently loaded events related to this map
	List<Script *> events;
};

#endif
