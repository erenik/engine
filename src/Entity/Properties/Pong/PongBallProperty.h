/// Emil Hedemalm
/// 2014-07-17
/// Property for the ball and other entities in a simple Pong-game.

#include "Entity/EntityProperty.h"

#include "Random/Random.h"

#define GOAL_MESSAGE "PongBallGoal"

class PongBallProperty : public EntityProperty 
{
public:
	PongBallProperty(Entity * owner, float defaultMinHorizontalVel);
	/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
	virtual int ID();


	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

	/// Resets velocity stuffs to default.
	void OnSpawn();

	/// Defaults. Current values are reset to these upon re-spawn.
	float defaultMinHorizontalVel;

	/// Set to true once reaching the goal, to ensure it does not trigger more scores.
	bool sleeping;
	/// Minimum velocity, may vary with time
	float minimumHorizontalVelocity;
	float velocityIncreasePerBounce;
	float maxYVel;
private:
	Random pongBallRand;
};

