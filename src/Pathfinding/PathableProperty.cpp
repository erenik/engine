/// Emil Hedemalm
/// 2016-07-29
/// Property for anything using pathfinding system. NPCs, player, walking enemies.

#include "PathableProperty.h"
#include "PathMessage.h"
#include "Waypoint.h"
#include "PathManager.h"
#include "StateManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "WaypointManager.h"

float PathableProperty::defaultProximityThreshold = 1.0f;

PathableProperty::PathableProperty(Entity * owner)
: EntityProperty("PathableProperty", ID(), owner)
{
	isWalking = false;
	nextWaypoint = 0;
	msSinceLastUpdate = 0;
	updateIntervalMs = 200;
	proximityThreshold = defaultProximityThreshold;
}
PathableProperty::~PathableProperty(){}

/// Mainly handles.
void PathableProperty::ProcessMessage(Message * message)
{
	if (message->type == MessageType::SET_PATH_DESTINATION_MESSAGE)
	{
		SetPathDestinationMessage * wm = (SetPathDestinationMessage*) message;
		RequestPath(wm->pos);
		return;
	}
	if (message->type != MessageType::PATHFINDING_MESSAGE)
		return;
	PathMessage * pm = (PathMessage*)message;
	/// Response from pathfinding server?
	assert(pm->requestResponse == PathMessage::RESPONSE);
	StartWalking(pm->path);
}

void PathableProperty::Process(int timeInMs)
{
	if (!isWalking)
		return;
	// Don't spam updates every frame? -> Every second might be good enough?
	msSinceLastUpdate += timeInMs;
	if (msSinceLastUpdate < updateIntervalMs)
		return;
	msSinceLastUpdate = msSinceLastUpdate % updateIntervalMs;
	bool updated = false;
	while (HasReachedNextWaypoint())
	{
		updated = true;
		if (!SetNextWaypointAsTarget())
			return; // If at the end of the road, return.
	}
	if (updated)
	{
		std::cout<<"\nNext waypoint: "<<path.GetIndexOf(nextWaypoint)<<"/"<<path.Size(); if (nextWaypoint) std::cout<<" "<<nextWaypoint->position;
	}
	GoToWaypoint(nextWaypoint);
}

/// Requests a path to the target waypoint. This will then be returned via the PathManaging system as a message.
void PathableProperty::RequestPath(Waypoint * to)
{
	Waypoint * from = WaypointMan.GetClosestWaypoint(owner->worldPosition);
	PathMessage * pm = new PathMessage(owner, from, to);
	PathMan.QueueMessage(pm); /// TODO: Add message queue and handling in path manager. 
	/// TODO: Make pathmanager use threads for querying each new path in the navmesh, since it may take some time for each.
}

/// Requests a path to the target position. This will then be returned via the PathManaging system as a message.
void PathableProperty::RequestPath(ConstVec3fr toPosition)
{
	/// Find WP.
	Waypoint * wp = WaypointMan.GetClosestValidWaypoint(toPosition);
	RequestPath(wp);
}

/// Sets new path to walk, first going to the first waypoint in the path.
void PathableProperty::StartWalking(Path alongPath)
{
	StopWalking();
	isWalking = false;
	nextWaypoint = 0;
	path = alongPath;
	if (!path.Size())
		return;
	isWalking = true;
	nextWaypoint = path[0];
	GoToWaypoint(nextWaypoint);
}

/// To resume walking along path.
void PathableProperty::ResumeWalking()
{
	GoToWaypoint(nextWaypoint);
	isWalking = true;
}
/// Sets isWalking to false. Stops checking distance to next waypoint. Sends by default also a PT_SET_ACCELERATION message with 0 as argument. Subclass to override.
void PathableProperty::StopWalking()
{
	isWalking = false;
}

/// Query if it has reached next waypoint. Just compares distance with the proximity threshold.
bool PathableProperty::HasReachedNextWaypoint()
{
	Vector3f vecDist = nextWaypoint->position - owner->worldPosition;
	float distance = vecDist.Length();
	return distance < proximityThreshold;
}
/// Sets next waypoint as target. If next waypoint becomes 0, it will stop walking. If path is cyclic, will never stop walking until StopWalking is called.
bool PathableProperty::SetNextWaypointAsTarget()
{
	int index = path.GetIndex(nextWaypoint);
	++index;
	if (index >= path.Size() - 1) // If final point, just go to it and stop.
	{
		std::cout<<"\nIs final waypoint.";
		GoToWaypoint(nextWaypoint);
		Message msg("OnFinalDestinationSet");
		owner->ProcessMessage(&msg);
		StopWalking();
		return false;
	}
	nextWaypoint = path[index];	
	// TODO check circularity and max size of path?
	return true;
}

/// This should be subclassed to actually add some physics, based on your given game. By default it will send a PT_SET_ACCELERATION message to the entity using walkingAcceleration.
void PathableProperty::GoToWaypoint(Waypoint * wp)
{
	if (wp == 0)
	{
		StopWalking();
		return;
	}
	MoveToMessage mtm(wp->position);
//	std::cout<<"\nGo to "<<wp->position;
	owner->ProcessMessage(&mtm);
}


