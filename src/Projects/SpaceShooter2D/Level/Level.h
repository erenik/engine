/// Emil Hedemalm
/// 2015-01-21
/// Level.

#ifndef LEVEL_H
#define LEVEL_H

#include "Ship.h"
#include "Color.h"

extern bool gameTimePaused;
extern bool defeatedAllEnemies;
extern bool failedToSurvive;

struct ShipColorCoding 
{
	String ship;
	Vector3i color;
};


class SpawnGroup;
class Camera;
class Level;
class LevelMessage;

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
	/// Creates player entity within this level. (used for spawning)
	Entity * AddPlayer(Ship * playerShip, ConstVec3fr atPosition = Vector3f(-50.f, 10.f, 0));
	void SetupCamera();
	/// o.o
	void Process(int timeInMs);
	void ProcessMessage(Message * message);
	void ProceedMessage();
	void SetTime(Time newTime);
	/// enable respawing on shit again.
	void OnLevelTimeAdjusted();
	Entity * ClosestTarget(bool ally, ConstVec3fr position);
	
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

	/// Default 0.
	int endCriteria;
	enum 
	{
		NO_MORE_ENEMIES,
		EVENT_TRIGGERED,
	};
	bool levelCleared;

	/// Displayed ones, that is.
	LevelMessage * activeLevelMessage;
private:
	// Check spawn groups.
	bool LevelCleared();
};

#endif
