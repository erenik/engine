/// Emil Hedemalm
/// 2014-07-28
/** Class that weaves together all elements of a Space-shooter game.
	Encapsulation like this makes it easy to switch in/out various small/mini-games as pleased.
*/

#include "MathLib.h"
#include "Game/Game2D.h"
#include "Random/Random.h"
#include "OS/Sleep.h"

class PongPlayerProperty;
class PongBallProperty;
class Entity;
class Message;
class PongIntegrator;
class PongCD;
class PongCR;
class Sparks;

class Pong : public Game2D
{
public:
	Pong();
	virtual ~Pong();

	/// Allocates and sets things up.
	void Initialize();

	/// o--0
	virtual void ProcessMessage(Message * message);
	/// Call on a per-frame basis.
	virtual void Process();
	/// Fetches all entities concerning this game.
	List<Entity*> GetEntities();
	/// Fetches all static entities.
	virtual List<Entity*> StaticEntities();

	/// Sets up physics integrator, etc. as needed.
	void SetupPhysics();
	/// Sets up stuff specific for the entities in this little game.
	void SetupPhysics(List<Entity*> forEntities);
	// Resets the ball in the middle, giving it an initial velocity toward one side.
	void SpawnBalls();
	// Update text on both entities displaying the scores.
	void OnScoreUpdated();
	/// Updates all balls' properties based on given settings.
	void UpdateBallProperties();

	/// o.o Called from ball maybe.
	void OnGoal(ConstVec3fr atPosition);

	// Sets new amount of balls and resets the game.
	void SetNumBalls(int num);
	void SetInitialBallSpeed(float f);
	void SetAISpeed(float sp);
	void SetPaddleScale(float f);
	virtual void SetZ(float newZ);


	void SetPlayer1PositionY(float y);
	void SetPlayer2PositionY(float y);

	virtual void Reset();

	Sparks * particleSystem;

private:

	// Including player lines and score columns, based on frameSize
	void RecalculatePlayfieldLines();

	/// Value from 0.0 to 1.0, distance from center to the sides, where the players should be.
	float distanceFromCenter;
	float ballSpeedIncreasePerCollision;
	float ballScale;
	float aiSpeed;
	float initialBallSpeed;
	/// Scale of the player paddles..!
	float paddleScale;
	/// X-position for player 1 and 2 respectively.
	Vector2f playerLines;
	/// X-position for the 2 scoreboards.
	Vector2f scoreColumns;
	
	// Default 1
	int numBalls;

	// All entities.
	List<Entity*> entities;
	List<Entity*> players;
	List<Entity*> frame;
	List<Entity*> scoreEntities;
	Entity * player1, * player2, * goals, * horizontalBars;

	/// l-l-
	List<Entity*> balls;
	

	Entity * score1Entity, * score2Entity;

	/// o-o
	List<PongBallProperty*> ballProperties;
	PongPlayerProperty * player1Properties;
	PongPlayerProperty * player2Properties;

	
	PongIntegrator * pongIntegrator;
	PongCR * pongCR;
	PongCD * pongCD;
	

	// random number generator.
	Random pongRand;
};



