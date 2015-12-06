// Emil Hedemalm
// 2013-03-19

#include "../Entity/Entity.h"
#include "Collision.h"
#include "../PhysicsProperty.h"

///=================================================================================//
///  Plane-Sphere Intersection
///=================================================================================//	
bool PlaneSphereCollision(Entity * planeEntity, Entity * sphereEntity, Collision &data)
{
	assert(planeEntity->physics->shapeType == ShapeType::PLANE);
	Plane plane = *(Plane*)planeEntity->physics->shape;
	Matrix4f m = planeEntity->transformationMatrix;
	plane.Set3Points(m * plane.point1, m * plane.point2, m * plane.point3);
	/// Remember to multiply it by it's normal-matrix....
	Vector3f planeNormal = plane.normal;
	float distance = abs(plane.Distance(sphereEntity->worldPosition)) - sphereEntity->physics->physicalRadius;
	/// Collision?!
	if (distance > ZERO)
		return false;
	data.collisionNormal = plane.normal;
	data.results = NORMAL_ONLY;
	data.distanceIntoEachOther = distance;
	data.results |= DISTANCE_INTO;
	return true;
}