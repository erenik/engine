/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#ifndef COLLISION_RESOLVER_H
#define COLLISION_RESOLVER_H

#include "Physics/PhysicsProperty.h"
#include "Entity/Entity.h"

class CollisionResolver 
{
public:
	CollisionResolver();
	/// Resolves collisions.
	virtual int ResolveCollisions(List<Collision> collisions);
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c) = 0;

	/// Threshold for considering to be in rest and nullifying velocity. Default 0.1 for fast-paced games? Reduce to make playable at lower scales or slower games.
	float inRestThreshold;
private:
};

#endif


