/// Emil Hedemalm
/// 2014-08-01
/// Collision resolver dedicated to the MORPG-project.

#include "Physics/CollisionResolver.h"

class MORPGCR : public CollisionResolver 
{
public:
	/// Resolves collisions.
	virtual int ResolveCollisions(List<Collision> collisions);
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c);

};

