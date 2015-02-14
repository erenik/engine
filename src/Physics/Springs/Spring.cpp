/// Emil Hedemalm
/// 2013-10-27
/// A spring class meant to be attached between 2 unique entities.

#include "Spring.h"
#include "Entity/Entity.h"

/// Ref: http://en.wikipedia.org/wiki/Spring_%28device%29#Hooke.27s_law
Spring::Spring(Entity * one, ConstVec3fr pos)
: one(one), two(NULL), position(pos)
{
	equilibriumLength = 1.0f;
	springConstant = 1.0f;
}

Spring::Spring(Entity * one, Entity * two)
: one(one), two(two)
{
	equilibriumLength = 1.0f;
	springConstant = 1.0f;
};

/// Returns the force to be exterted onto target entity. Must obviously be either of the entities that are set to the spring.
Vector3f Spring::GetForce(Entity * subject)
{
	Vector3f toOther;
	if (two)
	{
		assert(subject == one || subject == two);
		Entity * other = one == subject? two : one;
		toOther = other->position - subject->position;
	}
	else 
	{
		toOther = position - one->position;
	}
	float distance = toOther.Length();
	float distDiff = distance - equilibriumLength;
	Vector3f forceVector = distDiff * toOther.NormalizedCopy() * springConstant;
	return forceVector;
};
