// Emil Hedemalm
// 2013-07-11
// A Space Race player!

#ifndef RUNE_PLAYER_PLAYER_H
#define RUNE_PLAYER_PLAYER_H

#include "Player/Player.h"
#include "Battle/RuneBattler.h"

class RuneBattler;
class RunePlayerState;

class RunePlayer : public Player {
public:
	RunePlayer(String name);
	/// Be sure to deallocate all data, yo...
	virtual ~RunePlayer();
	
	/// Player entity state (primarily on map?)
	RunePlayerState * playerState;
	// They of course want a ship-type!
	Entity * entity;
	/// Add more statistics later, hm?
	
	RuneBattler * Battler();
private:
	/// Player battler data!
	RuneBattler battler;

};

#endif;