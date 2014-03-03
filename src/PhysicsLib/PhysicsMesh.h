// Emil Hedemalm
// 2013-03-20

#ifndef PHYSICS_MESH_H
#define PHYSICS_MESH_H

#include <Util.h>

struct Triangle;
struct Quad;
struct Ngon;
class Mesh;
class CollisionShapeOctree;

// Specialization structure for handling physics quickly
struct PhysicsMesh {
	/// Construct-Destruct
	PhysicsMesh();
	~PhysicsMesh();
	/// Generates a collission shape octree that can be used in the local-coordinate system or multiplied by matrices to be used globally.
	void GenerateCollisionShapeOctree();
	/// Vars
	List<Triangle*> triangles;
	List<Quad*> quads;
	List<Ngon*> ngons;
	/// For referenceŜ
	String source;
	const Mesh * meshCounterpart;
	/// Structure used for optimizing collission detection
	CollisionShapeOctree * collisionShapeOctree;
};

#endif
