/// Emil Hedemalm
/// 2014-02-13
/// Square mesh generated and meant to be used by UI rendering.

#include "Square.h"

/// A simple 2D square mesh used mainly for UI
Square::Square()
: Mesh()
{
	// Set numbers
	faces = 2;
	normals = 1;
	uvs = 4;
	vertices = 4;
	// Allocate data
	vertex = new Vector3f[vertices];
	normal = new Vector3f[normals];
	u = new float[uvs];
	v = new float[uvs];
	face = new MeshFace[faces];
	for (int i = 0; i < faces; ++i){
		int numV = 3;
		face[i].numVertices = numV;
		face[i].normal = new unsigned int[numV];
		face[i].uv = new unsigned int[numV];
		face[i].vertex = new unsigned int[numV];
	}
	/// Set default vector
	normal[0] = Vector3f(0, 0, 1);	// Vector toward user
	/// Set default UVs
	u[0] = 0;		v[0] = 0.0f;
	u[1] = 1.0f;	v[1] = 0.0f;
	u[2] = 1.0f;	v[2] = 1.0f;
	u[3] = 0;		v[3] = 1.0f;

	/// Set default face indices
	face[0].vertex[0] = 0;	face[0].uv[0] = 0;	face[0].normal[0] = 0;
	face[0].vertex[1] = 1;	face[0].uv[1] = 1;	face[0].normal[1] = 0;
	face[0].vertex[2] = 2;	face[0].uv[2] = 2;	face[0].normal[2] = 0;

	face[1].vertex[0] = 2;	face[1].uv[0] = 2;	face[1].normal[0] = 0;
	face[1].vertex[1] = 3;	face[1].uv[1] = 3;	face[1].normal[1] = 0;
	face[1].vertex[2] = 0;	face[1].uv[2] = 0;	face[1].normal[2] = 0;
};

/// Virtual destructor so that base class destructor is called correctly.
Square::~Square()
{
	/// Woo.
	std::cout<<"\nSquare destructor.";
	for (int i = 0; i < faces; ++i){
		int numV = 3;
		face[i].numVertices = numV;
		delete[] face[i].normal;
		delete[] face[i].uv;
		delete[] face[i].vertex;
		memset(&face[i], 0, sizeof(MeshFace));
	}
#define DeleteArr(a) {delete[] a; a= NULL;}
	DeleteArr(face);
	DeleteArr(u);
	DeleteArr(v);
	DeleteArr(vertex);
	DeleteArr(normal);
};

/// Sets the dimensions of the square using provided arguments
void Square::SetDimensions(float left, float right, float bottom, float top, float z /*= 0*/)
{
	vertex[0] = Vector3f(left, bottom, z);
	vertex[1] = Vector3f(right, bottom, z);
	vertex[2] = Vector3f(right, top, z);
	vertex[3] = Vector3f(left, top, z);
};
/// Sets UV coordinates of the square using provided arguments
void Square::SetUVs(float left, float right, float bottom, float top)
{
	u[0] = left;	v[0] = bottom;
	u[1] = right;	v[1] = bottom;
	u[2] = right;	v[2] = top;
	u[3] = left;	v[3] = top;
};
