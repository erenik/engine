// Emil Hedemalm
// 2014-04-18
// A RuneRPG player

#include "RRPlayer.h"
#include "Battle/RuneBattler.h"

RRPlayer::RRPlayer(String name)
: Player(name)
{
	playerState = NULL;
	entity = NULL;
	/// Set default values to the battler as needed
	battler.isAI = false;
	battler.name = name;
	isReady = false;
}

/// Be sure to deallocate all data, yo...
RRPlayer::~RRPlayer(){
}

RuneBattler * RRPlayer::Battler(){
	return &battler;
}