/// Emil Hedemalm
/// 2013-12-21
/// Entity property to handle pathfinding!

#include "PathfindingProperty.h"
#include "PathManager.h"
#include "Waypoint.h"
#include "Path.h"
#include "Entity/Entity.h"

PathfindingProperty::PathfindingProperty(Entity* entity)
 : owner(entity)
{
	currentWaypoint = NULL;
	targetWaypoint = NULL;
	queuedDestination = NULL;
}

PathfindingProperty::~PathfindingProperty()
{
	/// Remove pointers to this entity if possible.
	if (currentWaypoint)
		currentWaypoint->entities.Remove(this->owner);
}	

void PathfindingProperty::OnPathCompleted(){
	currentPath = queuedPath;
	queuedPath = Path();
}

// Woo
void PathfindingProperty::QueuePath(Path path){
	queuedPath = path;
}

bool PathfindingProperty::HasPathsToTread()
{
	if (currentPath.Size())
		return true;
	if (queuedPath.Size())
		return true;
	if (queuedDestination)
		return true;
	return false;
}

void PathfindingProperty::SetQueuedDestination(Waypoint * wp){
	queuedDestination = wp;
}

void PathfindingProperty::BeginPathIfNotAlreadyOneOne()
{
	if (currentPath.Size())
		return;
	else if (queuedPath.Size()){
		currentPath = queuedPath;
		queuedPath = Path();
	}
	else if (queuedDestination){
		Path path = PathMan.GetPath(currentWaypoint, queuedDestination);
		if (path.Size()){
			currentPath = path;
		}
		queuedDestination = NULL;
	}			
}