/// Emil Hedemalm
/// 2014-08-06
/// Collision resolver suitable for a first-person game.

#include "FirstPersonCR.h"
#include "Physics/Collision/Collision.h"

/// Resolves collisions.
int FirstPersonCR::ResolveCollisions(List<Collision> collisions)
{
	// Use inherited crap.
	return CollisionResolver::ResolveCollisions(collisions);
}

/// Returns false if the colliding entities are no longer in contact after resolution.
bool FirstPersonCR::ResolveCollision(Collision & c)
{
	return false;
}


