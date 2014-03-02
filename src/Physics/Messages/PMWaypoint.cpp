/// Emil Hedemalm
/// 2013-12-23
/// All messages relating to waypoints belong here!

#include "PhysicsMessage.h"
#include "Pathfinding/Waypoint.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathfindingProperty.h"

PMSetWaypoint::PMSetWaypoint(Vector3f position, int target, void * value)
: PhysicsMessage(PM_SET_WAYPOINT), position(position), target(target), pValue(value){
	switch(target){
		case ENTITY:
			break;
		default:
			assert(false && "Bad target");
	}
}
void PMSetWaypoint::Process(){
	switch(target){
		case ENTITY:{
			NavMesh * nm = WaypointMan.ActiveNavMesh();
			if (!nm){
				assert(false);
				return;
			}
			Waypoint * wp = nm->GetClosestTo(position);
			assert(wp);
			wp->entity = (Entity*)pValue;
			wp->entity->pathfindingProperty->currentWaypoint = wp;
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