// Emil Hedemalm
// 2013-03-20

#ifndef PHYSICS_MESH_H
#define PHYSICS_MESH_H

#include "MathLib.h"
#include <Util.h>
#include "PhysicsLib/Intersection.h"
#include "PhysicsLib/Shapes/AABB.h"

class Ray;
class Triangle;
class Quad;
class Ngon;
class Mesh;
class CollisionShapeOctree;

// Specialization structure for handling physics quickly
class PhysicsMesh 
{
public:
	/// Construct-Destruct
	PhysicsMesh();
	~PhysicsMesh();
	/// Loads
	bool LoadFrom(Mesh * mesh);

	/// Performs a raycast considering target ray and the transform of this physics mesh.
	List<Intersection> Raycast(Ray & ray, Matrix4f & transform);
	/// Generates a collission shape octree that can be used in the local-coordinate system or multiplied by matrices to be used globally.
	void GenerateCollisionShapeOctree();
	/// Vars
	List<Triangle*> triangles;
	List<Quad*> quads;
	List<Ngon*> ngons;
	/// Name, if based on mesh, same name as mesh.
	String name;
	/// Source name of source mesh.
	String source;
	/// Inherited from source mesh (or generated).
	AABB aabb;
//	const Mesh * meshCounterpart;
	/// Structure used for optimizing collission detection
	CollisionShapeOctree * collisionShapeOctree;

	/// If true, skips collision calcs with edges/corners of tris/quads. Default false.
	bool planeCollisionsOnly;

	/// Default false? Since they aren't optimized anyway?
	static bool useCollisionShapeOctrees;
};

#endif
