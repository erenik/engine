/// Emil Hedemalm
/// 2014-07-27 (header added recently, original was much older)
/// A complete model, which may have multiple mesh-parts.

#include "Model.h"
#include "Mesh/Mesh.h"

#include <cstdlib>

#include "PhysicsLib/Shapes/AABB.h"
#include "PhysicsLib/Shapes/Triangle.h"

char * Model::defaultMesh = NULL;
Texture * Model::defaultTexture = NULL;


Model::Model()
{
	users = 0;
	radius = 0;
	triangleList = NULL;

	mesh = NULL;
	triangulizedMesh = NULL;
}

Model::~Model()
{
	// Do stuff
	if (triangleList)
	{
		CLEAR_AND_DELETE((*triangleList));
		delete triangleList;
	}
	triangleList = NULL;
	
	if (mesh)
		delete mesh;
	if (triangulizedMesh)
		delete triangulizedMesh;

	mesh = triangulizedMesh = NULL;
}

String Model::Source(){
	if (mesh)
		return mesh->source;
	return "";
}

String Model::RelativePath()
{ 
	return mesh->RelativePath(); 
}

/// Calls render on the triangulized mesh parts within.
void Model::Render()
{
	Mesh * triangulatedMesh = GetTriangulatedMesh();
	if (!triangulatedMesh)
	{
		std::cout<<"\nUnable to render model: "<<name;
		return;
	}
	triangulatedMesh->Render();
}

void Model::SetName(String i_name){
	// Deallocate if needed, then re-allocate.
	name = i_name;
}


/// Returns all triangles in this model.
List<Triangle> Model::GetTris(){
	List<Triangle> triangleList;

	if (triangulizedMesh){
		List<Triangle> triMeshTris = triangulizedMesh->GetTris();
		triangleList += triMeshTris;
	}
	assert(triangleList.Size() > 0);
	return triangleList;
}


/// Returns the triangulized mesh, which may or may not be the original mesh depending on.. stuff.
Mesh * Model::GetTriangulatedMesh()
{
	if (mesh->IsTriangulated())
		return mesh;
	else if (triangulizedMesh)
		return triangulizedMesh;
	assert(false && "Lakcing triangulized mesh");
	return NULL;
}

/// Re-creates the triangulized mesh. Call after changes have been made to the base mesh.
bool Model::RegenerateTriangulizedMesh()
{
	// Hope noone's using this model.. maybe flag it somehow first..
	if (triangulizedMesh)
		// Delete all but any potential GL buffers we were using.
		triangulizedMesh->Delete();
	else
		triangulizedMesh = new Mesh();

	triangulizedMesh->LoadDataFrom(mesh);
	triangulizedMesh->Triangulate();
	return true;
}

/// Returns the AABB.
const AABB & Model::GetAABB()
{
	return *mesh->aabb;
}
