/// Emil Hedemalm
/// 2013-12-23
/// All messages relating to waypoints belong here!

#include "PhysicsMessage.h"
#include "Pathfinding/Waypoint.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathfindingProperty.h"

#include "Entity/Entity.h"

PMSetWaypoint::PMSetWaypoint(const Vector3f & position, int target, void * value)
: PhysicsMessage(PM_SET_WAYPOINT), position(position), target(target), pValue(value)
{
	switch(target){
		case PT_ENTITY:
			break;
		default:
			assert(false && "Bad target");
	}
}

void PMSetWaypoint::Process()
{
	switch(target){
		case PT_ENTITY:
		{
			NavMesh * nm = WaypointMan.ActiveNavMesh();
			if (!nm){
				assert(false);
				return;
			}
			Waypoint * wp = nm->GetClosestTo(position);
			assert(wp);
			Entity * entity = (Entity*)pValue;
			wp->entities.Add(entity);
			entity->pathfindingProperty->currentWaypoint = wp;
			break;
		}
	}
}

/*private:
	Vector3f position;
	int target;
	void * pValue;
};
*/