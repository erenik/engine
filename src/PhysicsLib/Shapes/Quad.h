/// Emil Hedemalm
/// 2014-08-06
/// Basic shapes.

#include "Plane.h"

/// Assumes following point-order: lower-left (min), lower-right, upper-right (max), upper-left.
class Quad : public Plane {
private:
	/// Make Set3Points inaccessible!
	void Set3Points(const Vector3f & p1, const Vector3f & p2, const Vector3f & p3);
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

	/// Returns width x height. Assumes point1 is min and point 3 is max.
	int ManhattanSize(); 

	/** Product with Matrix
		Postcondition: Returns the plane multiplied by the given matrix.
	*/
	Quad operator * (const Matrix4f matrix) const;
	/// Applies the given transform
	Quad Transform(const Matrix4f matrix);

	/// Create a rectangular quad using min and max values.
	void Set2Points(const Vector3f & min, const Vector3f & max);
	/** Sets all of the points that define the plane in counter clockwise order.
		the fourth point will be placed to mirror p2 along the line between p1 and p3.
	*/
	void Set4Points(const Vector3f & p1, const Vector3f & p2, const Vector3f & p3, const Vector3f & p4);

	/// Fourth point or our quadly quad.
	Vector3f point4;
};
