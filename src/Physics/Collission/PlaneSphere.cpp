// Emil Hedemalm
// 2013-03-19

#include "../Entity/Entity.h"
#include "Collission.h"
#include "../PhysicsProperty.h"

///=================================================================================//
///  Plane-Sphere Intersection
///=================================================================================//	
bool PlaneSphereCollission(Entity * planeEntity, Entity * sphereEntity, Collission &data){
	assert(planeEntity->physics->physicsShape == ShapeType::PLANE);
	Plane plane = *(Plane*)planeEntity->physics->shape;
	Matrix4f m = planeEntity->transformationMatrix;
	plane.Set3Points(m * plane.point1, m * plane.point2, m * plane.point3);
	/// Remember to multiply it by it's normal-matrix....
	Vector3f planeNormal = plane.normal;
	float distance = abs(plane.Distance(sphereEntity->position)) - sphereEntity->physics->physicalRadius;
	/// Collission?!
	if (distance > ZERO)
		return false;
	data.collissionNormal = plane.normal;
	data.results = NORMAL_ONLY;
	data.distanceIntoEachOther = distance;
	data.results |= DISTANCE_INTO;
	return true;
}