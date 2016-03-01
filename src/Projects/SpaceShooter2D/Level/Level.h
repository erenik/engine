/// Emil Hedemalm
/// 2015-01-21
/// Level.

#ifndef LEVEL_H
#define LEVEL_H

#include "../Base/Ship.h"
#include "Color.h"

extern bool gameTimePaused;
extern bool defeatedAllEnemies;
extern bool failedToSurvive;

void GenerateLevel (String arguments);

struct ShipColorCoding 
{
	String ship;
	Vector3i color;
};

class Explosion;
class SpawnGroup;
class Camera;
class Level;
class LevelMessage;
class Message;

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
	/// Called regularly to despawn superfluous stuffs.
	void Cleanup();
	/// Deletes all ships, spawngroups, resets variables to defaults.
	void Clear();
	bool FinishedSpawning();
	bool Load(String fromSource);
	/// Starts BGM, starts clocks/timers if any, etc.
	void OnEnter();
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
	/// o.o'
	void Explode(Weapon & weapon, Entity * causingEntity, bool enemy);
	/// Returns ships close enough to given point. Returns distance to them too. Checks only to center of ship, not edges.
	List<Ship*> GetShipsAtPoint(ConstVec3fr position, float maxRadius, List<float> & distances); 

	void RemoveRemainingSpawnGroups();
	void RemoveExistingEnemies();

	/// Yes.
	List<Ship*> PlayerShips();

	/// In format mm:ss.ms
	void JumpToTime(String timeString);

	String source;

	/// Enemy ships within.
	List<Ship*> enemyShips, alliedShips, ships;

	/// Default.. 20.0. Dictates movable region in Y, at least.
	float height;
	/// Default 0.
	enum 
	{
		NEVER,
		SURVIVE_ALL_SPAWN_GROUPS, NO_MORE_ENEMIES = SURVIVE_ALL_SPAWN_GROUPS,
		EVENT_TRIGGERED,
	};
	int endCriteria;


	/// New spawn style.
	List<SpawnGroup*> spawnGroups;
	/// o.o
	List<LevelMessage*> messages;

	List<Explosion*> explosions;

	/// To determine when things spawn and the duration of the entire "track".
	int millisecondsPerPixel;
	/// Music source to play.
	String music;
	/// Goal position in X.
	int goalPosition;

	Vector3f starSpeed;
	Color starColor;

	bool levelCleared;

	/// Displayed ones, that is.
	LevelMessage * activeLevelMessage;
private:
	// Check spawn groups.
	bool LevelCleared();
};

#endif
