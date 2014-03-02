// Emil Hedemalm
// 2013-09-04
// A class for optimizing collission-detection in physics.

#include "AxisAlignedBoundingBox.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Model.h"
#include "Mesh.h"
#include "Physics/PhysicsManager.h"

/// Fucking macros (on windows)
#undef max
#undef min

AxisAlignedBoundingBox::AxisAlignedBoundingBox(){
    min = Vector3f(-0.5f,-0.5f,-0.5f);
    max = Vector3f(0.5f,0.5f,0.5);
    scale = Vector3f(1,1,1);
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(Vector3f min, Vector3f max)
: min(min), max(max)
{
    scale = max - min;
    position = (max + min) * 0.5f;
}

bool AxisAlignedBoundingBox::Intersect(const AxisAlignedBoundingBox &aabb2) const {
    if (min.x > aabb2.max.x ||
        min.y > aabb2.max.y ||
        min.z > aabb2.max.z ||
        max.x < aabb2.min.x ||
        max.y < aabb2.min.y ||
        max.z < aabb2.min.z
        )
        return false;
    return true;
}

/// Recalculate the AABBs constraints based on the given entity using it.
void AxisAlignedBoundingBox::Recalculate(Entity * entity){

    Vector3f bounds[8];
    /// Reset min and max depending on the base AABB.
    min = entity->model->aabb.min;
    max = entity->model->aabb.max;
    Vector3f newMin, newMax;
    /// 8 vectors (extents), for-looped for funs.
    for (int i = 0; i < 8; ++i){
        if (i < 4)
            bounds[i].x = min.x;
        else
            bounds[i].x = max.x;
        if (i%4 < 2)
            bounds[i].y = min.y;
        else
            bounds[i].y = max.y;
        if (i%2 == 0)
            bounds[i].z = min.z;
        else
            bounds[i].z = max.z;

        /// Transform according to entity
        bounds[i] = entity->transformationMatrix.product(bounds[i]);

        if (i == 0)
            newMin = newMax = bounds[i];
        else {
            // Get min
            if (bounds[i].x < newMin.x)
                newMin.x = bounds[i].x;
            if (bounds[i].y < newMin.y)
                newMin.y = bounds[i].y;
            if (bounds[i].z < newMin.z)
                newMin.z = bounds[i].z;
            // Get max
            if (bounds[i].x > newMax.x)
                newMax.x = bounds[i].x;
            if (bounds[i].y > newMax.y)
                newMax.y = bounds[i].y;
            if (bounds[i].z > newMax.z)
                newMax.z = bounds[i].z;
        }
    }

    /// Update depending on the new stats!
    min = newMin;
    max = newMax;

    /// Update scale and such.
    position = (max + min) / 2.0f;
    scale = max - min;
}
