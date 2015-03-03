/// Emil Hedemalm
/// 2015-03-03
/// Spawn group, yo.

#ifndef SPAWN_GROUP_H
#define SPAWN_GROUP_H

#include "Ship.h"
#include "Time/Time.h"

/// Types. Default LINE_X?
namespace Formation {
enum {
	LINE_X,
	LINE_Y,
	V_X, /// Typical V-bird-formation, flying X-wise.
	V_Y, /// Typical V-bird-formation, flying Y-wise.
	FORMATIONS,
};
	String GetName(int forFormationType);
};

class SpawnGroup 
{
public:
	SpawnGroup();
	/// Spawns ze entities.
	void Spawn();
	/// o.o
	String name;
	String shipType;
	Time spawnTime;
	Vector3f groupPosition;
	/// Number along the formation bounds.
	int number;
	// See enum above.
	int formation;
	void ParseFormation(String fromString);
	/// Usually just 1 or 2 sizes are used (X,Y)
	Vector3f size;
private:
	// ?!
	Entity * SpawnShip(ConstVec3fr atPosition);
};


#endif