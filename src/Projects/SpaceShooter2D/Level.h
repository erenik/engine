/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "Ship.h"

struct ShipColorCoding 
{
	String ship;
	Vector3i color;
};

extern Camera * levelCamera;

// Right hand boundary when ships remove initial invulnerability.
extern float removeInvuln;
/// Position in X at which ships are spawned. Before that, their entity representations have not yet been created.
extern float spawnPositionRight;
/// Left X limit for despawning ships.
extern float despawnPositionLeft;

class Level 
{
public:
	bool Load(String fromSource);
	// Used for player and camera. Based on millisecondsPerPixel.
	Vector3f BaseVelocity();
	void AddPlayer(Ship * playerShip);
	void SetupCamera();
	/// o.o
	void Process(int timeInMs);
	/// Process target ship.
	void Process(Ship & ship);

	String source;
	/// Ships within.
	List<Ship> ships;
	/// To determine when things spawn and the duration of the entire "track".
	int millisecondsPerPixel;
	/// Music source to play.
	String music;
	/// Goal position in X.
	int goalPosition;

	Vector3f starSpeed;
};
