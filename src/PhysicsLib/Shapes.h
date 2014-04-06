
#ifndef SHAPES_H
#define SHAPES_H

#include "Physics.h"
#include "List/List.h"

namespace ShapeType {
	enum ShapeTypes {
		NULL_TYPE,
		PLANE,	// An unlimited plane!
		TRIANGLE,	// A trilangle o-o
		QUAD,		// A quad o-o;
		CYLINDER,
		SPHERE,		// Uses the Entity's internal [radius] and [positionVector]
		MESH,
		/// Don't mess with the order here, only insert new ones below as the save/load relies on this list for the physics. :)
		CUBE,

		NUM_TYPES,
		DEFAULT_TYPE = SPHERE,
	};
	const char PLANE_STR [] = {"Plane"};
	const char TRIANGLE_STR [] = {"Triangle"};
	const char QUAD_STR [] = {"Quad"};
	const char SPHERE_STR [] = {"Sphere"};
	const char MESH_STR [] = {"Mesh"};
};


#ifndef PLANE_H
#define PLANE_H

struct Ray;
struct Plane;


/// A 3-dimensional plane
/// Defines a plane class, used for physical calculations/checks.
struct Plane {
	Plane() {
		Set3Points(Vector3f(-0.5f,0,-0.5f), Vector3f(0.5f,0,-0.5f),Vector3f(0.5f,0,0.5f));
	};
	/// Copy constructor
	Plane(const Plane &plane);

	/** Product with Matrix
		Postcondition: Returns the plane multiplied by the given matrix.
	*/
	Plane operator * (const Matrix4f matrix) const;

	/// Applies the given transform
	Plane Transform(Matrix4f transformationMatrix);
	/// Sets the three points that define define the plane in counter clockwise order.
	void Set3Points(Vector3f p1, Vector3f p2, Vector3f p3);
	/// Calculates and returns the distance to target point.
	float Distance(Vector3f point) const;

	/// Three points that define the plane. Assigned in counter clockwise order.
	Vector3f point1, point2, point3;
	/// Position in center of the three points.
	Vector3f position;
	/// Normal, calculated based on the 3 points.
	Vector3f normal;
	/// D - value of the plane, if specified as Ax + By + Cz + D = 0
	float D;
};

struct Triangle : public Plane {
	Triangle() : Plane() {
	};
	/// Positions isn't updated correctly with just the inherited plane-constructor!
	Triangle(const Triangle &tri);
	/// Sets the three points that define define the plane in counter clockwise order.
	void Set3Points(Vector3f p1, Vector3f p2, Vector3f p3);
};

struct Quad : public Plane {
private:
	/// Make Set3Points inaccessible!
	void Set3Points(Vector3f p1, Vector3f p2, Vector3f p3);
public:
	// Creates a default plane spanning -0.5 to 0.5, quadratically.
	Quad() {
		Set4Points(Vector3f(-0.5f,0,-0.5f),
			Vector3f(0.5f,0,-0.5f),
			Vector3f(0.5f,0,0.5f),
			Vector3f(-0.5f,0,0.5f));
	};
	/// Copy constructor
	Quad(const Quad &quad);

	/** Product with Matrix
		Postcondition: Returns the plane multiplied by the given matrix.
	*/
	Quad operator * (const Matrix4f matrix) const;
	/// Applies the given transform
	Quad Transform(const Matrix4f matrix);

	/** Sets three of the points that define the plane in counter clockwise order.
		the fourth point will be placed to mirror p2 along the line between p1 and p3.
	*/
	void Set4Points(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f p4);

	/// Fourth point or our quadly quad.
	Vector3f point4;
};
/// For polygons
struct Ngon : public Plane {
	Ngon();
	// Empty..
};

#endif

struct Cylinder {
	Cylinder() { radius = 1.0f; length = 1.0f; };
	Vector3f position;
	Vector3f rotation;
	float radius;
	float length;
};

struct Sphere {
#define DEFAULT_SECTIONS	8
	Sphere() { radius = 1.0f; sections = DEFAULT_SECTIONS; };
	Sphere(float radius, Vector3f position = Vector3f());
	float radius;
	Vector3f position;
	int sections;	// For if generating/rendering custom spheres or doing tests using it for some stupid reason..
};


struct Cube {
	Cube() { size = 1.0f; };
	Vector3f position;
	float size;
	Vector3f hitherTopLeft, hitherTopRight, hitherBottomLeft, hitherBottomRight,
		fartherTopLeft, fartherTopRight, fartherBottomLeft, fartherBottomRight;
};

struct Ray{
    /// Returns true if they intersect, and the distance (along the ray) if so.
    bool Intersect(Triangle & triangle, float * distance);

	Vector3f start; // or where it origins from
	Vector3f direction;
};

/// Note that this specification is for a line, but can be used for line-segments if caution is taken!
struct Line {
	Line(){};
	Line(Vector3f start, Vector3f stop)
		: start(start), stop(stop)
	{
		direction = stop - start;
	}
	Vector3f start;
	Vector3f stop;
	Vector3f direction;
};



/** Calculates if the provided plane and ray intersect.
	Returns 1 if an intersection occurs, and 0 if not.
	If a collission occurs, the point, normal and distance to collission is stored in the pointers.
*/
bool RayQuadIntersection(Ray & ray, Quad & quad);
/** Calculates if the provided plane and ray intersect.
	Returns 1 if an intersection occurs, and 0 if not.
	If a collission occurs, the point, normal and distance to collission is stored in the pointers.
*/
int RayPlaneIntersection(const Ray& ray, const Plane& plane, Vector3f * collisionPoint = NULL, Vector3f * collissionPointNormal = NULL, double * collissionDistance = NULL);


#endif
