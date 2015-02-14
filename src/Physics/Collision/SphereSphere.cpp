// Emil Hedemalm
// 2013-03-19

///=================================================================================//
///  Sphere-Sphere Intersection
///=================================================================================//
	
#include "../Entity/Entity.h"
#include "Collision.h"
#include "../PhysicsProperty.h"

/// Checks and resolves a collission between target two spheres.
bool SphereSphereCollision(Entity * one, Entity * two, Collision &data){
	assert(one->physics->shapeType == ShapeType::SPHERE &&
		two->physics->shapeType == ShapeType::SPHERE);
	Vector3f distanceVector = one->position - two->position;
	/// Real distance between them, please....
	float distance = distanceVector.Length();
	distance -= one->physics->physicalRadius + two->physics->physicalRadius;
	distance = abs(distance);
	/// Fix for the above which seems to glitch at times.
	if (distance < ZERO)
		distance = ZERO;
	/// No collission if they're not into each other.
	if (distance < 0)
		return false;
	data.collisionNormal = distanceVector.NormalizedCopy();
	data.distanceIntoEachOther = distance;
	data.results |= DISTANCE_INTO;
	return true;
}
