/// Emil Hedemalm
/// 2014-08-10
/// A physics mesh based on a regular mesh.


#include "PhysicsMesh.h"
#include "Physics/Collision/CollisionShapeOctree.h"
#include "Mesh/Mesh.h"

#include "PhysicsLib/Shapes/Ray.h"
#include "PhysicsLib/Shapes/Ngon.h"
#include "PhysicsLib/Shapes/Quad.h"

#include "PhysicsLib/Shapes/AABB.h"

PhysicsMesh::PhysicsMesh(){
	collisionShapeOctree = NULL;
};

PhysicsMesh::~PhysicsMesh(){
	// Deallocate all dynamically allocated members
	triangles.ClearAndDelete();
	quads.ClearAndDelete();
	ngons.ClearAndDelete();
	if (collisionShapeOctree)
		delete collisionShapeOctree;
	collisionShapeOctree = NULL;
};

/// Performs a raycast considering target ray and the transform of this physics mesh.
List<Intersection> PhysicsMesh::Raycast(Ray & ray, Matrix4f & transform)
{
	List<Intersection> intersections;
	// o-o
	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle triangle = *triangles[i];	
		triangle.Transform(transform);
		// Do intersection test
		float distance;
		if (ray.Intersect(triangle, &distance))
		{
			Intersection newI;
			newI.distance = distance;
			intersections.Add(newI);
		}
	}
	return intersections;
}

/// Generates a collission shape octree that can be used in the local-coordinate system or multiplied by matrices to be used globally.
void PhysicsMesh::GenerateCollisionShapeOctree()
{
	assert(collisionShapeOctree == NULL && "Attempting to generate collission shape octree where not needed! This should only be done after loading the model.");
	Mesh * mesh = (Mesh*)meshCounterpart;
	assert(mesh != NULL);
	collisionShapeOctree = new CollisionShapeOctree();
	if (!mesh->aabb)
		mesh->CalculateBounds();
	assert(mesh->aabb);
	/// Fetch size of object and extend the size of the octree by 10% just to make sure that it works later on.
	Vector3f size = mesh->aabb->scale;
	Vector3f min = mesh->aabb->min,
		max = mesh->aabb->max;
	collisionShapeOctree->SetBoundaries(
		min.x - size.x * 0.1f,
		max.x + size.x * 0.1f,
		max.y + size.y * 0.1f,
		min.y - size.y * 0.1f,
		max.z + size.z * 0.1f,
		min.z - size.z * 0.1f);

	// Adding triangles..
	for (int i = 0; i < triangles.Size(); ++i)
	{
		Triangle * triangle = triangles[i];
		assert(triangle->normal.MaxPart());
		collisionShapeOctree->AddTriangle(triangles[i]);
	//	collisionShapeOctree->PrintContents();
	}

	
//	collisionShapeOctree->PrintContents();
	
	/// Optimize it! <- Does what?
	collisionShapeOctree->Optimize();
	std::cout<<"\nOptimizing collission shape octree by removing unused child cells.";
//	collisionShapeOctree->PrintContents();
	std::cout<<"\nCollision shape octree generated for mesh: "<<this->meshCounterpart->name;
}
