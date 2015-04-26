/// Emil Hedemalm
/// 2014-08-07 (though older)
/// Frustum class, i.e. a cube with various tilts on the sides. Used for camera culling among other things.

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "../MathLib.h"
// #include "Physics.h"

#include "Shapes.h"
#include "Plane.h"

enum planeSideEnum{
	LEFT_PLANE = 0, RIGHT_PLANE,
	BOTTOM_PLANE, TOP_PLANE,
	NEAR_PLANE, FAR_PLANE,
	FRUSTUM_PLANES
};

/** Class for handling a frustum, source taken in part from lighthouse3d.
http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-implementation/
*/
#ifdef USE_SSE
Align(16)
#endif
class Frustum {
private:
public:
    /// Default constructor
	Frustum();

	// Returns the center
	Vector3f GetCenter();

	/// Planes defining the frustum, with normals pointing inward.
	Plane frustumPlane[6];
	/// Frustum points
	Vector3f hitherTopLeft, hitherTopRight,
			hitherBottomLeft, hitherBottomRight,
			fartherTopLeft, fartherTopRight,
			fartherBottomLeft, fartherBottomRight;

	/// Plane specifiers
	float	left, right,
			bottom, top,
			nearPlaneDistance, farPlaneDistance; // Fuck aspect ratio and "angle"... aspectRatio, angle, tang;
	float farWidth, farHeight;

	/// 1 = Projection 3D, 2 = Orthogonal
	void SetProjection(int type);
	/// Sets camera internal values using default perspective arguments
	void SetCamInternals(float left = -1, float right = 1, float bottom = -1, float top = 1, float near = -1, float far = -10);
	/// Set camera orientation
	void SetCamPos(const Vector3f & position, const Vector3f & lookingAtVector, const Vector3f & upVector);
	/// Checks if a point is within the frustum
	int PointInFrustum(Vector3f &position) const;
	/// Checks if a sphere is within the frustum
	int SphereInFrustum(Vector3f &position, float radius) const;

private:
	/// Specifies if the camera internals have been set or not.
	bool initialized;
	/// 1 = Projection 3D, 2 = Orthogonal
	int projection;
};

#endif
