/// Emil Hedemalm
/// 2014-07-23
/// Property for the bricks in a Breakout-type game.

#include "Entity/EntityProperty.h"
#include "MathLib.h"
#include "Time/Time.h"

struct Collision;

class BreakoutBrickProperty : public EntityProperty 
{
public:
	BreakoutBrickProperty(Entity * owner);
	
	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);

	// Maybe spawn some power-ups?
	void OnBroken();

	/// Hits needed to actually break it.
	int hitsNeeded;
	/// ye.
	int hitsTaken;
	/// Score this brick will yield.
	int score;

	/// After "destroyed", sleeping and invisible.
	bool sleeping;
private:
};

