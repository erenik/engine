/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a Breakout-type game

#ifndef SPACE_SHOOTER_CD_H
#define SPACE_SHOOTER_CD_H

#include "Physics/CollisionDetector.h"

/// Add all pong-entities to the Pong-specific collision filter.
#define SPACE_SHOOTER_PHYSICS_CATEGORY_MAIN (1 << 4) // 16
#define SPACE_SHOOTER_PHYSICS_CATEGORY_PLAYER (1 << 5) // 32

class SpaceShooterCD : public CollisionDetector 
{
public:
	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> entities, List<Collision> & collisions);

};

#endif

