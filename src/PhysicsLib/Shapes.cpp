#include "Shapes.h"
#include <cassert>

Plane::Plane()
{
	Set3Points(Vector3f(-0.5f,0,-0.5f), Vector3f(0.5f,0,-0.5f),Vector3f(0.5f,0,0.5f));
};

// Creates a plane, setting its 3 reference points in counter clockwise order.
Plane::Plane(Vector3f point1, Vector3f point2, Vector3f point3)
{
	Set3Points(point1, point2, point3);
}

/// Copy constructor
Plane::Plane(const Plane &plane)
{
	Set3Points(plane.point1, plane.point2, plane.point3);
}
/** Product with Matrix
	Postcondition: Returns the plane multiplied by the given matrix.
*/
Plane Plane::operator * (const Matrix4f matrix) const {
	Plane p;
	p.Set3Points(matrix * this->point1, matrix * this->point2, matrix * this->point3);
	return p;
}

/// Applies the given transform
Plane Plane::Transform(Matrix4f transformationMatrix){
	point1 = transformationMatrix * point1;
	point2 = transformationMatrix * point2;
	point3 = transformationMatrix * point3;
	Set3Points(point1, point2, point3);
	return Plane(*this);
}

/*
Assuming three points p0, p1, and p2 the coefficients A, B, C and D can be computed as follows:

Compute vectors v = p1 – p0, and u = p2 – p0;
	-	Compute n = v x u (cross product)
	-	Normalize n
	-	Assuming n = (xn,yn,zn) is the normalized normal vector then
		-	A = xn
		-	B = yn
		-	C = zn
To compute the value of D we just use the equation above, hence -D = Ax + By + Cz. Replacing (x,y,z) for a point in the plane (for instance p0), we get D = – n . p0 (dot product).
*/
void Plane::Set3Points(Vector3f p1, Vector3f p2, Vector3f p3){
	point1 = p1;
	point2 = p2;
	point3 = p3;
	position = p1;
	normal = Vector3f(p2 - p1).CrossProduct(Vector3f(p3 - p1));
	normal.Normalize();
	D = - normal.DotProduct(p1);
}

float Plane::Distance(Vector3f point) const {
	return (normal.DotProduct(point) + D);
}

/// Positions isn't updated correctly with just the plane-constructor!
Triangle::Triangle(const Triangle &tri)
: Plane((Plane)tri)
{
	position = (point1 + point2 + point3) * 0.33333f;
}

void Triangle::Set3Points(Vector3f p1, Vector3f p2, Vector3f p3){
	point1 = p1;
	point2 = p2;
	point3 = p3;
	position = (p1 + p2 + p3) * 0.33333f;
	normal = Vector3f(p2 - p1).CrossProduct(Vector3f(p3 - p1));
	normal.Normalize();
	D = - normal.DotProduct(p1);
}

/// Copy constructor
Quad::Quad(const Quad &quad){
	Set4Points(quad.point1, quad.point2, quad.point3, quad.point4);
}

/// Returns width x height. Assumes point1 is min and point 3 is max.
int Quad::ManhattanSize()
{
	return point3.x - point1.x + point3.y - point1.y;
};



/// Create a rectangular quad using min and max values.
void Quad::Set2Points(Vector3f min, Vector3f max)
{
	point1 = min;
	point3 = max;
	position = (point1 + point3) * 0.5f;
}

void Quad::Set4Points(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f p4){
	point1 = p1;
	point2 = p2;
	point3 = p3;
	point4 = p4;
	position = (p1 + p2 + p3 + p4) * 0.25f;
	normal = Vector3f(p2 - p1).CrossProduct(Vector3f(p3 - p1));
	normal.Normalize();
	D = - normal.DotProduct(p1);
}

/// Applies the given transform
Quad Quad::Transform(Matrix4f transformationMatrix){
	point1 = transformationMatrix * point1;
	point2 = transformationMatrix * point2;
	point3 = transformationMatrix * point3;
	point4 = transformationMatrix * point4;
	Set4Points(point1, point2, point3, point4);
	return Quad(*this);
}


Ngon::Ngon(){
	assert(false);
}

/// Sphere Initializer
Sphere::Sphere(float radius, Vector3f position /* = Vector3f()*/ )
: radius(radius), position(position), sections(DEFAULT_SECTIONS)
{
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
int RayPlaneIntersection(const Ray& ray, const Plane& plane, Vector3f * collissionPoint, Vector3f * collissionPointNormal, double * collissionDistance){
	float dotProduct = ray.direction.DotProduct(plane.normal); // Dot Product Between Plane Normal And Ray Direction
    float distanceToCollisionPoint;

    // Determine If Ray Parallel To Plane
    if (dotProduct < ZERO && dotProduct > -ZERO)
        return 0;

	// Calculate distance to the collission point
	distanceToCollisionPoint = (plane.normal.DotProduct(plane.position - ray.start))/dotProduct;

	// Check if the collision occurred in the opposite direction of the ray.
    if (distanceToCollisionPoint < -ZERO)
        return 0;

	if (collissionPointNormal)
		*collissionPointNormal = plane.normal;
	if (collissionDistance)
		*collissionDistance = distanceToCollisionPoint;
	if (collissionPoint)
		*collissionPoint = Vector3f(ray.direction * distanceToCollisionPoint + ray.start);
    return 1;
}

bool RayQuadIntersection(Ray & ray, Quad & quad){
	/// http://en.wikipedia.org/wiki/Line-plane_intersection
	///  ray.start - quad->point1 =
	/// (matrixOf) [ray.start - ray.direction * 1000, quad->point2 - quad->point1, quad->point3 - quad->point1]
	/// *  t/u/v
	Matrix3f m(ray.start - ray.direction * 1000, quad.point2 - quad.point1, quad.point3 - quad.point1);
	Vector3f tuv = m.InvertedCopy() * (ray.start - quad.point1);
	if (tuv.x > 0.0f && tuv.x < 1.0f){
		std::cout<<"\nIntersects quad 1, t: "<<tuv.x;
		if (tuv.y < 1.0f && tuv.y > 0.0f &&
			tuv.z < 1.0f && tuv.z > 0.0f &&
			tuv.y + tuv.z <= 1.0f){
			std::cout<<" and within triangle!";
			return true;
		}
	}

	/// Check the other triangle toooooo! :P
	m = Matrix3f(ray.start - ray.direction * 1000, quad.point4 - quad.point3, quad.point1 - quad.point3);
	tuv = m.InvertedCopy() * (ray.start - quad.point3);
	if (tuv.x > 0.0f && tuv.x < 1.0f){
		std::cout<<"\nIntersects quad 2, t: "<<tuv.x;
		if (tuv.y < 1.0f && tuv.y > 0.0f &&
			tuv.z < 1.0f && tuv.z > 0.0f &&
			tuv.y + tuv.z <= 1.0f){
			std::cout<<" and within triangle!";
			return true;
		}
	}
	return false;
}


Line::Line()
{
	Line(Vector3f(), Vector3f());
}

Line::Line(Vector3f start, Vector3f stop)
	: start(start), stop(stop)
{
	direction = stop - start;
	weight = 1;
}

float Line::Length()
{
	return (stop - start).Length();
}

// Calculates distance to point.
float Line::Distance(Vector3f point)
{
	// Ref: http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
	// Return minimum distance between line segment vw and point p
	// Length of this segment
	float lengthSquared = (stop - start).LengthSquared();
	// Start == stop case
	if (lengthSquared == 0.0) 
		return (point - start).Length();

	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line. 
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2
	const float t = (point - start).DotProduct(stop - start) / lengthSquared;
	// Beyond the 'start' end of the segment
	if (t < 0.0) 
		return (point - start).Length(); 
	// Beyond the 'stop' end of the segment
	else if (t > 1.0) 
		return (point - stop).Length();  

	// Projection falls on the segment
	const Vector3f projection = start + t * (stop - start);
	return (point - projection).Length();
}

/// Merges with other lines, taking weights into consideration for Y, but expanding X as necessary.
void Line::MergeYExpandX(Line & line)
{
	float totalWeight = weight + line.weight;
	float y1 = (start.y * weight + line.start.y * line.weight) / totalWeight;
	float y2 = (stop.y * weight + line.stop.y * line.weight) / totalWeight;
	start.y = y1;
	stop.y = y2;
	start.x = start.x < line.start.x ? start.x : line.start.x;
	stop.x = stop.x > line.stop.x ? stop.x : line.stop.x;
	weight = totalWeight;
}

/// Merges with other line, taking weights into consideration.
void Line::Merge(Line & line)
{
	float totalWeight = weight + line.weight;
	Vector3f newStart = (start * weight + line.start * line.weight) / totalWeight;
	Vector3f newStop = (stop * weight + line.stop * line.weight) / totalWeight;
	start = newStart;
	stop = newStop;
	weight = totalWeight;
}
