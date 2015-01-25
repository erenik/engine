/// Emil Hedemalm
/// 2014-08-06
/// Ray used for intersection tests. Based on a start position and direction.

#include "Ray.h"

#include "Triangle.h"
#include "Sphere.h"
#include "Plane.h"
#include "Quad.h"
#include <cassert>

/// Intersect with sphere.
bool Ray::Intersect(Sphere & sphere, float * distance)
{
	// Project the center of the sphere onto the ray.
	// http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
	float s = direction.DotProduct(this->start - sphere.position);
	float t = (start - sphere.position).LengthSquared();
	float insideSquareRoot = s * s - t + sphere.radius * sphere.radius;
	if (insideSquareRoot < 0)
		return false;

	// Should work since it's positive!
	float squareRoot = sqrt(insideSquareRoot);
	float left = -(direction.DotProduct(start - sphere.position));
	if (insideSquareRoot == 0)
	{
//		assert(false && "One solution exists");
		*distance = left + squareRoot;
	}
	else 
	{
//		assert(false && "2 solutions exist");
		float d1 = left + squareRoot;
		float d2 = left - squareRoot;
		/// Both solutions negative? then the intersection is behind the ray's starting point.
		if (d1 < 0 && d2 < 0)
			return false;
		if (d1 < 0)
		{
			*distance = d1;
		}
		else if (d2 < 0)
			*distance = d2;
		// Both solutions positive?
		else
			*distance = d1 < d2 ? d1 : d2;
	}
	return true;
}

/// Source: http://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
#define EPSILON 0.000001
bool Ray::Intersect(Triangle & triangle, float * distance)
                 /*
                   triangle_intersection( const Vec3   V0,  // Triangle vertices
                           const Vec3   V1,
                           const Vec3   V2,
                           const Vec3    O,  //Ray origin
                           const Vec3    D,  //Ray direction
                                 float* out )
                   */
{
    Vector3f V0 = triangle.point1;
    Vector3f V1 = triangle.point2;
    Vector3f V2 = triangle.point3;

    Vector3f e1, e2;  //Edge1, Edge2
    Vector3f P, Q, T;
    float det, inv_det, u, v;
    float t;

    //Find vectors for two edges sharing V0
    e1 = V1 - V0;
    e2 = V2 - V0;
    // Begin calculating determinant - also used to calculate u parameter
    P = direction.CrossProduct(e2);
    // if determinant is near zero, ray lies in plane of triangle
    det = e1.DotProduct(P);
    // NOT CULLING
    if(det > -EPSILON && det < EPSILON)
        return false;
    inv_det = 1.f / det;

    // calculate distance from V0 to ray origin
    T = start - V0;

    // Calculate u parameter and test bound
    u = T.DotProduct(P) * inv_det;
    // The intersection lies outside of the triangle
    if(u < 0.f || u > 1.f)
        return 0;

    // Prepare to test v parameter
    Q = T.CrossProduct(e1);

    // Calculate V parameter and test bound
    v = direction.DotProduct(Q) * inv_det;
    // The intersection lies outside of the triangle
    if(v < 0.f || u + v  > 1.f)
        return 0;

    t = e2.DotProduct(Q) * inv_det;

    if(t > EPSILON) { //ray intersection
        *distance = t;
        return 1;
    }

    // No hit, no win
    return 0;
}




/** Calculates if the provided plane and ray intersect.
	Returns 1 if an intersection occurs, and 0 if not.
	If a collission occurs, the point, normal and distance to collission is stored in the pointers.
*/

bool Ray::Intersect(const Plane& plane, Vector3f * collissionPoint, Vector3f * collissionPointNormal, double * collissionDistance)
{
	float dotProduct = direction.DotProduct(plane.normal); // Dot Product Between Plane Normal And Ray Direction
    float distanceToCollisionPoint;

    // Determine If Ray Parallel To Plane
    if (dotProduct < ZERO && dotProduct > -ZERO)
        return false;

	// Calculate distance to the collission point
	distanceToCollisionPoint = (plane.normal.DotProduct(plane.position - start))/dotProduct;

	// Check if the collision occurred in the opposite direction of the ray.
    if (distanceToCollisionPoint < -ZERO)
        return false;

	if (collissionPointNormal)
		*collissionPointNormal = plane.normal;
	if (collissionDistance)
		*collissionDistance = distanceToCollisionPoint;
	if (collissionPoint)
		*collissionPoint = Vector3f(direction * distanceToCollisionPoint + start);
    return true;
}

bool Ray::Intersect(Quad & quad, float * distance)
{
	/// http://en.wikipedia.org/wiki/Line-plane_intersection
	///  ray.start - quad->point1 =
	/// (matrixOf) [ray.start - ray.direction * 1000, quad->point2 - quad->point1, quad->point3 - quad->point1]
	/// *  t/u/v
	Matrix3f m(start - direction * 1000, quad.point2 - quad.point1, quad.point3 - quad.point1);
	Vector3f tuv = m.InvertedCopy() * (start - quad.point1);
	if (tuv[0] > 0.0f && tuv[0] < 1.0f){
		std::cout<<"\nIntersects quad 1, t: "<<tuv[0];
		if (tuv[1] < 1.0f && tuv[1] > 0.0f &&
			tuv[2] < 1.0f && tuv[2] > 0.0f &&
			tuv[1] + tuv[2] <= 1.0f){
			std::cout<<" and within triangle!";
			return true;
		}
	}

	/// Check the other triangle toooooo! :P
	m = Matrix3f(start - direction * 1000, quad.point4 - quad.point3, quad.point1 - quad.point3);
	tuv = m.InvertedCopy() * (start - quad.point3);
	if (tuv[0] > 0.0f && tuv[0] < 1.0f){
		std::cout<<"\nIntersects quad 2, t: "<<tuv[0];
		if (tuv[1] < 1.0f && tuv[1] > 0.0f &&
			tuv[2] < 1.0f && tuv[2] > 0.0f &&
			tuv[1] + tuv[2] <= 1.0f){
			std::cout<<" and within triangle!";
			return true;
		}
	}
	return false;
}



//
///** Calculates if the provided plane and ray intersect.
//	Returns 1 if an intersection occurs, and 0 if not.
//	If a collission occurs, the point, normal and distance to collission is stored in the pointers.
//*/
//
//int RayPlaneIntersection(const Ray& ray, const Plane& plane, Vector3f * collissionPoint, Vector3f * collissionPointNormal, double * collissionDistance){
//	float dotProduct = ray.direction.DotProduct(plane.normal); // Dot Product Between Plane Normal And Ray Direction
//    float distanceToCollisionPoint;
//
//    // Determine If Ray Parallel To Plane
//    if (dotProduct < ZERO && dotProduct > -ZERO)
//        return 0;
//
//	// Calculate distance to the collission point
//	distanceToCollisionPoint = (plane.normal.DotProduct(plane.position - ray.start))/dotProduct;
//
//	// Check if the collision occurred in the opposite direction of the ray.
//    if (distanceToCollisionPoint < -ZERO)
//        return 0;
//
//	if (collissionPointNormal)
//		*collissionPointNormal = plane.normal;
//	if (collissionDistance)
//		*collissionDistance = distanceToCollisionPoint;
//	if (collissionPoint)
//		*collissionPoint = Vector3f(ray.direction * distanceToCollisionPoint + ray.start);
//    return 1;
//}
//
//bool RayQuadIntersection(Ray & ray, Quad & quad){
//	/// http://en.wikipedia.org/wiki/Line-plane_intersection
//	///  ray.start - quad->point1 =
//	/// (matrixOf) [ray.start - ray.direction * 1000, quad->point2 - quad->point1, quad->point3 - quad->point1]
//	/// *  t/u/v
//	Matrix3f m(ray.start - ray.direction * 1000, quad.point2 - quad.point1, quad.point3 - quad.point1);
//	Vector3f tuv = m.InvertedCopy() * (ray.start - quad.point1);
//	if (tuv[0] > 0.0f && tuv[0] < 1.0f){
//		std::cout<<"\nIntersects quad 1, t: "<<tuv[0];
//		if (tuv[1] < 1.0f && tuv[1] > 0.0f &&
//			tuv[2] < 1.0f && tuv[2] > 0.0f &&
//			tuv[1] + tuv[2] <= 1.0f){
//			std::cout<<" and within triangle!";
//			return true;
//		}
//	}
//
//	/// Check the other triangle toooooo! :P
//	m = Matrix3f(ray.start - ray.direction * 1000, quad.point4 - quad.point3, quad.point1 - quad.point3);
//	tuv = m.InvertedCopy() * (ray.start - quad.point3);
//	if (tuv[0] > 0.0f && tuv[0] < 1.0f){
//		std::cout<<"\nIntersects quad 2, t: "<<tuv[0];
//		if (tuv[1] < 1.0f && tuv[1] > 0.0f &&
//			tuv[2] < 1.0f && tuv[2] > 0.0f &&
//			tuv[1] + tuv[2] <= 1.0f){
//			std::cout<<" and within triangle!";
//			return true;
//		}
//	}
//	return false;
//}
//
