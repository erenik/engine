/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for pong.

#include "Physics/CollisionResolver.h"

class PongCR : public CollisionResolver 
{
public:
	/// Returns false if the colliding entities are no longer in contact after resolution.
	virtual bool ResolveCollision(Collision & c);
};


