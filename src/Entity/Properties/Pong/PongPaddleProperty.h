/// Emil Hedemalm
/// 2014-07-17
/// Property for the paddles (players) in a simple Pong-game.
/// Contains the logic for a simple AI which can be toggled.

#include "Entity/EntityProperty.h"
#include "MathLib.h"
#include "Time/Time.h"

class PongPaddleProperty : public EntityProperty 
{
public:
	PongPaddleProperty(Entity * owner, Vector2f lookAt, float aiSpeed);
	/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
	virtual int ID();


	/// Time passed in seconds..!
	virtual void Process(int timeInMs);

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
private:
	void StopMovement();
};

