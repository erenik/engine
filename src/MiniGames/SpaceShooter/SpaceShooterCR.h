/// Emil Hedemalm
/// 2014-07-25
/// Custom integrator for Space shooter type games.

#include "Physics/CollisionResolver.h"

class SpaceShooterCR : public CollisionResolver 
{
public:
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c);
	/// Resolves collisions.
	virtual int ResolveCollisions(List<Collision> collisions);
	
};


