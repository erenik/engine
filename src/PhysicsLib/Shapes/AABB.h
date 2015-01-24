// Emil Hedemalm
// 2013-09-04
// A class for optimizing collission-detection in physics.

#ifndef AXIS_ALIGNED_BOUNDING_BOX_H
#define AXIS_ALIGNED_BOUNDING_BOX_H

#include "MathLib.h"

class Entity;

//#define AABB AABB

/** By default, all AABBs are re-calculated by the PhysicsManager, but only translation is taken into consideration.
	All calls to Recalcualte the AABB will perform all calculations needed.
*/
class AABB {
	friend class PhysicsManager;
public:
    AABB();
    AABB(Vector3f min, Vector3f max);
    bool Intersect(const AABB &aabb2) const;
    /// Recalculate the AABBs constraints based on the given entity's transform and base model AABB.
    void Recalculate(Entity * entity);

    Vector3f min;
    Vector3f max;
    Vector3f position;
    /// Scale assuming a 1,1,1 cube centered at 0,0,0 by default (i.e.: regular total size along the axises)
    Vector3f scale;
private:

	Vector3f lastScale, lastRot;

};

#endif

