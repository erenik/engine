// Emil Hedemalm
// 2013-07-11
// A Space Race player!

#include "ScrollerPlayer.h"

ScrollerPlayer::ScrollerPlayer(String name)
: Player(name)
{
	playerState = NULL;
	entity = NULL;
}

/// Be sure to deallocate all data, yo...
ScrollerPlayer::~ScrollerPlayer(){
}
