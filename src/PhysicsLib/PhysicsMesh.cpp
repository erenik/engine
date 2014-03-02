
#include "PhysicsMesh.h"
#include "Physics/Collission/CollissionShapeOctree.h"
#include "Mesh.h"


PhysicsMesh::PhysicsMesh(){
	collissionShapeOctree = NULL;
};

PhysicsMesh::~PhysicsMesh(){
	// Deallocate all dynamically allocated members
	triangles.ClearAndDelete();
	quads.ClearAndDelete();
	ngons.ClearAndDelete();
	if (collissionShapeOctree)
		delete collissionShapeOctree;
	collissionShapeOctree = NULL;
};

/// Generates a collission shape octree that can be used in the local-coordinate system or multiplied by matrices to be used globally.
void PhysicsMesh::GenerateCollissionShapeOctree(){
	assert(collissionShapeOctree == NULL && "Attempting to generate collission shape octree where not needed! This should only be done after loading the model.");
	Mesh * mesh = (Mesh*)meshCounterpart;
	assert(mesh != NULL);
	collissionShapeOctree = new CollissionShapeOctree();
	if (mesh->min.MaxPart() == 0 && mesh->max.MaxPart() == 0)
		mesh->CalculateBounds();
	assert(mesh->min.MaxPart() != 0 && mesh->max.MaxPart() != 0);
	collissionShapeOctree->SetBoundaries(
		mesh->min.x * 1.1f,
		mesh->max.x * 1.1f,
		mesh->max.y * 1.1f,
		mesh->min.y * 1.1f,
		mesh->max.z * 1.1f,
		mesh->min.z * 1.1f);

	// Adding triangles..
	for (int i = 0; i < triangles.Size(); ++i){
		collissionShapeOctree->AddTriangle(triangles[i]);

	//	collissionShapeOctree->PrintContents();
	}
//	collissionShapeOctree->PrintContents();
	std::cout<<"\nDret.";
}
