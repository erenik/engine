/// Emil Hedemalm
/// 2014-07-17
/// Property for the ball and other entities in a simple Pong-game.

#ifndef PONG_BALL_PROPERTY_H
#define PONG_BALL_PROPERTY_H

#include "Entity/EntityProperty.h"
#include "Random/Random.h"
#include "Time/Time.h"

#define GOAL_MESSAGE "PongBallGoal"

class Pong;
class ParticleEmitter;

class PongBallProperty : public EntityProperty 
{
public:
	PongBallProperty(Pong * game, Entity * owner, float defaultMinHorizontalVel);
	virtual ~PongBallProperty();
	/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
	static int ID();

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	/// Resets velocity stuffs to default.
	void OnSpawn();
	/// If reacting to collisions...
	virtual void OnCollision(Collision & data);

	/// Makes this ball sleep, unregistering it from physics/graphics?
	void Sleep();

	/// Plays target SFX, but only if a sufficient amount of time has elapsed since the last sfx, so that the colission system, if failing, doesn't spam the system.
	void PlaySFX(String source, float volume = 1.f);

	/// Defaults. Current values are reset to these upon re-spawn.
	float defaultMinHorizontalVel;

	/// Set to true once reaching the goal, to ensure it does not trigger more scores.
	bool sleeping;
	/// Minimum velocity, may vary with time
	float minimumHorizontalVelocity;
	float velocityIncreasePerBounce;
	float maxYVel;
private:

	Time ballStoppedStartTime;			

	// Emitter which may emit some particles where the ball has gone.
	ParticleEmitter * pe;

	Time lastSFX;

	Random pongBallRand;

	Pong * game;
};

#endif
