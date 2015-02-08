/// Emil Hedemalm
/// 2013-10-01

#include "Collisions.h"
#include "Collision.h"
#include "Model/Geometry.h"

#define EPSILON 0.000001

/// Geometrical collission tests.
bool EdgeFaceCollision(Edge * edge, Face * face, Collision &data){

	// Project a ray from the edge to the face. Check it's distance until intersection.
	/// Copied from Ray::Intersect
     /*
       triangle_intersection( const Vec3   V0,  // Triangle vertices
               const Vec3   V1,
               const Vec3   V2,
               const Vec3    O,  //Ray origin
               const Vec3    D,  //Ray direction
                     float* out )
       */
	assert(edge->start && edge->stop);
	Vector3f start = edge->start->position;
	Vector3f stop = edge->stop->position;
	Vector3f direction = stop - start;
	
	float edgeLength = (stop - start).Length();
	assert(edgeLength > 0);

	assert(face->triangleList.Size());
	/// Check all triangles in the face!
	for (int i = 0; i < face->triangleList.Size(); ++i){

		Triangle & triangle = *face->triangleList[i];

		Vector3f V0 = triangle.point1;
		Vector3f V1 = triangle.point2;
		Vector3f V2 = triangle.point3;

		Vector3f e1, e2;  //Edge1, Edge2
		Vector3f P, Q, T;
		float det, inv_det, u, v;
		float t;

		// Find vectors for two edges sharing V0
		e1 = V1 - V0;
		e2 = V2 - V0;
		// Begin calculating determinant - also used to calculate u parameter
		P = direction.CrossProduct(e2);
		// if determinant is near zero, ray lies in plane of triangle
		det = e1.DotProduct(P);
		// NOT CULLING
		if(det > -EPSILON && det < EPSILON)
			continue;
		inv_det = 1.f / det;

		// calculate distance from V0 to ray origin
		T = start - V0;

		// Calculate u parameter and test bound
		u = T.DotProduct(P) * inv_det;
		// The intersection lies outside of the triangle
		if(u < 0.f || u > 1.f)
			continue;

		// Prepare to test v parameter
		Q = T.CrossProduct(e1);

		// Calculate V parameter and test bound
		v = direction.DotProduct(Q) * inv_det;
		// The intersection lies outside of the triangle
		if(v < 0.f || u + v  > 1.f)
			continue;

		t = e2.DotProduct(Q) * inv_det;

		if(t > EPSILON) { //ray intersection
			/// Check the length of the edge!
			if (t > edgeLength)
				continue;
			/// T is the distance along the ray, and not the distance into each other.. probably.
		//	data.distanceIntoEachOther = t;
			data.distanceIntoEachOther = EPSILON;
			data.collissionPoint = start + direction.NormalizedCopy() * t;
			if (data.results & PRELIMINARY_COLLISSION_NORMAL)
			{
				Vector3f tmpNormal = (edge->faceList[0]->normal + edge->faceList[1]->normal).NormalizedCopy();
				float dot = data.preliminaryCollisionNormal.DotProduct(tmpNormal);
				if (dot < 0.5f){
					std::cout<<"\nWARNING: Collision normal and preliminary collission-normal do not coincide!";
				}
				data.collisionNormal = data.preliminaryCollisionNormal;
			}
			else {
				/// Use the normal of this edge as collission normal, jaow? = average of the two face's normals beside it.
				assert(edge->faceList.Size() >= 2);
				if (edge->faceList.Size() < 2)
					data.collisionNormal = Vector3f(0,1,0);
				else 
					data.collisionNormal = (edge->faceList[0]->normal + edge->faceList[1]->normal).NormalizedCopy();
			}
			return true;
		}
	}

    // No hit, no win
    return false;
}