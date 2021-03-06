// Emil Hedemalm
// 2013-03-19

#include "../Entity/Entity.h"
#include "Collisions.h"
#include "../PhysicsProperty.h"

#include "Sphere.h"

bool TriangleSphereCollision(Entity* triEntity, Entity* sphereEntity, Collision &data, bool planesOnly){
	assert(triEntity->physics->shapeType == ShapeType::TRIANGLE);
	assert(triEntity->physics->shape);
	Triangle tri = *(Triangle*)(triEntity->physics->shape);
	tri.Transform(triEntity->transformationMatrix);

	if(TriangleSphereCollision(&tri, sphereEntity, data, planesOnly))
		return true;
	return false;
}

/// For checking between static shapes (most raw data types, pretty much)
bool TriangleSphereCollision(Triangle * triangle, Sphere * sphere, Collision &data, bool planeOnly)
{

	/// Remember to multiply it by it's normal-matrix....
	Vector3f planeNormal = triangle->normal;
	Vector3f & spherePosition = sphere->position;
	float & sphereRadius = sphere->radius;
	assert(triangle->normal.MaxPart());
	/// First early-out: distance to the triangle.
	float distance = triangle->Distance(sphere->position);
	/// Absolute value needed, since the distance can be negative!
	if (AbsoluteValue(distance) > sphere->radius)
		return false;
	
	// Check for another early out if invalid triangle
	if (triangle->normal.MaxPart() < ZERO){
		/** This means that the triangle pretty much lacks a normal,
			and is thusly probably not even a triangle at all!
		*/
		return false;
	}

	/// Now comes the initial test: is the sphere colliding with the plane of the triangle?
	bool colliding = true;
	const int PLANE_PLANES = 3;
	Plane planeFrustum[PLANE_PLANES];
	planeFrustum[0].Set3Points(triangle->point1, triangle->point2, triangle->point1 + triangle->normal);
	planeFrustum[1].Set3Points(triangle->point2, triangle->point3, triangle->point2 + triangle->normal);
	planeFrustum[2].Set3Points(triangle->point3, triangle->point1, triangle->point3 + triangle->normal);

	float planeCollisionRadius = sphere->radius;
	int result = Loc::INSIDE;
	for (int i = 0; i < PLANE_PLANES; ++i)
	{
		// If any of the plane's distance is negative it means we're outside the planeski.
		float distanceToPlane = planeFrustum[i].Distance(spherePosition);
		if (distanceToPlane > sphereRadius)
			return false;
		/// If larger than 0, the sphere's center is outside of the quad, and may intersect with its edges or corners, but not the plane itself.
		else if (distanceToPlane > 0)
			result = Loc::INTERSECT;
	}
	if (result == Loc::INSIDE)
	{
		data.collisionNormal = triangle->normal;
		data.results = NORMAL_ONLY;
		data.distanceIntoEachOther = distance - sphere->radius;
		data.results |= DISTANCE_INTO;
		return true;
	}
	if (planeOnly)
		return false;
	/// If not, proceed with testing the sphere against each of the triangle's edges.
	const int CORNERS = 3;
	Vector3f edgeStart[CORNERS], edgeStop[CORNERS], vertexToSphere[CORNERS];
	Vector3f temp, collissionPoint, edgeToSphere;
	edgeStart[0] = triangle->point1;
	edgeStart[1] = triangle->point2;
	edgeStart[2] = triangle->point3;
	edgeStop[0] = triangle->point2;
	edgeStop[1] = triangle->point3;
	edgeStop[2] = triangle->point1;

	/// For each vertex (easier to check, yo)
	for (int i = 0; i < CORNERS; ++i)
	{
		vertexToSphere[i] = spherePosition - edgeStart[i];
		float distanceVertToSphere = vertexToSphere[i].Length();
		if (distanceVertToSphere < sphereRadius)
		{
			data.collisionNormal = vertexToSphere[i].NormalizedCopy();
			data.distanceIntoEachOther = vertexToSphere[i].Length() - sphereRadius;
			data.results = NORMAL_ONLY | DISTANCE_INTO;
			return true;
		}
	}

	/// For each edge.
	for (int i = 0; i < CORNERS; ++i)
	{
		Vector3f edgeVector = edgeStop[i] - edgeStart[i];
		Vector3f vertexToSphereNormalized = vertexToSphere[i].NormalizedCopy();
		Vector3f edgeVectorNormalized = edgeVector.NormalizedCopy();

		/// Get dot products
		float vertexToSphereDotEdgeVector = vertexToSphereNormalized.DotProduct(edgeVectorNormalized);
		float distanceAlongEdge = vertexToSphereDotEdgeVector * vertexToSphere[i].Length();
		float edgeLength = edgeVector.Length();

		/// Skip this edge if the sphere's projected point is outside of the edge's length.
		if (distanceAlongEdge <= 0 || distanceAlongEdge >= edgeLength)
			continue;

		float relativeDistanceAlongEdge =  distanceAlongEdge / edgeLength;

		/// Projected point is somewhere along the edge, find it!
		Vector3f projectedPoint = edgeStart[i] + relativeDistanceAlongEdge * edgeVector;
		Vector3f projectedPointToSphere = spherePosition - projectedPoint;
		float distanceToSphere = projectedPointToSphere.Length();

	//	if (distanceToSphere < 100.0f)
	//		std::cout<<"\nDistance to sphere,: "<< distanceToSphere ;
		if (distanceToSphere > sphereRadius)
			continue;

		data.collisionNormal = projectedPointToSphere.NormalizedCopy();
		data.distanceIntoEachOther = distanceToSphere - sphereRadius;
		data.results = NORMAL_ONLY | DISTANCE_INTO;
		return true;
	}
	return false;
}

bool TriangleSphereCollision(Triangle * triangle, Entity* sphereEntity, Collision &data, bool planesOnly)
{
	static Sphere sphere; // = (Sphere*)sphereEntity->physics->shape;
	sphere.position = sphereEntity->worldPosition;
	sphere.radius = sphereEntity->physics->physicalRadius;
	return TriangleSphereCollision(triangle, &sphere, data, planesOnly);
/*
	/// Remember to multiply it by it's normal-matrix....
	Vector3f planeNormal = triangle->normal;
	/// First early-out: distance to the triangle.
	float distance = abs(triangle->Distance(sphereEntity->position));
	if (distance > sphereEntity->physics->physicalRadius)
		return false;

	// Check for another early out if invalid triangle
	if (triangle->normal.MaxPart() < ZERO){
		// This means that the triangle pretty much lacks a normal,
		//	and is thusly probably not even a triangle at all!

		return false;
	}

	/// Now comes the initial test: is the sphere colliding with the plane of the triangle?
	bool colliding = true;
	const int PLANE_PLANES = 3;
	Plane planeFrustum[PLANE_PLANES];
	planeFrustum[0].Set3Points(triangle->point1, triangle->point2, triangle->point1 + triangle->normal);
	planeFrustum[1].Set3Points(triangle->point2, triangle->point3, triangle->point2 + triangle->normal);
	planeFrustum[2].Set3Points(triangle->point3, triangle->point1, triangle->point3 + triangle->normal);
	for (int i = 0; i < PLANE_PLANES; ++i){
		// If any of the plane's distance is negative it means we're outside the planeski.
																// 0 will make it only the triangle and not a perimeter around it.
		if (planeFrustum[i].Distance(sphereEntity->position) > 0){ // sphereEntity->physics->physicalRadius)
			colliding = false;
			break;
		}
	}
	/// If it's colliding with the inside, then return the result!
	if (colliding){
		data.collisionNormal = triangle->normal;
		data.results = NORMAL_ONLY;
		data.distanceIntoEachOther = distance - sphereEntity->physics->physicalRadius;
		data.results |= DISTANCE_INTO;
		return true;
	}
	/// If not, proceed with testing the sphere against each of the triangle's edges.
	Vector3f edgeStart[3], edgeStop[3], vertexToSphere[3];
	Vector3f temp, collissionPoint, edgeToSphere;
	edgeStart[0] = triangle->point1;
	edgeStart[1] = triangle->point2;
	edgeStart[2] = triangle->point3;
	edgeStop[0] = triangle->point2;
	edgeStop[1] = triangle->point3;
	edgeStop[2] = triangle->point1;

	/// For each vertex (easier to check, yo)
	for (int i = 0; i < 3; ++i){
		vertexToSphere[i] = sphereEntity->position - edgeStart[i];
		float distanceVertToSphere = vertexToSphere[i].Length();
		if (distanceVertToSphere < sphereEntity->physics->physicalRadius){
			data.collisionNormal = vertexToSphere[i].NormalizedCopy();
			data.distanceIntoEachOther = vertexToSphere[i].Length() - sphereEntity->physics->physicalRadius;
			data.results = NORMAL_ONLY | DISTANCE_INTO;
			return true;
		}
	}

	/// For each edge.
	for (int i = 0; i < 3; ++i){
		Vector3f edgeVector = edgeStop[i] - edgeStart[i];
		Vector3f vertexToSphereNormalized = vertexToSphere[i].NormalizedCopy();
		Vector3f edgeVectorNormalized = edgeVector.NormalizedCopy();

		/// Get dot products
		float vertexToSphereDotEdgeVector = vertexToSphereNormalized.DotProduct(edgeVectorNormalized);
		float distanceAlongEdge = vertexToSphereDotEdgeVector * vertexToSphere[i].Length();
		float edgeLength = edgeVector.Length();

		/// Skip this edge if the sphere's projected point is outside of the edge's length.
		if (distanceAlongEdge <= 0 || distanceAlongEdge >= edgeLength)
			continue;

		float relativeDistanceAlongEdge =  distanceAlongEdge / edgeLength;

		/// Projected point is somewhere along the edge, find it!
		Vector3f projectedPoint = edgeStart[i] + relativeDistanceAlongEdge * edgeVector;
		Vector3f projectedPointToSphere = sphereEntity->position - projectedPoint;
		float distanceToSphere = projectedPointToSphere.Length();

		if (distanceToSphere > sphereEntity->physics->physicalRadius)
			continue;

		data.collisionNormal = projectedPointToSphere.NormalizedCopy();
		data.distanceIntoEachOther = distanceToSphere - sphereEntity->physics->physicalRadius;
		data.results = NORMAL_ONLY | DISTANCE_INTO;
		return true;
	}
	return false;
	*/

}
