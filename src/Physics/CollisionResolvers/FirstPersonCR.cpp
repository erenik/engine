/// Emil Hedemalm
/// 2014-08-06
/// Collision resolver suitable for a first-person game.

#include "FirstPersonCR.h"
#include "Physics/Collision/Collision.h"

#include "Physics/PhysicsManager.h"

#include "Debug.h"

/// Resolves collisions.
int FirstPersonCR::ResolveCollisions(List<Collision> collisions)
{
	// Use inherited crap.
	return CollisionResolver::ResolveCollisions(collisions);
}

/// Returns false if the colliding entities are no longer in contact after resolution.
bool FirstPersonCR::ResolveCollision(Collision & c)
{
	EntitySharedPtr dynamic;
	EntitySharedPtr other;
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
	if (c.one->physics->noCollisionResolutions || c.two->physics->noCollisionResolutions)
		return false;

	// Retardation?
//	if (dynamic->physics->lastCollisionMs + dynamic->physics->minCollisionIntervalMs > physicsNowMs)
	//	return false;

	dynamic->physics->lastCollisionMs = physicsNowMs;

	EntitySharedPtr dynamic2 = (other->physics->type == PhysicsType::DYNAMIC) ? other : NULL;
	EntitySharedPtr staticEntity;
	EntitySharedPtr kinematicEntity;
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
		// Skip? No.
//		if (kinematicEntity)
//			return true;

		/// Flip normal if dynamic is two.
		if (dynamic == c.two)
			c.collisionNormal *= -1;

		if (c.collisionNormal.y > 0.8f)
			pp->lastGroundCollisionMs = physicsNowMs;

		// Default plane? reflect velocity upward?
		// Reflect based on the normal.
		float velDotNormal = pp->velocity.DotProduct(c.collisionNormal);
		
		/// This will be used to reflect it.
		Vector3f velInNormalDir = c.collisionNormal * velDotNormal;
		Vector3f velInTangent = pp->velocity - velInNormalDir;
		
		Vector3f newVel = velInTangent * (1 - pp->friction) + velInNormalDir * (-pp->restitution);

		/// Apply resitution and stuffs.
		dynamic->physics->velocity = newVel;
		assert(dynamic->parent == 0); 
		/// Adjusting local position may not help if child entity.
		// Old code
		Vector3f moveVec = AbsoluteValue(c.distanceIntoEachOther) * c.collisionNormal;
//		if (moveVec.x || moveVec.z)
//			std::cout<<"\nMoveVec: "<<moveVec;
		dynamic->localPosition += moveVec;
		/// For double-surface collision resolution (not bouncing through walls..)

		/// For the previously backwards-collisions.
//		if (c.distanceIntoEachOther < 0)
	//		dynamic->localPosition += c.distanceIntoEachOther * c.collisionNormal;
//		else // Old code
	//		dynamic->localPosition += AbsoluteValue(c.distanceIntoEachOther) * c.collisionNormal;

		/// If below threshold, sleep it.
		if (dynamic->physics->velocity.Length() < inRestThreshold && c.collisionNormal.y > 0.8f)
		{
			// Sleep eeet.
			dynamic->physics->state |= CollisionState::IN_REST;
			// Nullify velocity.
			dynamic->physics->velocity = Vector3f();
		}
		if (debug == 7)
		{
			std::cout<<"\nCollision resolution: "<<(c.one->name+" "+c.two->name+" ")<<c.collisionNormal<<" onePos"<<c.one->worldPosition<<" twoPos"<<c.two->worldPosition;
		}
	}
	// Dynamic-dynamic collision.
	else 
	{
//		std::cout<<"\nProblem?"	;
		/// Push both apart?
		PhysicsProperty * pp = dynamic->physics, * pp2 = dynamic2->physics;

		/// Flip normal if dynamic is two.
		// See if general velocities align with one or the other? or...
		
//		if (dynamic == c.two)

		;// c.collisionNormal *= -1;

//		std::cout<<"\nNormal: "<<c.collisionNormal;

		// Default plane? reflect velocity upward?		
		/// This will be used to reflect it.
		Vector3f velInNormalDir = c.collisionNormal * pp->velocity.DotProduct(c.collisionNormal);
		Vector3f velInNormalDir2 = c.collisionNormal * pp2->velocity.DotProduct(c.collisionNormal);
		Vector3f velInTangent = pp->velocity - velInNormalDir,
			velInTangent2 = pp2->velocity - velInNormalDir2;
		Vector3f velInTangentTot = velInTangent + velInTangent2,
			velInNormalDirTot = velInNormalDir + velInNormalDir2;		
		/// Use lower value restitution of the two?
		float restitution = 0.5f; // MinimumFloat(pp->restitution, pp2->restitution);
		Vector3f normalDirPart = velInNormalDirTot * restitution * 0.5f;
		Vector3f to2 = (dynamic2->worldPosition - dynamic->worldPosition).NormalizedCopy();
		float partTo2 = normalDirPart.DotProduct(to2);
	//	std::cout<<" normalDirPart: "<<normalDirPart;

		float normalDirPartVal = normalDirPart.Length();

		Vector3f fromCollisionToDyn1 = (dynamic->worldPosition - c.collissionPoint).NormalizedCopy();
		Vector3f fromCollisionToDyn2 = (dynamic2->worldPosition - c.collissionPoint).NormalizedCopy();

		pp->velocity = velInTangent * (1 - pp->friction) + fromCollisionToDyn1 * normalDirPartVal;
		pp2->velocity = velInTangent2 * (1 - pp->friction) + fromCollisionToDyn2 * normalDirPartVal;
		/// Apply resitution and stuffs.
		assert(dynamic->parent == 0);
		/// Adjusting local position may not help if child entity.
		// Old code
		float distanceIntoAbsH = AbsoluteValue(c.distanceIntoEachOther * 0.5f);

		dynamic->localPosition += distanceIntoAbsH * fromCollisionToDyn1;
		dynamic2->localPosition += distanceIntoAbsH * fromCollisionToDyn2;

		/// For double-surface collision resolution (not bouncing through walls..)
		/// If below threshold, sleep it.
		if (dynamic->physics->velocity.Length() < inRestThreshold && c.collisionNormal.y > 0.8f)
		{
			pp->Sleep();
		}
		else
			pp->Activate();
		if (pp2->velocity.Length() < inRestThreshold && c.collisionNormal.y > 0.8f)
		{
			pp2->Sleep();
		}
		else
			pp2->Activate();
		if (debug == 7)
		{
			std::cout<<"\nCollision resolution: "<<(c.one->name+" "+c.two->name+" ")<<c.collisionNormal<<" onePos"<<c.one->worldPosition<<" twoPos"<<c.two->worldPosition;
		}
	}


	// Check collision normal.


	return true;
}


