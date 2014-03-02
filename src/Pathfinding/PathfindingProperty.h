/// Emil Hedemalm
/// 2013-12-21
/// Entity property to handle pathfinding!

#ifndef PATHFINDING_PROPERTY_H
#define PATHFINDING_PROPERTY_H

#include "MathLib.h"
#include "Pathfinding/Path.h"

class Entity;
class Path;
class Waypoint;

/// Entity property to handle pathfinding!
struct PathfindingProperty {
	friend class PhysicsManager;
public:
	PathfindingProperty(Entity * owner);
	void OnPathCompleted();
	// Woo
	void SetQueuedDestination(Waypoint * wp);
	void QueuePath(Path path);
	bool HasPathsToTread();
	void BeginPathIfNotAlreadyOneOne();
	/// Stuff!
	Vector3f desiredVelocity;
	/// When standing still, currentWaypoint should be non-NULL and targetWaypoint should be NULL.
	/// When moving, both currentWaypoint and targetWaypoint should be non-NULL, and the entity moving from currentWaypoint and to targetWaypoint.
	Waypoint * currentWaypoint;
	Waypoint * targetWaypoint;
	
	Path CurrentPath(){ return currentPath; };
	// wuff
	Entity * owner;
private:
	Path currentPath;
	Path queuedPath;
	Waypoint * queuedDestination;
	
};

#endif