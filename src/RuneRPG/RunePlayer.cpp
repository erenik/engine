// Emil Hedemalm
// 2013-07-11
// A Space Race player!

#include "RunePlayer.h"
#include "Battle/RuneBattler.h"

RunePlayer::RunePlayer(String name)
: Player(name)
{
	playerState = NULL;
	entity = NULL;
	/// Set default values to the battler as needed
	battler.isAI = false;
	battler.name = name;
}

/// Be sure to deallocate all data, yo...
RunePlayer::~RunePlayer(){
}

RuneBattler * RunePlayer::Battler(){
	return &battler;
}