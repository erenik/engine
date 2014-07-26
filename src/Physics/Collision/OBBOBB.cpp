/// Emil Hedemalm
/// 2013-09-19
/// Oriented bounding box intersection test.

#include "Collisions.h"
#include "PhysicsLib/OrientedBoundingBox.h"
#include "Physics/PhysicsManager.h"

/// Checks intersection along an axis... yup.
bool IntersectOnAxis(OrientedBoundingBox * obb1, OrientedBoundingBox * obb2, Vector3f axis, float & intersectionDepth){
	
	float oneMax, oneMin, twoMax, twoMin;
	float tmp;

	oneMax = oneMin = axis.DotProduct(obb1->corners[0]);
	for (int i = 1; i < OrientedBoundingBox::CORNERS; ++i){
		tmp = axis.DotProduct(obb1->corners[i]);
		if (tmp < oneMin)
			oneMin = tmp;
		else if (tmp > oneMax)
			oneMax = tmp;
	}

	twoMax = twoMin = axis.DotProduct(obb2->corners[0]);
		for (int i = 1; i < OrientedBoundingBox::CORNERS; ++i){
		tmp = axis.DotProduct(obb2->corners[i]);
		if (tmp < twoMin)
			twoMin = tmp;
		else if (tmp > twoMax)
			twoMax = tmp;
	}
	
	/// Check if they intersect or not.
	float tmpDepth = oneMax - twoMin;
	intersectionDepth = tmpDepth;
	if (intersectionDepth < 0)
		return false;


	tmpDepth = twoMax - oneMin;
	if (tmpDepth < intersectionDepth){
		intersectionDepth = tmpDepth;
	}
	else {
		/// CPPPP
		// std::cout<<"\nHej. Jag var fel förut.";
	}
	if (intersectionDepth < 0)
		return false;
	/// Intersection depth should now be set appropriately.. ish.
	return true;
}




/// Along axis...
List<Vector3f> FetchExtremePoints(OrientedBoundingBox * obb, Vector3f axis){
	float extreme = 0;
	float clipRange = obb->localSize.MaxPart() * 0.01f;
	List<Vector3f> points;
	for (int i = 0; i < OrientedBoundingBox::CORNERS; ++i){
		Vector3f c = obb->corners[i];
		float p = c.DotProduct(axis);
		if (points.Size()){
			/// Greater than.
			if (p > extreme + clipRange){
				points.Clear();
				points.Add(c);
				extreme = p;
			}
			/// Less than
			else if (p < extreme - clipRange){
				continue;
			}
			/// Within the interval
			else {
				points.Add(c);
			}
		}
		else {
			extreme = p;
			points.Add(c);
		}
	}
	return points;
};

/// Reference: Real-time collission detection page 146-147
List<Vector3f> ClosestPointsTwoLines(Line one, Line two){
	List<Vector3f> points;
	float a = one.direction.DotProduct(one.direction);
	float e = two.direction.DotProduct(two.direction);
	float b = two.direction.DotProduct(one.direction);
	float d = a*e - b*b;
	assert(d >= 0);
	if (d == 0){
		std::cout<<"\nLines are parralel! Must be handled separately! D:";
		return points;
	}
	Vector3f r = one.start - two.start;
	float c = one.direction.DotProduct(r);
	float f = two.direction.DotProduct(r);
	float s = (b * f - c * e ) / d;
	float t = (a * f - b * c) / d;

	Vector3f point1 = one.start + one.direction * s;
	Vector3f point2 = two.start + two.direction * t;
	points.Add(point1);
	points.Add(point2);
	return points;
}

/** Modifies the point so that it is within the face, 
	but keeps it's relative position (as far as possible).
	The point will be moved toward the reference point until it is within the face bounds.
	DO NOTE that no check is performed to ensure that the reference point is within the face.
*/
void EnsurePointWithinFaceBoundary(Vector3f & point, const Face & face, const Vector3f & referencePoint){
	float distance, maxDistance;
	Vector3f boundaryNormal;
	int numPlanesPointIsOutside = 0;
	do {
		maxDistance = 0;
		for (int i = 0; i < 4; ++i){
			/// Construct 4 faces aligned with the normal to get the bounds.
			Quad q;
			Vertex * v1 = face.vertexList[i], 
				* v2;
			if (i < 3)
				v2 = face.vertexList[i+1];
			else
				v2 = face.vertexList[0];
			Vector3f p1 = v1->position, 
					 p2 = v2->position;
			q.Set4Points(p1,p2,p2+face.normal,p1+face.normal);

			distance = q.Distance(point);
			if (distance < maxDistance)
			{
				maxDistance = distance;
				boundaryNormal = q.normal;
			}
			if (distance < 0){
				++numPlanesPointIsOutside;
				// std::cout<<"\nPoint "<<point<<"is outside quad with normal: "<<q.normal;
			}
		}
		/// If maxDistance is 0, it means the point is already inside the plane! All is good!
		if (maxDistance >= -0.001f)
			return;
		/// Now shove the point inside, using the given boundary normal, referencePoint and distance outside the boundary.
		Vector3f referenceToPoint = point - referencePoint;
		Vector3f refToPointNormalized = referenceToPoint.NormalizedCopy();
		float refToPointDotBoundaryNormal = refToPointNormalized.DotProduct(boundaryNormal);
		assert(refToPointDotBoundaryNormal != 0);
		if (refToPointDotBoundaryNormal == 0)
			return;
		Vector3f vectorToShove = referenceToPoint;
		vectorToShove.Normalize();
		vectorToShove *= AbsoluteValue(maxDistance) * 1 / refToPointDotBoundaryNormal;
		point += vectorToShove;
		/// Repeat until the maxDistance is nullified (within the face).
	} while (maxDistance < -0.001f);
}

/// For them OBBs.
bool OBBOBBCollision(OrientedBoundingBox * obb1, OrientedBoundingBox * obb2, Collision & data){

    /// Data-data!
    Sphere sphere;
    Triangle tri;

    Collision deepestCollision;
    deepestCollision.distanceIntoEachOther = -1000000.0f;
    Collision tempData;
    bool collissionImminent = false;

	/// Perform separating axis checks to see if there's any chance that they intersect at all.
	/// TODO: magical stuff

#define AXES			15
#define AXES_TO_TEST	15

	Vector3f axis[AXES];
	axis[0] = obb1->localUp;
	axis[1] = obb1->localForward;
	axis[2] = obb1->localRight;

	axis[3] = obb2->localUp;
	axis[4] = obb2->localForward;
	axis[5] = obb2->localRight;

	axis[6] = obb1->localUp.CrossProduct(obb2->localUp);
	axis[7] = obb1->localUp.CrossProduct(obb2->localForward);
	axis[8] = obb1->localUp.CrossProduct(obb2->localRight);

	axis[9] = obb1->localForward.CrossProduct(obb2->localUp);
	axis[10] = obb1->localForward.CrossProduct(obb2->localForward);
	axis[11] = obb1->localForward.CrossProduct(obb2->localRight);

	axis[12] = obb1->localRight.CrossProduct(obb2->localUp);
	axis[13] = obb1->localRight.CrossProduct(obb2->localForward);
	axis[14] = obb1->localRight.CrossProduct(obb2->localRight);

	float intersectionDepth = 100000.f;
	float shallowestIntersection = 100000.f;
	Vector3f intersectionAxis;
	for (int i = 0; i < AXES_TO_TEST; ++i){
		
		
		/// Can be good to.. y'know.. normalize the axes if you want the intersection depth to make any SENSE.
		axis[i].Normalize();
		/// Make sure the axis has decent numbers before continuing (low numbers = parralell axles have been crossed)
		if (axis[i].MaxPart() < 0.01f)
			continue;
		if (!IntersectOnAxis(obb1, obb2, axis[i], intersectionDepth))
			return false;
		if (intersectionDepth < shallowestIntersection){
			shallowestIntersection = intersectionDepth;
			intersectionAxis = axis[i];
			intersectionDepth = 100000.f;
		}
	}
	data.separatingAxes.Clear();
	for (int i = 0; i < AXES_TO_TEST; ++i)
		data.separatingAxes.Add(axis[i]);
		
	/// Save the shallowest intersection-axis as the probable collission normal!
	intersectionAxis.Normalize();	
	
	/// Make the intersection axis go from obb1 to obb2!
	Vector3f oneToTwo = obb2->position - obb1->position;
	float dot = oneToTwo.DotProduct(intersectionAxis);
	if (dot < 0){
		intersectionAxis *= -1.0f;
	}
	/// Save it as stuff
	if (intersectionAxis.MaxPart() > 0){
		data.preliminaryCollisionNormal = intersectionAxis;
		data.results |= PRELIMINARY_COLLISSION_NORMAL;
//		std::cout<<"\nEstimated collission-normal: "<<data.collisionNormal;
		tempData.results |= PRELIMINARY_COLLISSION_NORMAL;
		tempData.preliminaryCollisionNormal = data.preliminaryCollisionNormal;
	}


	/// Get the most extreme points of both A and B projected onto the intersecting axis.
	List<Vector3f> pointsOne, pointsTwo;
	/// Fetch extreme points going toward obb2
	pointsOne = FetchExtremePoints(obb1, intersectionAxis);
	/// And extreme points going to obb1
	pointsTwo = FetchExtremePoints(obb2, -intersectionAxis);
/*
	std::cout<<"\nPoints on object one ("<<obb1->position<<"): "<<pointsOne.Size();
	std::cout<<"\nPoints on object two ("<<obb2->position<<"): "<<pointsTwo.Size();
*/
	data.collisionNormal = data.preliminaryCollisionNormal;
	data.pointsOne = pointsOne;
	data.pointsTwo = pointsTwo;

	/// Vert-Vert
	if (pointsOne.Size() == 1 && pointsTwo.Size() == 1){
		data.collissionPoint = (pointsOne[0] + pointsTwo[0]) * 0.5f; 
	}
	/// Vert-face
	else if (pointsOne.Size() == 1){
		data.collissionPoint = pointsOne[0];
	}
	else if (pointsTwo.Size() == 1){
		data.collissionPoint = pointsTwo[0];
	}
	/// Edge-Edge
	else if (pointsOne.Size() == 2 && pointsTwo.Size() == 2){
//		std::cout<<"\nEdge-Edge Collision!";
		/// Do a ray-face intersection, or just approximate it..?
		Line lineOne(pointsOne[0], pointsOne[1]), lineTwo(pointsTwo[0], pointsTwo[1]);
		List<Vector3f> closestPoints = ClosestPointsTwoLines(lineOne, lineTwo);
		data.collissionPoint = (closestPoints[0] + closestPoints[1]) * 0.5f;
	}
	// Edge-Face intersectionnnnnnn
	else if (pointsOne.Size() == 2 || pointsTwo.Size() == 2){
		List<Vector3f> * edgePoints;
		/// Check which was the edge.
		if (pointsOne.Size() == 2)
			edgePoints = &pointsOne;
		else 
			edgePoints = &pointsTwo;

		/// Ensure that all points are within the face. (consider an edge not fully intersecting with the face, only half of it)
		
		/// Fetch the face that is aligned with the collission-normal.
		List<Vector3f> * facePoints = NULL;
		if (pointsOne.Size() == 4)
			facePoints = &pointsOne;
		else 
			facePoints = &pointsTwo;

		/// Find a face that is aligned with the collission-normal.
		Face * targetFace = NULL;
		for (int i = 0; i < OrientedBoundingBox::FACES; ++i){
			Face * f = obb1->faceList[i];
			if (f->normal.DotProduct(data.collisionNormal) >= 0.9f)
				targetFace = f;
			f = obb2->faceList[i];
			if (f->normal.DotProduct(data.collisionNormal) >= 0.9f)
				targetFace = f;
		}
		assert(targetFace);
		if (!targetFace)
			return false;
		Vector3f point1 = (*edgePoints)[0], 
				 point2 = (*edgePoints)[1];
		EnsurePointWithinFaceBoundary(point1, *targetFace, point2);
		EnsurePointWithinFaceBoundary(point2, *targetFace, point1);
		/// Lay the collission-point right in the middle!
		data.collissionPoint = (point1 + point2) * 0.5f;
	//	std::cout<<"\nEDGE_Face collission, yaow. o-o;";
	}
	/// Lol..`?
	else if (pointsOne.Size() >= 3 && pointsTwo.Size() >= 3){
	//	std::cout<<"\n OMG! Face-Face Collisshuuuuiiion! DAYOU";

		/// Find relevant faces that is aligned with the collission-normal.
		Face * targetFace1 = NULL, * targetFace2 = NULL;
		for (int i = 0; i < OrientedBoundingBox::FACES; ++i){
			Face * f = obb1->faceList[i];
			if (f->normal.DotProduct(data.collisionNormal) >= 0.9f)
				targetFace1 = f;
			f = obb2->faceList[i];
			float dotProduct = f->normal.DotProduct(data.collisionNormal);
			if (dotProduct <= -0.9f)
				targetFace2 = f;
		}

		assert(targetFace1 && targetFace2);
		targetFace1->ComputePosition();
		targetFace2->ComputePosition();
		/// ...
		/// Set the collission-point to be the average of all interacting vertices, 
		/// as this should be the points in the middle at which both faces overlap.
		data.collissionPoint = Vector3f();
		for (int i = 0; i < 4; ++i){
			Vector3f point1 = pointsOne[i];
			Vector3f point2 = pointsTwo[i];
			/// Ensure all points are within both face's boundaries before continuing.... 
			/// Use the center as reference..?
			EnsurePointWithinFaceBoundary(point1, *targetFace2,  targetFace2->position);
			EnsurePointWithinFaceBoundary(point2, *targetFace1,  targetFace1->position);

			data.collissionPoint += point1;
			data.collissionPoint += point2;
		}
		data.collissionPoint *= 0.125f;
	}
	/// Misc others.
	else {
		std::cout<<"\nMisc. other collissionssss! D:";
		assert(false);
		data.collissionPoint = obb1->position;
	}
	data.distanceIntoEachOther = shallowestIntersection;
	data.results |= DISTANCE_INTO;
	// Ignore below results for now.
	return true;

	/// Seprating Axises tested! Collision is probable.


    /// Check vertexes of obb1 vs. faces of obb2.
    /// Set it's radius to be a fraction (0.0001f?) of the related bounding box.
    sphere.radius = obb1->localSize.MaxPart() * 0.0001f;
    for (int i = 0; i < 8; ++i){
        /// Set the vertex to be a very small sphere!
        sphere.position = obb1->corners[i];

		/// Check if it's inside at all first!
		if (!obb2->IsInside(sphere))
			continue;

        /// Do proper collission check.
		bool collided = obb2->Collide(sphere, tempData);
        if (collided){
            tempData.distanceIntoEachOther = AbsoluteValue(tempData.distanceIntoEachOther);
            std::cout<<"\nDistance into each other: "<<tempData.distanceIntoEachOther<<" deepest: "<<deepestCollision.distanceIntoEachOther;
            if (tempData.distanceIntoEachOther > deepestCollision.distanceIntoEachOther){
                deepestCollision = tempData;
                std::cout<<"\nSetting deepest collission.";
            }
            std::cout<<"\nCollision Imminent!";
            collissionImminent = true;
        }
    }

	if (data.results & PRELIMINARY_COLLISSION_NORMAL){
		tempData.results |= PRELIMINARY_COLLISSION_NORMAL;
		tempData.preliminaryCollisionNormal = data.preliminaryCollisionNormal;
	}

    /// Continue, reverse relationship.
    /// Check vertexes of obb2 vs. faces of obb1.
    /// Set it's radius to be a fraction (0.0001f?) of the related bounding box.
	assert(obb2->localSize.MaxPart() > 0);
    sphere.radius = obb2->localSize.MaxPart() * 0.0001f;
    for (int i = 0; i < 8; ++i){
        /// Set the vertex to be a very small sphere!
        sphere.position = obb2->corners[i];

		/// Check if it's inside at all first!
		if (!obb1->IsInside(sphere))
			continue;

		bool collided = obb1->Collide(sphere, tempData);
        if (collided){
            tempData.distanceIntoEachOther = AbsoluteValue(tempData.distanceIntoEachOther);
            std::cout<<"\nDistance into each other: "<<tempData.distanceIntoEachOther<<" deepest: "<<deepestCollision.distanceIntoEachOther;
            if (tempData.distanceIntoEachOther > deepestCollision.distanceIntoEachOther){
                deepestCollision = tempData;
                std::cout<<"\nSetting deepest collission.";
            }
            std::cout<<"\nCollision Imminent!";
            collissionImminent = true;
        }
    }

    /// If we got any collission now, return it.
    if (collissionImminent){
        data = deepestCollision;
        /// Only let collissions where the points and normals are decent past..
        if (data.collissionPoint.MaxPart() > 0.0f &&
            data.collisionNormal.MaxPart() > 0.0f)
            return true;
        else {
            std::cout<<"\nCollision point or normal is wacked out yo! Cannot be considered a serious collission! o.o";
            std::cout<<"\nPoint: "<<data.collissionPoint;
            std::cout<<"\nNormal: "<<data.collisionNormal;
        }
    }

    /// If not, check for edge-edge collission!
    /// TODO: ;______;
	for (int i = 0; i < obb1->edgeList.Size(); ++i){
		Edge * edge = obb1->edgeList[i];
		for (int j = 0; j < obb2->faceList.Size(); ++j){
			Face * face = obb2->faceList[j];
			bool collided = EdgeFaceCollision(edge, face, tempData);

			if (collided){
				tempData.distanceIntoEachOther = AbsoluteValue(tempData.distanceIntoEachOther);
				std::cout<<"\nDistance into each other: "<<tempData.distanceIntoEachOther<<" deepest: "<<deepestCollision.distanceIntoEachOther;
				if (tempData.distanceIntoEachOther > deepestCollision.distanceIntoEachOther){
					deepestCollision = tempData;
					std::cout<<"\nSetting deepest collission.";
				}
				std::cout<<"\nCollision Imminent!";
				collissionImminent = true;
			}
		}
	}

	/// If we got any collission now, return it.
    if (collissionImminent){
        data = deepestCollision;
        /// Only let collissions where the points and normals are decent past..
        if (data.collissionPoint.MaxPart() > 0.0f &&
            data.collisionNormal.MaxPart() > 0.0f)
            return true;
        else {
            std::cout<<"\nCollision point or normal is wacked out yo! Cannot be considered a serious collission! o.o";
            std::cout<<"\nPoint: "<<data.collissionPoint;
            std::cout<<"\nNormal: "<<data.collisionNormal;
        }
    }


	std::cout<<"\nSAT expected collission, but no collission occured! Is this true? Check iiit!";
	Physics.Pause();
    return false;
};
