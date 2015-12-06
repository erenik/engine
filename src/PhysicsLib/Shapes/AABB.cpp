// Emil Hedemalm
// 2013-09-04
// A class for optimizing collission-detection in physics.

#include "AABB.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Model/Model.h"
#include "Mesh/Mesh.h"
#include "Physics/PhysicsManager.h"
#include "PhysicsLib/Location.h"

/// Fucking macros (on windows)
#undef max
#undef min

AABB::AABB()
{
}

AABB::AABB(const Vector3f & min, const Vector3f & max)
: min(min), max(max)
{
    scale = max - min;
    position = (max + min) * 0.5f;
}

bool AABB::Intersect(const AABB &aabb2) const 
{
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

bool debugAABB = false;

/// Recalculate the AABBs constraints based on the given entity using it.
void AABB::Recalculate(Entity * entity)
{
	if (!entity->model)
		return;
	if (!entity->model->mesh->aabb)
		return;
	
	/// Check physics-type.
	PhysicsProperty * pp = entity->physics;
	/// Default: mesh?
	int physicsShape = PhysicsShape::MESH;
	if (pp)
		physicsShape = pp->shapeType;
	Vector3f entityWorldPosition = entity->worldPosition;
	switch(physicsShape)
	{
		case PhysicsShape::SPHERE:
		{
			Vector3f radiusVec = Vector3f(1,1,1) * entity->radius;
			this->position = entityWorldPosition;
			this->min = position - radiusVec;
			this->max = position + radiusVec;
			this->scale = radiusVec * 2;
			return;
		}
		case PhysicsShape::MESH:
			// Default use below method.
			break;
		case PhysicsShape::AABB:
		{
			/// Use default below,.. I guess?
			break;
		}
		default:
			assert(false && "Implement");
	}


	/// Recalculate from scratch if re-scaled or rotated?    
	lastScale = entity->scale;
	lastRot = entity->rotation;
	Vector3f bounds[8];
	/// Reset min and max depending on the base AABB.
	min = entity->model->mesh->aabb->min;
	max = entity->model->mesh->aabb->max;
	Vector3f newMin, newMax;
	/// 8 vectors (extents), for-looped for funs.
	for (int i = 0; i < 8; ++i)
	{
		if (i < 4)
			bounds[i][0] = min[0];
		else
			bounds[i][0] = max[0];
		if (i%4 < 2)
			bounds[i][1] = min[1];
		else
			bounds[i][1] = max[1];
		if (i%2 == 0)
			bounds[i][2] = min[2];
		else
			bounds[i][2] = max[2];

		if (debugAABB)
			std::cout<<"\nBound "<<i<<": "<<bounds[i];

		/// Transform according to entity
		bounds[i] = entity->transformationMatrix.Product(bounds[i]);

		if (debugAABB)
			std::cout<<"\nTransformed "<<i<<": "<<bounds[i];

		if (i == 0)
			newMin = newMax = bounds[i];
		else {
			// Get min
			if (bounds[i][0] < newMin[0])
				newMin[0] = bounds[i][0];
			if (bounds[i][1] < newMin[1])
				newMin[1] = bounds[i][1];
			if (bounds[i][2] < newMin[2])
				newMin[2] = bounds[i][2];
			// Get max
			if (bounds[i][0] > newMax[0])
				newMax[0] = bounds[i][0];
			if (bounds[i][1] > newMax[1])
				newMax[1] = bounds[i][1];
			if (bounds[i][2] > newMax[2])
				newMax[2] = bounds[i][2];
		}
	}

	/// Update depending on the new stats!
	min = newMin;
	max = newMax;

	if (debugAABB)
		std::cout<<"\nMin "<<min<<" Max "<<max;

	/// Update scale and such.
	position = (max + min) * 0.5f;
	scale = max - min;
	if (debugAABB)
		std::cout<<"\nPosition "<<position<<" Scale "<<scale;
}

bool AABB::WriteTo(std::fstream & file)
{
	// version?
	int version = 0;
	file.write((char*)&version, sizeof(int));
	min.WriteTo(file);
	max.WriteTo(file);
	position.WriteTo(file);
	scale.WriteTo(file);
	return true;
}
bool AABB::ReadFrom(std::fstream & file)
{
	int version, currentVersion = 0;
	file.read((char*)&version, sizeof(int));
	if (version != currentVersion)
		return false;
	min.ReadFrom(file);
	max.ReadFrom(file);
	position.ReadFrom(file);
	scale.ReadFrom(file);
	return true;
}



/// Returns this AABB in the form of 8 quads.
List<Quad> AABB::AsQuads()
{
	List<Quad> quads;
	List<Vector3f> points;
	points.Allocate(8, true);

	/// Z+ is higher, Y+ is upper, X+ is right.
	points[HITHER_UPPER_RIGHT] = max;
	points[HITHER_UPPER_LEFT] = max - Vector3f(scale.x,0,0);
	points[HITHER_LOWER_RIGHT] = max - Vector3f(0, scale.y,0);
	points[HITHER_LOWER_LEFT] = min + Vector3f(0, 0, scale.z);

	points[FARTHER_LOWER_LEFT] = min;
	points[FARTHER_LOWER_RIGHT] = min + Vector3f(scale.x,0,0);
	points[FARTHER_UPPER_LEFT] = min + Vector3f(0, scale.y,0);
	points[FARTHER_UPPER_RIGHT] = max - Vector3f(0, 0, scale.z);

	/// Right. Create the quads! .. was it clockwise or counter clockwise.
	bool clockwise = false;
	if (clockwise)
	{
		/// Clockwise order (seen from outside it).
		quads.Add(
			/// Hither.
			Quad(points[HITHER_UPPER_RIGHT], points[HITHER_LOWER_RIGHT], points[HITHER_LOWER_LEFT], points[HITHER_UPPER_LEFT]),
			/// Farther
			Quad(points[FARTHER_UPPER_LEFT], points[FARTHER_LOWER_LEFT], points[FARTHER_LOWER_RIGHT], points[FARTHER_UPPER_RIGHT]),
			/// Left
			Quad(points[HITHER_UPPER_LEFT], points[HITHER_LOWER_LEFT], points[FARTHER_LOWER_LEFT], points[FARTHER_UPPER_LEFT]),
			/// Right
			Quad(points[HITHER_LOWER_RIGHT], points[HITHER_UPPER_RIGHT], points[FARTHER_UPPER_RIGHT], points[FARTHER_LOWER_RIGHT])
		);
		quads.Add(
			/// Up.
			Quad(points[HITHER_UPPER_RIGHT], points[HITHER_UPPER_LEFT], points[FARTHER_UPPER_LEFT], points[FARTHER_UPPER_RIGHT]),
			/// Down.
			Quad(points[HITHER_LOWER_LEFT], points[HITHER_LOWER_RIGHT], points[FARTHER_LOWER_RIGHT], points[FARTHER_LOWER_LEFT])
		);
	}
	else 
	{
		/// Counter-clockwise order (seen from outside it).
		quads.Add(
			/// Hither, farther
			Quad(points[HITHER_UPPER_RIGHT], points[HITHER_UPPER_LEFT], points[HITHER_LOWER_LEFT], points[HITHER_LOWER_RIGHT]),
			Quad(points[FARTHER_UPPER_LEFT], points[FARTHER_UPPER_RIGHT], points[FARTHER_LOWER_RIGHT], points[FARTHER_LOWER_LEFT]),
			/// Left, right
			Quad(points[HITHER_UPPER_LEFT], points[FARTHER_UPPER_LEFT], points[FARTHER_LOWER_LEFT], points[HITHER_LOWER_LEFT]),
			Quad(points[HITHER_LOWER_RIGHT], points[FARTHER_LOWER_RIGHT], points[FARTHER_UPPER_RIGHT], points[HITHER_UPPER_RIGHT])
		);
		quads.Add(
			/// Up n down.
			Quad(points[HITHER_UPPER_RIGHT], points[FARTHER_UPPER_RIGHT], points[FARTHER_UPPER_LEFT], points[HITHER_UPPER_LEFT]),
			Quad(points[HITHER_LOWER_LEFT], points[FARTHER_LOWER_LEFT], points[FARTHER_LOWER_RIGHT], points[HITHER_LOWER_RIGHT])
		);
	}
	/// Test it?

	/*
	Vector3f point = position;
	for (int i = 0; i < quads.Size(); ++i)
	{
		Quad & quad = quads[i];
		float distance = quad.Distance(point);
		std::cout<<"\nDist: "<<distance;
	}*/


	return quads;
}


/*
/// Returns 0 if false, 1 if intersecting, 2 if inside
int AABB::SphereInside(Vector3f spherePosition, float sphereRadius)
{
	/*
	// Make box test insteeeead
	for (int i = 0; i < quads.Size(); ++i)
	{
		// Check if it's inside.
		if (entity->position[0] + entity->physics->physicalRadius < right &&
			entity->position[0] - entity->physics->physicalRadius > left &&
			entity->position[1] + entity->physics->physicalRadius < top &&
			entity->position[1] - entity->physics->physicalRadius > bottom &&
			entity->position[2] + entity->physics->physicalRadius < nearBound &&
			entity->position[2] - entity->physics->physicalRadius > farBound
		)
			// Inside
			continue;
		// Or intersecting, just compare with inverted radius
		else if (entity->position[0] - entity->physics->physicalRadius < right &&
			entity->position[0] + entity->physics->physicalRadius > left &&
			entity->position[1] - entity->physics->physicalRadius < top &&
			entity->position[1] + entity->physics->physicalRadius > bottom &&
			entity->position[2] - entity->physics->physicalRadius < nearBound &&
			entity->position[2] + entity->physics->physicalRadius > farBound
		)
			// Intersect
			continue;
		// It's outside if the previous were false, logical :P
		return false;
	}
}
*/