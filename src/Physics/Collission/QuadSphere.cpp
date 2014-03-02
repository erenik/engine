// Emil Hedemalm
// 2013-03-24

#include "../Entity/Entity.h"
#include "Collissions.h"
#include "../PhysicsProperty.h"

bool QuadSphereCollission(Entity * quadEntity, Entity * sphere, Collission &data){
	assert(quadEntity->physics->physicsShape == ShapeType::QUAD);
	Quad * pquad = (Quad*)quadEntity->physics->shape;
	Quad quad = *(Quad*)quadEntity->physics->shape;
	quad.Transform(quadEntity->transformationMatrix);
	return QuadSphereCollission(&quad, sphere, data);
}

bool QuadSphereCollission(Quad * quad, Entity * sphereEntity, Collission &data){
	float distance = abs(quad->Distance(sphereEntity->positionVector)) - sphereEntity->physics->physicalRadius;
	/// Collission?!
	if (distance > ZERO)
		return false;
	/// Check that the collissionpoint is within the plane too, by comparing the center 
	// of the sphere with the planes set up by the plane and it's normal
	const int PLANE_PLANES = 4;
	Plane planeFrustum[PLANE_PLANES];
	planeFrustum[0].Set3Points(quad->point1, quad->point2, quad->point1 + quad->normal);
	planeFrustum[1].Set3Points(quad->point2, quad->point3, quad->point2 + quad->normal);
	planeFrustum[2].Set3Points(quad->point3, quad->point4, quad->point3 + quad->normal);
	planeFrustum[3].Set3Points(quad->point4, quad->point1, quad->point4 + quad->normal);
	for (int i = 0; i < PLANE_PLANES; ++i){
		// If any of the plane's distance is negative it means we're outside the planeski.
		if (planeFrustum[i].Distance(sphereEntity->positionVector) > sphereEntity->physics->physicalRadius)
			return false;
	}
	data.collissionNormal = quad->normal;
	data.results = NORMAL_ONLY;
	data.distanceIntoEachOther = distance;
	data.results |= DISTANCE_INTO;
	return true;
}