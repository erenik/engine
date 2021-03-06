/// Emil Hedemalm
/// 2014-08-06
/// Collision resolver suitable for a first-person game.

#ifndef FIRST_PERSON_CR_H
#define FIRST_PERSON_CR_H

#include "Physics/CollisionResolver.h"

class FirstPersonCR : public CollisionResolver 
{
public:
	/// Resolves collisions.
	virtual int ResolveCollisions(List<Collision> collisions);
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c);


};

#endif
