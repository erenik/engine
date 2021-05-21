// Emil Hedemalm
// 2013-09-04
// A class for optimizing collission-detection in physics.

#ifndef AXIS_ALIGNED_BOUNDING_BOX_H
#define AXIS_ALIGNED_BOUNDING_BOX_H

#include "MathLib.h"

#include "PhysicsLib/Shapes/Quad.h"
#include "Entity/Entity.h"

struct EntityPair 
{
	EntityPair();
	EntityPair(Entity* one, Entity* two);
    Entity* one;
    Entity* two;
    void PrintDetailed();
};

//#define AABB AABB

/** By default, all AABBs are re-calculated by the PhysicsManager, but only translation is taken into consideration.
	All calls to Recalcualte the AABB will perform all calculations needed.
*/
class AABB {
	friend class PhysicsManager;
public:

	enum {
		HITHER_UPPER_LEFT = 0, HITHER_UPPER_RIGHT, HITHER_LOWER_LEFT, HITHER_LOWER_RIGHT,
		FARTHER_UPPER_LEFT, FARTHER_UPPER_RIGHT, FARTHER_LOWER_LEFT, FARTHER_LOWER_RIGHT,
	};

    AABB();
    AABB(const Vector3f & min, const Vector3f & max);
    bool Intersect(const AABB &aabb2) const;
	/// Expands this AABB to include the given one.
	void Expand(AABB & aabb2);
	void Expand(ConstVec3fr vec); 
    /// Recalculate the AABBs constraints based on the given entity's transform and base model AABB.
    void Recalculate(Entity* entity);
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

	void UpdatePositionScaleUsingMinMax();

	/// Returns this AABB in the form of 8 quads.
	List<Quad> AsQuads();
	/// Returns 0 if false, 1 if intersecting, 2 if inside
//	int SphereInside(Vector3f spherePosition, float sphereRadius);

    Vector3f min;
    Vector3f max;
    Vector3f position; // Center of the AABB.
    /// Scale assuming a 1,1,1 cube centered at 0,0,0 by default (i.e.: regular total size along the axises)
    Vector3f scale;
private:

	Vector3f lastScale, lastRot;

};

#endif

