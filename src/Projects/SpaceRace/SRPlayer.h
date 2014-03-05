// Emil Hedemalm
// 2013-07-11
// A Space Race player!

#ifndef SPACE_RACE_PLAYER_H
#define SPACE_RACE_PLAYER_H

#include "Player/Player.h"
class Ship;
class Viewport;

class SRPlayer : public Player {
public:
	SRPlayer();
	/// Retard constructor below. Assertion for ship should be handled elsewhere.
	SRPlayer(String name);
	/// Creates a Space Race player
	SRPlayer(String name, Ship * ship, int clientIndex = -1);
	void Nullify();
	/// Virtual destructor so inheritence destruction works.
	virtual ~SRPlayer();

	/// They of course want a ship-type!
	Ship * ship;
	/// And times for when they reached goal.
	float goalTime;
	/// Add more statistics later, hm?

	/// Resets the below stats to default values.
	void ResetRacingStatistics();

	/// Lap statistics. Should be reset before every new race!
	int checkpointsPassed;
	int lapsFinished;
	bool finished;
	/// Current position in the race!
	int position;
	/// Lap time in milliseconds.
	int lapTime;
	// Lap times in milliseconds.
	List<int> lapTimes;
	/// Total time in milliseconds.
	float totalTime;
	int lastLapStart;

private:


};

#endif;
