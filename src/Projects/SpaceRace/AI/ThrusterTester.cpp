// Emil Hedemalm
// 2013-07-28

#include "ThrusterTester.h"
#include "SpaceRace/EntityStates/RacingShipGlobal.h"
#include "../GameStates/Racing/RacingState.h"
#include "SpaceRace/SRConstants.h"
#include "Maps/MapManager.h"
#include <cstring>
#include <cmath>
#include "Timer/Timer.h"


ThrusterTester::ThrusterTester(Entity * shipEntity, RacingShipGlobal * shipState)
: AI(AI_TYPE_STATE_MACHINE), shipEntity(shipEntity), shipState(shipState)
{
	assert(shipEntity);
	assert(shipState);
	Reset();
	enabled = true;
}
ThrusterTester::~ThrusterTester(){
	// Wosh.
}

/// Reset so it gathers variables again.
void ThrusterTester::Reset(){
	state = 0;
	timePassed = 0.0f;
}

void ThrusterTester::Process(float timeInSeconds){
	timePassed += timeInSeconds;
	// Refill boost auto.
	shipState->RefillBoost(0.1f);
	if (timePassed < 8.0f)
		return;
	timePassed = 0.0f;

	state++;
	switch(state){
		case 1:
			shipState->Accelerate();
			break;
		case 2:
			shipState->StopAccelerating();
			break;
		case 3:
			shipState->Accelerate();
			shipState->Boost();
			break;
		default: 
			state = 0;
			shipState->StopAccelerating();
			shipState->StopBoosting();
			break;
	}
}
