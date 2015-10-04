/// Emil Hedemalm
/// 2015-03-03
/// Spawn group, yo.

#ifndef SPAWN_GROUP_H
#define SPAWN_GROUP_H

#include "SpaceShooter2D/Base/Ship.h"
#include "Time/Time.h"

/// Types. Default LINE_X?
namespace Formation {
enum {
	LINE_X,
	DOUBLE_LINE_X,
	LINE_Y,
	LINE_XY,
	V_X, /// Typical V-bird-formation, flying X-wise.
	V_Y, /// Typical V-bird-formation, flying Y-wise.
	SWARM_BOX_XY, /// Random-based swarm with some minimum threshold distance between each ship, skipping ships if area is not large enough.
	FORMATIONS,
};
	String GetName(int forFormationType);
};

class SpawnGroup 
{
public:
	SpawnGroup();
	void Reset();
	/// Spawns ze entities.
	void Spawn();
	void OnShipDestroyed(Ship * ship);
	void OnShipDespawned(Ship * ship);
	/// o.o
	String name;
	String shipType;
	bool spawned, defeated, survived;
	Time spawnTime;
	Vector3f groupPosition;
	/// Number along the formation bounds.
	int number;
	// See enum above.
	int formation;
	void ParseFormation(String fromString);
	/// Usually just 1 or 2 sizes are used (X,Y)
	Vector3f size;

	/// o-o relative to 5.0 for max.
	float relativeSpeed;
	bool shoot;
	/// If true, pauses game time, until all ships of the group have either been destroyed or despawned by exiting the screen.
	bool pausesGameTime;
	int shipsDefeatedOrDespawned;
	int shipsDespawned, shipsDefeated;
private:
	// ?!
	Entity * SpawnShip(ConstVec3fr atPosition);
};


#endif