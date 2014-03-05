// Emil Hedemalm
// 2013-07-11
// A Space Race player!

#include "Game/GameType.h"

#include "SRPlayer.h"
#include "Ship.h"

SRPlayer::SRPlayer()
: Player(){
	Nullify();
}

SRPlayer::SRPlayer(String name)
: Player(name)
{
	Nullify();
	type = "SRPlayer";
}

SRPlayer::SRPlayer(String name, Ship * ship, int clientIndex)
: Player(name), ship(ship){
	type = "SRPlayer";
    assert(ship != NULL);
 //   std::cout<<"\nShip: "<<ship->name<<" "<<ship->source;
	viewport = NULL;
	this->clientIndex = clientIndex;
}
void SRPlayer::Nullify(){
	ship = NULL;
	viewport = NULL;
}

SRPlayer::~SRPlayer(){
	/// Should we destruct ship here? Nay, the ShipManager probably handles it..!
}

/// Resets the below stats to default values.
void SRPlayer::ResetRacingStatistics(){
	checkpointsPassed = 0;
	lapsFinished = 0;
	finished = false;
	lastLapStart = 0.0f;
	position = 0;
	lapTime = 0;
	lapTimes.Clear();
	totalTime = 0;
}
