/// Emil Hedemalm
/// 2014-08-01
/// Old contents, but new file separation. 

#include "MeshFace.h"

MeshFace::MeshFace()
{
    numVertices = 0;
//	Nullify();
}
void MeshFace::Nullify()
{
    numVertices = 0;
};
MeshFace::~MeshFace()
{
	// No need to de-allocate... all arrays in List objects are de-allocated automatically!
//	vertices.Deallocate();
//	uvs.Deallocate();
//	normals.Deallocate();
}

/// Debug
void MeshFace::Print()
{
	std::cout<<"\nv:";
	for (int i = 0; i < numVertices; ++i){
		std::cout<<" "<<vertices[i];
	}
}

/// Copy CONSTRUCTOR
MeshFace::MeshFace(const MeshFace & otherMeshFace){
     Nullify();
     std::cout<<"op3";
     assert(false);
}
void MeshFace::operator = (const MeshFace * otherMeshFace){
     std::cout<<"op2";
     assert(false);
}

/// Copy CONSTRUCTOR
void MeshFace::operator = (const MeshFace & otherMeshFace)
{
	numVertices = otherMeshFace.numVertices;
	assert(otherMeshFace.numVertices);
	
	vertices = otherMeshFace.vertices;
	uvs = otherMeshFace.uvs;
	normals = otherMeshFace.normals;

	uvTangent = otherMeshFace.uvTangent;
 }

// Call after setting numVertices
void MeshFace::AllocateArrays()
{
	vertices.Allocate(numVertices, true);
	uvs.Allocate(numVertices, true);
	normals.Allocate(numVertices, true);
}

void MeshFace::DeallocateArrays()
{
	vertices.Clear();
	uvs.Clear();
	normals.Clear();
}

bool MeshFace::WriteTo(std::fstream & file)
{
	assert(numVertices > 0);
	file.write((char*)&numVertices, sizeof(int));
//	std::cout<<"\nNumVertices: "<<numVertices;
	
	file.write((char*)vertices.GetArray(), sizeof(int) * numVertices);
	file.write((char*)uvs.GetArray(), sizeof(int) * numVertices);
	file.write((char*)normals.GetArray(), sizeof(int) * numVertices);
	//*/
	return true;
}

bool MeshFace::ReadFrom(std::fstream & file)
{
	file.read((char*)&numVertices, sizeof(int));
	
	/// Why are there 0 vertices? This face should have been skipped if so?
	assert(numVertices > 0);
//	assert(numVertices < 20);
	// Assume deallocation before is not necessary...
	AllocateArrays();
	/*
	for (int i = 0; i < numVertices; ++i)
	{
	vertices[i]
	}
	/**/
	
	file.read((char*)vertices.GetArray(), sizeof(int) * numVertices);
	file.read((char*)uvs.GetArray(), sizeof(int) * numVertices);
	file.read((char*)normals.GetArray(), sizeof(int) * numVertices);
	//*/
	return true;
}


