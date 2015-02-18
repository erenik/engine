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
	Entity * dynamic;
	Entity * other;
	if (c.one->physics->type == PhysicsType::DYNAMIC)
	{
		dynamic = c.one;
		other = c.two;
	}
	else 
	{
		dynamic = c.two;
		other = c.one;
	}
	Entity * dynamic2 = other->physics->type == PhysicsType::DYNAMIC? other : NULL;
	Entity * staticEntity;
	Entity * kinematicEntity;
	if (dynamic == c.one)
		other = c.two;
	else
		other = c.one;
	if (!dynamic2)
	{
		staticEntity = other->physics->type == PhysicsType::STATIC? other : NULL;
		kinematicEntity = other->physics->type == PhysicsType::KINEMATIC? other : NULL;
	}
//	std::cout<<"\nCollision: "<<c.one->name<<" "<<c.two->name;
	// Collision..!
	// Static-Dynamic collision.
	if (!dynamic2)
	{
		PhysicsProperty * pp = dynamic->physics;
		// Skip
		if (kinematicEntity)
			return true;

		/// Flip normal if dynamic is two.
		if (dynamic == c.two)
			c.collisionNormal *= -1;
		// Default plane? reflect velocity upward?
		if (c.collisionNormal.y > 0.9f)
		{
			// Reflect based on the normal.
			float velDotNormal = pp->velocity.DotProduct(c.collisionNormal);
			
			/// This will be used to reflect it.
			Vector3f velInNormalDir = c.collisionNormal * velDotNormal;
			Vector3f velInTangent = pp->velocity - velInNormalDir;
			
			Vector3f newVel = velInTangent * (1 - pp->friction) + velInNormalDir * (-pp->restitution);

			/// Apply resitution and stuffs.
			dynamic->physics->velocity = newVel;

			dynamic->position += AbsoluteValue(c.distanceIntoEachOther) * c.collisionNormal;
			/// If below threshold, sleep it.
			if (dynamic->physics->velocity.Length() < 0.1f)
			{
				// Sleep eeet.
				dynamic->physics->state |= PhysicsState::AT_REST;
				// Nullify velocity.
				dynamic->physics->velocity = Vector3f();
			}
//			std::cout<<"\nCollision normal: "<<c.collisionNormal;
		}	
	}
	// Dynamic-dynamic collision.
	else 
	{
	
	}


	// Check collision normal.


	return false;
}


