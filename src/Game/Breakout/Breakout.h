/// Emil Hedemalm
/// 2014-07-21
/// 2D ball-based brick-breaking game using computer vision based input.

#include "Random/Random.h"
#include "Game/Game2D.h"

class Entity;
class Texture;
class Message;
class BreakoutBallProperty;
class BreakoutPaddleProperty;
class BreakoutIntegrator;
class BreakoutCR;
class BreakoutCD;

// The classic Pong based on IPM
class Breakout : public Game2D 
{
public:
	Breakout();
	virtual ~Breakout();
	/// Allocates and sets things up.
	virtual void Initialize();
	/// Resets the entire game. Similar to a hardware reset on old console games.
	virtual void Reset();
	/// Call on a per-frame basis.
	virtual void Process();

	void SetBallScale(float scale);
	void SetPaddleScale(float scale);
	void SetBallSpeed(float unitsPerSecond);
	void SetSpeedIncreasePerBounce(float unitsPerSecond);
	void SetAISpeed(float aiSpeed);
	void SetNumBalls(int num);
	virtual void SetZ(float newZ);

	void SetLevel(int level);

	/// For setting position manually o.o
	void SetPlayerPositionX(float x);


	/// Fetches all entities concerning this game.
	virtual List<Entity*> GetEntities();
	/// Fetches all static entities.
	virtual List<Entity*> StaticEntities();

	/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
	virtual void ProcessMessage(Message * message);

private:

	// Call to re-create the playing field as it started out.
	void SetupPlayingField();


	/// Sets up physics integrator, etc. as needed.
	void SetupPhysics();
	/// Sets up stuff specific for the entities in this little game.
	void SetupPhysics(List<Entity*> forEntities);
	/// Spawns bricks for a level. Pattern will be decided later.
	void SpawnBricks(int level);
	void CreateUniformBrickMatrix(Vector2i withBrickColumnsAndRows);
	// Resets the ball in the middle, giving it an initial velocity toward one side.
	void SpawnBalls();
	/// Triggered by the power-up.
	void SpawnNewBall();
	// Update text on both entities displaying the scores.
	void OnScoreUpdated();
	/// Updates all balls' properties based on given settings.
	void UpdateBallProperties();

	// All entities.
	List<Entity*> players;
	List<Entity*> frame;
	List<Entity*> scoreEntities;
	// p=p
	List<Entity*> bricks;

	Entity * player1, 
		* base, // Where the player lives. 
		* topBar, * leftBar, * rightBar;

	/// l-l-
	List<Entity*> balls, powerUpEntities;
	
	/// Absolute co-ordinates of the game field.
	float top, bottom, left, right;

	// For setting up the playing field.
	// Relative space of the active game field between the bottom of the screen and the bricks lower boundary.
	float bottomSpacing;
	/// Relative space of the active game field between the top of the screen and the bricks upper boundary.
	float topSpacing;
	/// Space between the bricks and the sides, for passing the ball up.
	float sideSpace;
	/// Total Y-space (bottomSpacing + topSpacing)
	float totalYSpacing;
		
	/// Initial ball speed.
	float ballSpeedIncreasePerBounce, ballMinVerticalSpeed;
	float aiSpeed;
	float paddleScale, ballScale;
	/// o.o
	int currentLevel;
	int numBalls;

	Entity * score1Entity; // , * score2Entity;

	/// o-o
	List<BreakoutBallProperty*> ballProperties;
	BreakoutPaddleProperty * player1Properties;
//	PongPlayerProperty * player2Properties;

	// random number generator.
	Random breakoutRand;


	
	BreakoutIntegrator * breakoutIntegrator;
	BreakoutCR * breakoutCR;
	BreakoutCD * breakoutCD;
	
};







