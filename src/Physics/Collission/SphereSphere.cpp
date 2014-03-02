// Emil Hedemalm
// 2013-03-19

///=================================================================================//
///  Sphere-Sphere Intersection
///=================================================================================//
	
#include "../Entity/Entity.h"
#include "Collission.h"
#include "../PhysicsProperty.h"

/// Checks and resolves a collission between target two spheres.
bool SphereSphereCollission(Entity * one, Entity * two, Collission &data){
	assert(one->physics->physicsShape == ShapeType::SPHERE &&
		two->physics->physicsShape == ShapeType::SPHERE);
	Vector3f distanceVector = one->positionVector - two->positionVector;
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
	data.collissionNormal = distanceVector.Normalize();
	data.distanceIntoEachOther = distance;
	data.results |= DISTANCE_INTO;
	return true;
}
