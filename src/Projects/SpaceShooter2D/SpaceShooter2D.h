/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "AppStates/AppState.h"

class Ship 
{
public:
	Ship();
	~Ship();
	/// Creates new ship of specified type.
	static Ship New(String type);
	String type;
	// Default false
	bool spawned;
	Entity * entity;
	// Data details.
	// Spawn position.
	Vector3f position;
	/// As loaded.
	static List<Ship> types;
private:
};

class Level 
{
public:
	/// Ships within.
	List<Ship> ships;
	/// To determine when things spawn and the duration of the entire "track".
	int millisecondsPerPixel;
	/// Music source to play.
	String music;
};


class SpaceShooterIntegrator;
class SpaceShooterCR;
class SpaceShooterCD;

class SpaceShooter2D : public AppState 
{
public:
	SpaceShooter2D();
	virtual ~SpaceShooter2D();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(AppState * nextState);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

private:
	/// Starts a new game. Calls LoadLevel
	void NewGame();
	/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
	void LoadLevel(String levelSource);

	void UpdatePlayerVelocity();

	enum {
		IN_MENU,
		PLAYING_LEVEL,
	};
	int mode;
	SpaceShooterIntegrator * integrator;
	SpaceShooterCR * cr;
	SpaceShooterCD * cd;
	Level level;
	Camera * levelCamera;
	Entity * playerShip;
	List<int> movementDirections;
	/// All ships, including player.
	List<Entity*> shipEntities;
};


