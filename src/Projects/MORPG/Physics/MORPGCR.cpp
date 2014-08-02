/// Emil Hedemalm
/// 2014-08-01
/// Collision resolver dedicated to the MORPG-project.

#include "MORPGCR.h"

#include "Physics/Collision/Collision.h"

/// Resolves collisions.
int MORPGCR::ResolveCollisions(List<Collision> collisions)
{
	// Use inherited crap.
	return CollisionResolver::ResolveCollisions(collisions);
}

/// Returns false if the colliding entities are no longer in contact after resolution.
bool MORPGCR::ResolveCollision(Collision & c)
{
	return false;
}


