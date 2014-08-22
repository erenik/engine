/// Emil Hedemalm
/// 2014-07-28
/** Class that weaves together all elements of a Space-shooter game.
	Encapsulation like this makes it easy to switch in/out various small/mini-games as pleased.
*/

#include "MathLib.h"
#include "Game/Game.h"
#include "Random/Random.h"

class PongPlayerProperty;
class PongBallProperty;
class Entity;
class Message;

class Pong : public Game
{
public:
	Pong();
	~Pong();

	/// Allocates and sets things up.
	void Initialize();

	/// o--0
	virtual void ProcessMessage(Message * message);
	/// Call on a per-frame basis.
	virtual void Process();
	/// Fetches all entities concerning this game.
	List<Entity*> GetEntities();


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

	// Sets new amount of balls and resets the game.
	void SetNumBalls(int num);
	void SetZ(float z);
	void SetInitialBallSpeed(float f);
	void SetAISpeed(float sp);
	void SetFrameSize(Vector2f size);
	void SetPaddleScale(float f);

	void SetPlayer1PositionY(float y);
	void SetPlayer2PositionY(float y);

	void Reset();

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
	/// Size of the frame within which the game will be played...!
	Vector2f frameSize;
	/// X-position for player 1 and 2 respectively.
	Vector2f playerLines;
	/// X-position for the 2 scoreboards.
	Vector2f scoreColumns;
	/// Constant z used for most all entities.
	float z;

	// Default 1
	int numBalls;

	int gameState;
	enum 
	{
		SETTING_UP_PLAYFIELD,
		GAME_BEGUN,
	};

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

	// random number generator.
	Random pongRand;
};



