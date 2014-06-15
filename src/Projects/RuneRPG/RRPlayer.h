// Emil Hedemalm
// 2013-07-11
// A RuneRPG player

#ifndef RUNE_PLAYER_PLAYER_H
#define RUNE_PLAYER_PLAYER_H

#include "Player/Player.h"
#include "Battle/RuneBattler.h"

class RuneBattler;
class RREntityState;

class RRPlayer : public Player {
public:
	RRPlayer(String name = "");
	/// Be sure to deallocate all data, yo...
	virtual ~RRPlayer();
	
	/// Player entity state (primarily on map?)
	RREntityState * playerState;
	/// Entity when walking around the map.
	Entity * mapEntity;
	/// Entity created and linked to the battler.
	Entity * battlerEntity;
	/// Add more statistics later, hm?
	
	RuneBattler * Battler();

	/// If ready?! Used for starting game, but maybe also pauses in-game.
	bool isReady;

private:
	/// Player battler data!
	RuneBattler battler;

};

#endif;