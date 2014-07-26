/// Emil Hedemalm
/// 2014-07-17
/// Property for the balls in a Breakout-type game.

#include "Entity/EntityProperty.h"

#include "Random/Random.h"

#define GOAL_MESSAGE "PongBallGoal"

class BreakoutBallProperty : public EntityProperty 
{
public:
	BreakoutBallProperty(Entity * owner, float defaultMinHorizontalVel);

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	// o-o play sfx!
	void OnCollision(Collision & c);

	/// Resets velocity stuffs to default.
	void OnSpawn();

	/// Defaults. Current values are reset to these upon re-spawn.
	float defaultMinVerticalVel;

	/// Set to true once reaching the goal, to ensure it does not trigger more scores.
	bool sleeping;
	/// Minimum velocity, may vary with time
	float minimumVerticalVelocity;
	float velocityIncreasePerBounce;
	float maxYVel;
private:
	Random pongBallRand;
};

