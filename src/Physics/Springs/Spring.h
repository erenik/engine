/// Emil Hedemalm
/// 2013-10-27
/// A spring class meant to be attached between 2 unique entities.

#ifndef SPRING_H
#define SPRING_H

#include "MathLib.h"
class Entity;

/// Ref: http://en.wikipedia.org/wiki/Spring_%28device%29#Hooke.27s_law
class Spring {
public:
	Spring(Entity * one, ConstVec3fr pos);
	Spring(Entity * one, Entity * two);

	/// Returns the force to be exterted onto target entity. Must obviously be either of the entities that are set to the spring.
	Vector3f GetForce(Entity * subject);

	Entity * one, * two;
	Vector3f position;
	float equilibriumLength;
	/// A constant that depends on the spring's material and construction.
	float springConstant;

private:

};

#endif
