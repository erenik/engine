
#include "Model.h"

#include <cstdlib>
#include "PhysicsLib/Shapes.h"

char * Model::defaultMesh = NULL;
Texture * Model::defaultTexture = NULL;


Model::Model()
{
	users = 0;
	this->radius = 0;
	triangleList = NULL;

	mesh = NULL;
	triangulizedMesh = NULL;
 }

Model::~Model()
{
	// Do stuff
	if (triangleList)
		CLEAR_AND_DELETE((*triangleList));
	delete triangleList;
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
	return NULL;
}

