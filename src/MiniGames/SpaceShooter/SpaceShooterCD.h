/// Emil Hedemalm
/// 2014-07-16
/// Collision-detection for a Breakout-type game

#ifndef SPACE_SHOOTER_CD_H
#define SPACE_SHOOTER_CD_H

#include "Physics/CollisionDetector.h"
#include "Entity/Entity.h"

/// Add all pong-entities to the Pong-specific collision filter.
#define SPACE_SHOOTER_PHYSICS_CATEGORY_MAIN (1 << 4) // 16
#define SPACE_SHOOTER_PHYSICS_CATEGORY_PLAYER (1 << 5) // 32

class SpaceShooterCD : public CollisionDetector 
{
public:
	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<EntityPair> & pairs, List<Collision> & collisions);

	/// Brute-force method. Does not rely on other structures that require further updates. All entities are present in the list.
	virtual int DetectCollisions(List<Entity*> & entities, List<Collision> & collisions);

	/// Detects collisions between two entities. Method used is based on physics-shape. Sub-class to override it.
	virtual int DetectCollisions(Entity * one, Entity * two, List<Collision> & collisions);

};

#endif

