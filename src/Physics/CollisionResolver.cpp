/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#include "CollisionResolver.h"
#include "Collision/Collision.h"

CollisionResolver::CollisionResolver()
{
	inRestThreshold = 0.1f;
}


/// Resolves collisions.
int CollisionResolver::ResolveCollisions(List<Collision> collisions)
{
	int numResolved = 0;
	if (collisions.Size())
		;//std::cout<<"\n"<<collisions.Size()<<" collisions to resolve.";

	float maxLikelyhood;
	int deepestCollisionIndex;

	for (int i = 0; i < collisions.Size(); ++i)
	{
		Collision & c = collisions[i];
		bool didSomething = ResolveCollision(c);
		numResolved += didSomething;
		if (didSomething)
		{
			++numResolved;
		}
	}
	collisions.Clear();

		/*
	Collision * mostLikelyCollision = 0;
	while(collisions.Size())
	{
		maxLikelyhood = -1000.f;

		/// Solve the deepest collisions first.
		for (int i = 0; i < collisions.Size(); ++i)
		{
			Collision & c = collisions[i];
			float collisionDepth = AbsoluteValue(c.distanceIntoEachOther);
			float alignmentToVelocity;
			Vector3f relVel = c.one->physics->velocity - c.two->physics->velocity;
			relVel.Normalize();
			alignmentToVelocity = relVel.DotProduct(c.collisionNormal);
			float absRelVelDotNormal = AbsoluteValue(alignmentToVelocity);
			float likelyhood = absRelVelDotNormal * 2.f - collisionDepth;
			if (likelyhood > maxLikelyhood)
			{
				mostLikelyCollision = &c;
				maxLikelyhood = likelyhood;
				deepestCollisionIndex = i;
			}
		}
		if (mostLikelyCollision)
		{
			bool didSomething = ResolveCollision(*mostLikelyCollision);
			numResolved += didSomething;
			collisions.RemoveIndex(deepestCollisionIndex, ListOption::RETAIN_ORDER);
			if (didSomething)
			{
				// Remove all duplicate collisions containing these two entities too, yo?
				for (int j = 0; j < collisions.Size(); ++j)
				{
					Collision & c = collisions[j];
					if ((c.one == mostLikelyCollision->one && c.two == mostLikelyCollision->two) || 
						(c.one == mostLikelyCollision->two && c.two == mostLikelyCollision->one))
					{
						collisions.RemoveIndex(j, ListOption::RETAIN_ORDER);
					}
				}
			}
			mostLikelyCollision = NULL;

			// Break inner loop? One collision per frame?
	//		break;
		}
		else 
		{
//			std::cout<<"\nNo likely collision detected. Could require some adjustment.";
			break;
		}

	}
		*/
	return numResolved;
}
