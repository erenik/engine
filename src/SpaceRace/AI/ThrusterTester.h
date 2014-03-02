// Emil Hedemalm
// 2013-07-28

#ifndef THRUSTER_TESTER_H
#define THRUSTER_TESTER_H

#include "AI/AI.h"
#include "MathLib.h"

class RacingShipGlobal;
class Racing;
class Entity;
class SRPlayer;

class ThrusterTester : public AI {
public:
	ThrusterTester(Entity * shipEntity, RacingShipGlobal * shipState);
	virtual ~ThrusterTester();
	virtual void Process(float timeInSeconds);
	/// Reset so it gathers variables again.
	void Reset();

private:
	float timePassed;
	int state;
	Entity * shipEntity;
	SRPlayer * player;
	RacingShipGlobal * shipState;
	Racing * gameState;
	int nextCheckpointIndexuu;
};

#endif