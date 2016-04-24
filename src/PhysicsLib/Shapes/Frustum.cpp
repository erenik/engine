/// Emil Hedemalm
/// 2014-08-07 (though older)
/// Frustum class, i.e. a cube with various tilts on the sides. Used for camera culling among other things.

#include "Frustum.h"

#include "PhysicsLib.h"

Frustum::Frustum(){
	left = right = bottom = top = 0;
	nearPlaneDistance = 0;
	farPlaneDistance = 0;
	initialized = false;
	projection = 1;
}

// Returns the center
Vector3f Frustum::GetCenter()
{
	return (fartherBottomLeft + fartherTopRight + hitherBottomLeft + hitherTopRight) * 0.25f;
}

/// 1 = Projection 3D, 2 = Orthogonal
void Frustum::SetProjection(int type)
{
	projection = type;
}


/// Sets camera internal values using default perspective arguments
void Frustum::SetCamInternals(float i_left, float i_right,
							   float i_bottom, float i_top,
							   float i_nearPlaneDistance, float i_farPlaneDistance)
{
	left = i_left;
	right = i_right;
	bottom = i_bottom;
	top = i_top;
	nearPlaneDistance = i_nearPlaneDistance;
	farPlaneDistance = i_farPlaneDistance;

	// Calculate farPlane width and height. They should be relative to the ratio between the far and near plane distance and the near plane width.
	// Regular projection
	if (projection == 1)
	{
		farWidth = right * (farPlaneDistance / nearPlaneDistance);
		farHeight = top * (farPlaneDistance / nearPlaneDistance);
	}
	// Orthogonal
	else if (projection == 2)
	{
		farWidth = right;
		farHeight = top;
	}
}

void Frustum::SetCamPos(const Vector3f & position, const Vector3f & lookAt, const Vector3f & upVec)
{
	// First calculate the right vector using the looking at vector cross produced with the up vector.
	Vector3f rightVector = lookAt.CrossProduct(upVec).NormalizedCopy();
	Vector3f lookingAtVector = lookAt.NormalizedCopy();
	Vector3f upVector = upVec.NormalizedCopy();

	// Calculate center of the near and far plane to decrease amount of calculations oh-so-slightly
	Vector3f hitherCenter = position + lookingAtVector * nearPlaneDistance;
	Vector3f fartherCenter = position + lookingAtVector * farPlaneDistance;

	hitherTopLeft = hitherCenter + upVector * top  + rightVector * left;
	hitherTopRight = hitherCenter + upVector * top  + rightVector * right;
	hitherBottomLeft = hitherCenter + upVector * bottom  + rightVector * left;
	hitherBottomRight = hitherCenter + upVector * bottom  + rightVector * right;

	fartherTopLeft = fartherCenter + upVector * farHeight - rightVector * farWidth;
	fartherTopRight = fartherCenter + upVector * farHeight + rightVector * farWidth;
	fartherBottomLeft = fartherCenter - upVector * farHeight - rightVector * farWidth;
	fartherBottomRight = fartherCenter - upVector * farHeight + rightVector * farWidth;

	// Create the six planes by giving them three points each, in counter-clockwise order
	// are given in counter clockwise order
	frustumPlane[LEFT_PLANE].Set3Points(hitherTopLeft, hitherBottomLeft, fartherBottomLeft);
	frustumPlane[RIGHT_PLANE].Set3Points(hitherTopRight, fartherTopRight, fartherBottomRight);
	frustumPlane[BOTTOM_PLANE].Set3Points(fartherBottomRight, fartherBottomLeft, hitherBottomLeft);
	frustumPlane[TOP_PLANE].Set3Points(hitherTopLeft, fartherTopLeft, fartherTopRight);
	frustumPlane[NEAR_PLANE].Set3Points(hitherBottomLeft, hitherTopLeft, hitherTopRight);
	frustumPlane[FAR_PLANE].Set3Points(fartherBottomLeft, fartherBottomRight, fartherTopRight);
}

int Frustum::PointInFrustum(Vector3f &point) const {

	int result = Loc::INSIDE;

	for(int i=0; i < 6; i++) 
	{
		float distance = frustumPlane[i].Distance(point);
		if (distance < 0)
			return Loc::OUTSIDE;
	}
	return(result);
}

int Frustum::SphereInFrustum(Vector3f &position, float radius) const {

	float distance;
	int result = Loc::INSIDE;

	for(int i=0; i < 6; i++) {
		distance = frustumPlane[i].Distance(position);
		if (distance < -radius)
			return Loc::OUTSIDE;
		else if (distance < radius)
			result =  Loc::INTERSECT;
	}
	return(result);
}
