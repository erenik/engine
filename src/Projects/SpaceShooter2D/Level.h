/// Emil Hedemalm
/// 2015-01-21
/// Level.

#ifndef LEVEL_H
#define LEVEL_H

#include "Ship.h"
#include "Color.h"

struct ShipColorCoding 
{
	String ship;
	Vector3i color;
};


struct LevelMessage 
{
	LevelMessage();
	// UI
	void Display();
	void Hide();

	bool displayed;
	Time startTime;
	Time stopTime;
	int textID;
};

class SpawnGroup;
class Camera;
class Level;

extern Camera * levelCamera;
// Right hand boundary when ships remove initial invulnerability.
extern float removeInvuln;
/// Position in X at which ships are spawned. Before that, their entity representations have not yet been created.
extern float spawnPositionRight;
/// Left X limit for despawning ships.
extern float despawnPositionLeft;
extern Level * activeLevel;

class Level 
{
public:
	Level();
	virtual ~Level();
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
	List<Ship*> ships;

	/// Default.. 20.0. Dictates movable region in Y, at least.
	float height;

	/// New spawn style.
	List<SpawnGroup*> spawnGroups;
	/// o.o
	List<LevelMessage*> messages;

	/// To determine when things spawn and the duration of the entire "track".
	int millisecondsPerPixel;
	/// Music source to play.
	String music;
	/// Goal position in X.
	int goalPosition;

	Vector3f starSpeed;
	Color starColor;
private:
	// Check spawn groups.
	bool LevelCleared();
};

#endif
