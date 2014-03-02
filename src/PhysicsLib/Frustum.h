
#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "../MathLib.h"
#include "Physics.h"

/// Number used for converting angles to radians, just multiply it in.
#define ANG2RAD 3.14159265358979323846/180.0

#include "Shapes.h"

enum planeSideEnum{
	LEFT_PLANE = NULL, RIGHT_PLANE,
	BOTTOM_PLANE, TOP_PLANE,
	NEAR_PLANE, FAR_PLANE,
	FRUSTUM_PLANES
};

/** Class for handling a frustum, source taken in part from lighthouse3d.
http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-implementation/
*/
class Frustum {
private:
public:
    /// Default constructor
	Frustum();

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

	/// Sets camera internal values using default perspective arguments
	void SetCamInternals(float left = -1, float right = 1, float bottom = -1, float top = 1, float near = -1, float far = -10);
	/// Set camera orientation
	void SetCamPos(Vector3f position, Vector3f lookingAtVector, Vector3f upVector);
	/// Checks if a point is within the frustum
	int PointInFrustum(Vector3f &position) const;
	/// Checks if a sphere is within the frustum
	int SphereInFrustum(Vector3f &position, float radius) const;

private:
	/// Specifies if the camera internals have been set or not.
	bool initialized;
};

#endif
