/// Emil Hedemalm
/// 2014-07-17
/// Property for the paddles (players) in a simple Pong-game.
/// Contains the logic for a simple AI which can be toggled.

#include "Entity/EntityProperty.h"
#include "MathLib.h"
#include "Time/Time.h"

#include "BreakoutPowerupProperty.h"

class BreakoutPaddleProperty : public EntityProperty 
{
public:
	BreakoutPaddleProperty(Entity * owner, Vector2f lookAt, float aiSpeed);

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);


	/// Adds the power up, and sets its starting time. Calls OnPowerupsUpdated afterward.
	void AddPowerup(Powerup power);

	// Call after the list changes.
	void OnPowerupsUpdated();
	// the list
	List<Powerup> powerups;

	// Starts at 0.
	int score;
	/// Direction this paddle is facing.
	Vector2f lookAt;
	/// Minimum velocity, may vary with time
	bool aiEnabled;
	/// Movement speed.
	float aiSpeed;
	/// To notify the AI.
	Time lastUserInput;

	/// Required.
	Vector3f initialScale;
private:
	void StopMovement();
};

