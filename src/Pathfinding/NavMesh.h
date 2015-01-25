// Author: Emil Hedemalm
// Date: 2013-02-27

#ifndef NAV_MESH_H
#define NAV_MESH_H

#include "Waypoint.h"
#include "../Globals.h"
#include "String/AEString.h"

class Ray;

/// An organization of waypoints that are interconnected somehow, like a map.
class NavMesh {
	/// Let the waypointManager handle all allocation/deallocation!
	friend class WaypointManager;
private:
	NavMesh();
	~NavMesh();
public:

	/// Deletes all waypoints.
	void Clear();

	/// If loaded from a 2D-map, the original layout will be stored here!
	Waypoint *** original2DMap;
	int rows, columns;

	/// If any single successful merge operation has been done.
	bool optimized;

	/// Waypoint list
	List<Waypoint*> waypoints;
	int walkables;
	int waypointArraySize;

	/// Default start and goal for testing the navmesh quickly.
	Waypoint * defaultStart, * defaultGoal;

	/// Scales the positions/elevations of all waypoints.
	void Scale(float amount);
	/// Loads from target navmesh file (specific to this system)
	bool LoadFromFile(const char * filename);
	/// Saves to target navmesh file (specific to this system)
	bool SaveToFile(const char * filename) const;

	/// Maps the defualt start, goal, and other relevant nodes.
	void CalculateDefaults();

	/// Adds a waypoint to the NavMesh.
	int AddWaypoint(Waypoint * wp);
	/// Removes a waypoint from the NavMesh, but does not clean up any remaining bindings.
	int RemoveWaypoint(Waypoint * wp);
	/// Cleans up and removes all neighbour references to this waypoint.
	int CleanupNeighbours(Waypoint * wp);

	/// Returns a pointer to specified Waypoint, or NULL if it does not exist.
	Waypoint * GetWaypointById(int id) const;
	/// Checks if this waypoint exists in this navMesh.
	bool WaypointPartOf(Waypoint * wp) const;
	/// Gets current index of the waypoint in the array.
	int GetIndex(Waypoint * wp);

	/** Re-load all waypoints from the original 2D reference map. */
	int ReloadFromOriginal();
	/** Optimizes the navmesh, using the original 2D-map as reference. 
		Returns amount of merges that were performed.
		Returns -1 if the NavMesh already is optimized.
	*/
	int Optimize();
	/// Returns amount of discarded waypoints. Automatically re-routes/discards neighbours pointers.
	int DiscardUnwalkables();
	/// Attempts to merge all walkable waypoints depending on their distance to each other.
	int MergeWaypointsByProximity(float maxDistance);
	/** Optimization function for merging a select number of waypoints, averaging the position, 
		and re-directing all neighbours' pointers to the new merged waypoint.
	*/
	bool MergeWaypoints(Waypoint ** waypointList, int waypointsToMerge);

	/// Entities..
	Waypoint * GetClosestToRay(Ray & ray);
	/// If maxDistance is positive, it will limit the search within that vicinity-range. A negative number will set no limit.
	Waypoint * GetClosestTo(const Vector3f & position, float maxDistance = -1.f);
	/// Returns vacant waypoint closest to target position
	Waypoint * GetClosestVacantWaypoint(const Vector3f & position);
	
	/// Creates neighbour-connections automatically, using only a maximum distance. Returns number of connections made.
	int ConnectWaypointsByProximity(float maxDistance);
	/// Ensures that each waypoint has atLeast neighbours.
	void EnsureNeighbours(int atLeast);
	
	/// Performs OnSelection functions, like nullifying pData.
	void OnSelect();

	String source;
	String name;
private:

	/// An ID-counter, where IDs are set automatically when waypoints are added to the navMesh
	int idCounter;
};

#endif
