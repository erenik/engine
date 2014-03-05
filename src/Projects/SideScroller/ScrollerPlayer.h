// Emil Hedemalm
// 2013-07-11

#ifndef RUNE_PLAYER_PLAYER_H
#define RUNE_PLAYER_PLAYER_H

#include "Player/Player.h"

class ScrollerPlayerState;

class ScrollerPlayer : public Player {
public:
	ScrollerPlayer(String name);
	/// Be sure to deallocate all data, yo...
	virtual ~ScrollerPlayer();
	
	/// Player entity state (primarily on map?)
	ScrollerPlayerState * playerState;
	// They of course want a ship-type!
	Entity * entity;
	/// Add more statistics later, hm?
	
private:
};

#endif;