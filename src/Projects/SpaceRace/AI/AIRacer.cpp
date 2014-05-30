// Emil Hedemalm
// 2013-07-28

#include <cstring>
#include <cmath>

#include "AIRacer.h"
#include "../EntityStates/RacingShipGlobal.h"
#include "../GameStates/Racing/RacingState.h"
#include "../SRConstants.h"
#include "Pathfinding/NavMesh.h"
#include "Pathfinding/Path.h"
#include "Maps/MapManager.h"
#include "../SRPlayer.h"

#ifndef max
#define max(a,b) (a > b? a : b)
#endif

AIRacer::AIRacer(Entity * shipEntity, SRPlayer * player, RacingShipGlobal * shipState, Racing * gameState)
: AI(AI_TYPE_STATE_MACHINE), shipEntity(shipEntity), player(player), shipState(shipState), gameState(gameState)
{
	assert(shipEntity);
	assert(player);
	assert(shipState);
	assert(gameState);
	Reset();
	autoTurn = autoThrust = true;
	enabled = true;
}
AIRacer::~AIRacer(){
	// Wosh.
}

/// Reset so it gathers variables again.
void AIRacer::Reset(){
	closestWaypoint = NULL;
	nextWaypoints.Clear();
	nextCheckpointIndexuu = 0;
	path = NULL;
	/// To the next checkpoint. If this is exceeded by x2 at any time it means we missed the checkpoint, so reset.
	closestDistance = 1000000000000000000000.f;
}

void AIRacer::Process(float timeInSeconds){

	/// Go towards next waypoint!
	while(closestWaypoint == NULL){
		// Get it.
		const NavMesh * nm = MapMan.GetNavMesh();
/*		if (nm != NULL){
			assert(false && "Implement");
			break;
		}
		*/
		path = MapMan.GetPath(MAIN_TRACK_PATH_NAME);
		if (path == NULL){
			; //std::cout<<"\nERROR: No navmesh nor path available. AI will not function.";
			return;
		}
		closestWaypoint = path->GetClosest(shipEntity->position);
		if (closestWaypoint == NULL){
			std::cout<<"\nERROR: Unable to find closest waypoint. This ship is probably fucked.";
			return;
		}
		assert(closestWaypoint);
		/// Calculate new next waypoint-average.
		CalculateNextAverage();
	}

	// Get new closest waypoint if applicable.
	if (path == NULL){
		std::cout<<"\nNo valid pathhhh? >:";
		return;
	}
	float distToCurr = (closestWaypoint->position - shipEntity->position).LengthSquared();
	float distToNext = (nextWaypointAverage - shipEntity->position).LengthSquared();
	if (distToNext < distToCurr){
		closestWaypoint = nextWaypoints[0];
		CalculateNextAverage();
	}

	if (max(distToCurr, distToNext) > 8000000){
		shipState->ResetPosition();
		return;
	}
	/// Go towards current waypoint.
	Vector3f currDirr = (shipEntity->rotationMatrix * Vector4d(0,0,-1,1)).NormalizedCopy();
	Vector3f dirToNext = (nextWaypointAverage - shipEntity->position).NormalizedCopy();

	float currDirrDotNextDirr = currDirr.DotProduct(dirToNext);

	/// Turning
	if (autoTurn){
		Vector3f currDirrXZ = Vector3f(currDirr.x, 0.0f, currDirr.z);
		currDirrXZ.Normalize();
		float currentAngle = GetAngler(currDirrXZ.x, currDirrXZ.z);
		Vector3f reqDirrXZ = Vector3f(dirToNext.x, 0.0f, dirToNext.z);
		reqDirrXZ.Normalize();
		float requestedAngle = GetAngler(reqDirrXZ.x, reqDirrXZ.z);
		/// While testing, disable any turns just.
		//	shipState->StopTurnLeft();
		//	shipState->StopTurnRight();
		float angleDiff = requestedAngle - currentAngle;
		/// Convert to interval between -PI and +PI
		if (angleDiff > PI){
			angleDiff = 2 * PI - angleDiff;
		}
		if (angleDiff < -PI){
			angleDiff = 2 * PI + angleDiff;
		}

		/// If far right, turn far left.
		if (angleDiff < -PI/4){
			shipState->Turn(-1.0f);
		}
		/// Medium, turn a bit more?
		else if (angleDiff < -PI/8)
			shipState->Turn(angleDiff / (PI/4));
		/// If not so far right, turn a bit left.
		else if (angleDiff < -0.01f){
			shipState->Turn(angleDiff / (PI/2));
		}
		else if (angleDiff > PI/4){
			shipState->Turn(1.0f);
		}
		else if (angleDiff > PI/8)
			shipState->Turn(angleDiff / (PI/4));
		else if (angleDiff > 0.01f){
			shipState->Turn(angleDiff / (PI/2));
		}
		else {
			shipState->StopTurning();
		}
	}

	/// Thrusting
	if (autoThrust){
		/// Go forward as long as we're not totally off! o.o
		if (currDirrDotNextDirr > 0){
			shipState->Accelerate();
			if (currDirrDotNextDirr > 0.9f && distToNext > 100000){
				shipState->Boost();
			}
		}
		else {
			shipState->StopAccelerating();
		}
	}

	// Check once in a while maybe ?
	Entity * e = gameState->GetCheckpoint(player->checkpointsPassed);
	float distanceToCheckpoint = (e->position - shipEntity->position).Length();
	if (distanceToCheckpoint < closestDistance){
		closestDistance = distanceToCheckpoint;
	}
	// Reset if we missed a checkpoint.
	else if (distanceToCheckpoint > closestDistance * 2.0f + 200.0f){
		shipState->ResetPosition();
		closestDistance = 100000000000000.0f;
	}

}

void AIRacer::CalculateNextAverage(){
	/// Set next to the next one along ze path
	int currentIndex = path->GetIndex(closestWaypoint);
	nextWaypoints.Clear();
	nextWaypointAverage = Vector3f();
#define WAYPOINTS_TO_CONSIDER	3
	for (int i = 1; i <= WAYPOINTS_TO_CONSIDER; ++i){
		Waypoint * wp = path->GetWaypoint(currentIndex+i);
		nextWaypoints.Add(wp);
		nextWaypointAverage += wp->position;
	}
	nextWaypointAverage /= WAYPOINTS_TO_CONSIDER;
}

// WOshi.
void AIRacer::ProcessMessage(Message * message){
	String msg = message->msg;
	if (msg  == "CheckpointPassed" || msg == "PassedCheckpoint"){
		closestDistance = 100000000000000.0f;
	}
	else {
		assert(false && "Undefined messag recevied in AIRacer::ProcessMessage!");
	}
}

