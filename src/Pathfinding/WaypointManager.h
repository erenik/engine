/// Emil Hedemalm
/// 2013-03-01

#ifndef WAYPOINT_MANAGER_H
#define WAYPOINT_MANAGER_H

#include "NavMesh.h"
//#include "../Entity.h"
#include "Mutex/Mutex.h"
class Entity;
class Map;

#define WaypointMan	(*WaypointManager::Instance())

/** Class for handling groups of waypoints, often organized into so-called Navigational Meshes (NavMesh).
	Only one single NavMesh can be active at the time of this writing.
*/
class WaypointManager{
	/// Private constructor for singleton pattern
	WaypointManager();
	static WaypointManager * waypointManager;
public:
	~WaypointManager();
	/// Performs initial calculations (like loading default waypoint-maps for visualization/testing purposes..)
	void Initialize();
	/// Allocates the waypoint manager singleton
	static void Allocate();
	static void Deallocate();
	/// Singleton getter function
	static inline WaypointManager * Instance() { return waypointManager; };

	/// Create a new navmesh!
	NavMesh * CreateNavMesh(String name);
	/// Fetch by name
	NavMesh * GetNavMeshByName(String name);
	/// Returns the loaded navMesh by source
	NavMesh * GetNavMesh(String source);

	/// Makes it active for manipulation
	void MakeActive(NavMesh * nm);
	/// Clears all waypoints from the navmesh!
	void Clear();

	/// For when auto-generating,
	static void SetMinimumWaypointProximity(float minDist);
	static void SetMinimumNeighbours(int neighbours);
	static void SetMinimumInclination(float inclination);

	/// Loads navmesh from file into a free NavMesh slot
	NavMesh * LoadNavMesh(const char * filename = "default.nav");
	/// Saves active navmesh to file
	bool SaveNavMesh(const char * filename = "default.nav");

	/// Returns a reference to it :) Uses pre-defined settings defined in the waypoint manager and looks at all existing entities' faces to generate it!
	NavMesh * GenerateNavMesh(Map * fromMap);
	NavMesh * GenerateNavMesh(List<Entity*> fromEntities);
	/// Attempts to load a spherical world from target entity's model file.
	void GenerateNavMeshFromWorld(Entity * worldEntity);

	/// generates and stores new waypoints for this entity into the active navmesh.
	void GenerateWaypointsFromEntity(Entity * entity);

	/// Nullifies the pData varible of every active waypoint.
	int CleansePData();
	/** Attempts to toggle walkability on the waypoint closest to the provided position both in the current array
		as well as the arrays from which the NavMesh is based upon.
	*/
	bool ToggleWaypointWalkability(Vector3f position);
	/// Returns a waypoint that has no active data bound to it's pData member.
	Waypoint * GetFreeWaypoint();
	/// Returns the closest waypoint to target position that is passable/walkable/valid.
	Waypoint * GetClosestValidWaypoint(Vector3f position);
	/// Gets the closest valid free waypoint (equivalent to GetFreeWaypoint combined with GetClosestValidWaypoint)
	Waypoint * GetClosestValidFreeWaypoint(Vector3f position);
	Waypoint * GetClosestVacantWaypoint(Vector3f position);
	/// TODO: Make a custom request function using binary & on an integer for search-settings. This is getting cumbersome.. lol

	/** Loads waypoint map from target file. */
	bool Load2DWaypointMap(const char * filename, bool optimize = false);
	/** Attempts to get control of the Active NavMesh Mutex
		Make sure you call ReleaseActiveNavMeshMutex afterward!
		The maxWaitTime-parameter defines how long the function will wait before returning automatically.
	*/
	static bool GetActiveNavMeshMutex(int maxWaitTime = 1000);
	/// Releases active NavMeshMutex.
	static bool ReleaseActiveNavMeshMutex();

	/// Returns the active nav mesh
	static NavMesh * ActiveNavMesh();
	
	/// Optimizes the currently selected navMesh
	int Optimize();
	/// Reloads the active NavMesh from it's base.
	int ReloadFromOriginal() { return activeNavMesh->ReloadFromOriginal(); };
private:
	static float minimumWaypointProximity;
	static int minimumNeighbours;
	static float minimumInclination;

	/// Most recently loaded, manipulated or retrieved navMesh.
	NavMesh * activeNavMesh;
	Mutex activeNavMeshMutex;
	
	/// Listification........ YOW!½
	List<NavMesh*> navMeshList;
};

#endif