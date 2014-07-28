
#include "PhysicsMesh.h"
#include "Physics/Collision/CollisionShapeOctree.h"
#include "Mesh/Mesh.h"


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

/// Generates a collission shape octree that can be used in the local-coordinate system or multiplied by matrices to be used globally.
void PhysicsMesh::GenerateCollisionShapeOctree()
{
	assert(collisionShapeOctree == NULL && "Attempting to generate collission shape octree where not needed! This should only be done after loading the model.");
	Mesh * mesh = (Mesh*)meshCounterpart;
	assert(mesh != NULL);
	collisionShapeOctree = new CollisionShapeOctree();
	if (mesh->min.MaxPart() == 0 && mesh->max.MaxPart() == 0)
		mesh->CalculateBounds();
	assert(mesh->min.MaxPart() != 0 && mesh->max.MaxPart() != 0);
	/// Fetch size of object and extend the size of the octree by 10% just to make sure that it works later on.
	Vector3f size = mesh->max - mesh->min;
	collisionShapeOctree->SetBoundaries(
		mesh->min.x - size.x * 0.1f,
		mesh->max.x + size.x * 0.1f,
		mesh->max.y + size.y * 0.1f,
		mesh->min.y - size.y * 0.1f,
		mesh->max.z + size.z * 0.1f,
		mesh->min.z - size.z * 0.1f);

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
