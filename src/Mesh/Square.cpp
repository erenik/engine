/// Emil Hedemalm
/// 2014-02-13
/// Square mesh generated and meant to be used by UI rendering.

#include "Square.h"

/// A simple 2D square mesh used mainly for UI
Square::Square()
: Mesh()
{
	// Set numbers
	numFaces = 2;
	numNormals = 1;
	numUVs = 4;
	numVertices = 4;
	// Allocate data
	AllocateArrays();
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace * face;
		int numV = 3;
		face = &faces[i];
		face->numVertices = numV;
		face->AllocateArrays();
	}
	/// Set default vector
	normals[0] = Vector3f(0, 0, 1);	// Vector toward user
	/// Set default UVs
	uvs[0] = Vector2f(0, 0.0f);
	uvs[1] = Vector2f(1.f, 0.0f);
	uvs[2] = Vector2f(1.f, 1.0f);
	uvs[3] = Vector2f(0, 1.0f);
	
	/// Set default face indices
	faces[0].vertices[0] = 0;	faces[0].uvs[0] = 0;	faces[0].normals[0] = 0;
	faces[0].vertices[1] = 1;	faces[0].uvs[1] = 1;	faces[0].normals[1] = 0;
	faces[0].vertices[2] = 2;	faces[0].uvs[2] = 2;	faces[0].normals[2] = 0;

	faces[1].vertices[0] = 2;	faces[1].uvs[0] = 2;	faces[1].normals[0] = 0;
	faces[1].vertices[1] = 3;	faces[1].uvs[1] = 3;	faces[1].normals[1] = 0;
	faces[1].vertices[2] = 0;	faces[1].uvs[2] = 0;	faces[1].normals[2] = 0;

	// Already trianguilated yo.
	triangulated = true;
};

/// Virtual destructor so that base class destructor is called correctly.
Square::~Square()
{
	/// Woo.
//	std::cout<<"\nSquare destructor.";
	for (int i = 0; i < faces.Size(); ++i)
	{
		int numV = 3;
		MeshFace * face = &faces[i];
		face->DeallocateArrays();
		face->numVertices = 0;
	}
	DeallocateArrays();
};

/// Sets the dimensions of the square using provided arguments
void Square::SetDimensions(float left, float right, float bottom, float top, float z /*= 0*/)
{
	vertices[0] = Vector3f(left, bottom, z);
	vertices[1] = Vector3f(right, bottom, z);
	vertices[2] = Vector3f(right, top, z);
	vertices[3] = Vector3f(left, top, z);
};
/// Sets UV coordinates of the square using provided arguments
void Square::SetUVs(float left, float right, float bottom, float top)
{
	uvs[0] = Vector2f(left, bottom);
	uvs[1] = Vector2f(right, bottom);
	uvs[2] = Vector2f(right, top);
	uvs[3] = Vector2f(left, top);
	
};
