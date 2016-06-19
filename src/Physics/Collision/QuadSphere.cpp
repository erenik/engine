// Emil Hedemalm
// 2013-03-24

#include "../Entity/Entity.h"
#include "Collisions.h"
#include "../PhysicsProperty.h"
#include "PhysicsLib/Shapes/Quad.h"
#include "String/StringUtil.h"
#include "File/LogFile.h"

extern int debug;

bool QuadSphereCollision(Entity * quadEntity, Entity * sphere, Collision &data, bool planesOnly)
{
	assert(quadEntity->physics->shapeType == ShapeType::QUAD);
	Quad * pquad = (Quad*)quadEntity->physics->shape;
	Quad quad = *(Quad*)quadEntity->physics->shape;
	quad.Transform(quadEntity->transformationMatrix);
	return QuadSphereCollision(&quad, sphere, data, planesOnly);
}

bool QuadSphereCollision(Quad * quad, Entity * sphereEntity, Collision &data, bool planeOnly)
{
	if (debug == 5)
		std::cout<<"\nSphere entity position "<<sphereEntity->worldPosition;
	float radius = sphereEntity->physics->physicalRadius;
	float distance = quad->Distance(sphereEntity->worldPosition);
	Vector3f spherePosition = sphereEntity->worldPosition;
	/// Collision?!
	if (AbsoluteValue(distance) > radius)
		return false;

 //   LogPhysics("Distance: "+String(distance)+" Sphere pos"+VectorString(sphereEntity->position )+" Quad n"+VectorString(quad->normal)+" p" + VectorString(quad->position), INFO);

	/// Check that the collissionpoint is within the plane too, by comparing the center
	// of the sphere with the planes set up by the plane and it's normal
	const int PLANE_PLANES = 4;
	Plane planeFrustum[PLANE_PLANES];
	planeFrustum[0].Set3Points(quad->point1, quad->point2, quad->point1 + quad->normal);
	planeFrustum[1].Set3Points(quad->point2, quad->point3, quad->point2 + quad->normal);
	planeFrustum[2].Set3Points(quad->point3, quad->point4, quad->point3 + quad->normal);
	planeFrustum[3].Set3Points(quad->point4, quad->point1, quad->point4 + quad->normal);
	int result = Loc::INSIDE;
	for (int i = 0; i < PLANE_PLANES; ++i)
	{
		// If any of the plane's distance is negative it means we're outside the planeski.
		float distanceToPlane = planeFrustum[i].Distance(sphereEntity->worldPosition);
		if (distanceToPlane > radius)
			return false;
		/// If larger than 0, the sphere's center is outside of the quad, and may intersect with its edges or corners, but not the plane itself.
		else if (distanceToPlane > 0)
			result = Loc::INTERSECT;
	}
	if (result == Loc::INSIDE)
	{
		data.collisionNormal = quad->normal;
		data.results = NORMAL_ONLY;
		data.distanceIntoEachOther = distance - radius;
		data.results |= DISTANCE_INTO;
		return true;
	}
	// Add handling for the edges and corners?
	if (planeOnly)
		return false;

	/// If not, proceed with testing the sphere against each of the triangle's edges.
	const int CORNERS = 4;
	Vector3f edgeStart[CORNERS], edgeStop[CORNERS], vertexToSphere[CORNERS];
	Vector3f temp, collissionPoint, edgeToSphere;
	edgeStart[0] = quad->point1;
	edgeStart[1] = quad->point2;
	edgeStart[2] = quad->point3;
	edgeStart[3] = quad->point4;
	edgeStop[0] = quad->point2;
	edgeStop[1] = quad->point3;
	edgeStop[2] = quad->point4;
	edgeStop[3] = quad->point1;

	/// For each vertex (easier to check, yo)
	for (int i = 0; i < CORNERS; ++i)
	{
		vertexToSphere[i] = spherePosition - edgeStart[i];
		float distanceVertToSphere = vertexToSphere[i].Length();
		if (distanceVertToSphere < radius)
		{
			data.collisionNormal = vertexToSphere[i].NormalizedCopy();
			data.distanceIntoEachOther = vertexToSphere[i].Length() - radius;
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
		if (distanceToSphere > radius)
			continue;

		data.collisionNormal = projectedPointToSphere.NormalizedCopy();
		data.distanceIntoEachOther = distanceToSphere - radius;
		data.results = NORMAL_ONLY | DISTANCE_INTO;
		return true;
	}
	return false;
}
