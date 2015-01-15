/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a Breakout-type game

#ifndef BREAKOUT_CD_H
#define BREAKOUT_CD_H

#include "Physics/CollisionDetector.h"

/// Add all pong-entities to the Pong-specific collision filter.
#define BREAKOUT_PHYSICS_CATEGORY_MAIN (1 << 2) // 4
#define BREAKOUT_PHYSICS_CATEGORY_PLAYER (1 << 3) // 8

class BreakoutCD : public CollisionDetector 
{
public:
	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> entities, List<Collision> & collisions);

};

#endif

