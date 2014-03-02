// Emil Hedemalm
// 2013-07-28

#include "Game/GameType.h"

// #ifdef SPACE_RACE

#ifndef AI_RACER_H
#define AI_RACER_H

#include "AI/AI.h"
#include "MathLib.h"

class RacingShipGlobal;
class Racing;
class Entity;
class SRPlayer;
class Waypoint;
class Path;

class AIRacer : public AI {
public:
	AIRacer(Entity * shipEntity, SRPlayer * player, RacingShipGlobal * shipState, Racing * gameState);
	virtual ~AIRacer();
	virtual void Process(float timeInSeconds);
	/// Reset so it gathers variables again.
	void Reset();

	// WOshi.
	virtual void ProcessMessage(Message * message);

	/// For disabling/enabling only certain aspects of the AI.
	bool autoThrust;
	bool autoTurn;
private:

	/// For smoother turns.
	void CalculateNextAverage();

	/// To the next checkpoint. If this is exceeded by x2 at any time it means we missed the checkpoint, so reset.
	float closestDistance;
	Entity * shipEntity;
	SRPlayer * player;
	RacingShipGlobal * shipState;
	Racing * gameState;
	const Waypoint * closestWaypoint;
	List<const Waypoint *> nextWaypoints;
	Vector3f nextWaypointAverage;
	int nextCheckpointIndexuu;
	Path * path;
};

#endif

//#endif /// SPACE_RACE
