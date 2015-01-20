/// Emil Hedemalm
/// 2014-07-28
/// An editable mesh class, in contrast to the relatively optimized/compact EMesh class.

#include "EMesh.h"
#include "Material.h"
#include "OS/Sleep.h"

#include <fstream>
#include <iostream>
#include <Util.h>

#include "EVertex.h"
#include "EFace.h"
#include "EUV.h"
#include "ENormal.h"


EMesh::EMesh()
{
	Nullify();
}

	/// Nullifies all default variables!
void EMesh::Nullify()
{
	source = "Generated EMesh";
	name = "EMesh";
}

EMesh::~EMesh()
{
	faces.ClearAndDelete();
	vertices.ClearAndDelete();
	uvs.ClearAndDelete();
	normals.ClearAndDelete();
}

/// Deletes all parts within this mesh
void EMesh::Delete()
{
	vertices.ClearAndDelete();
	faces.ClearAndDelete();
	uvs.ClearAndDelete();
	normals.ClearAndDelete();
}

/// Adds a plane, creating 2 numFaces in a counter-clockwise manner.
void EMesh::AddPlane(Vector3f upperLeft, Vector3f lowerLeft, Vector3f lowerRight, Vector3f upperRight)
{
	
}

/// Adds a grid (basically a plane), with the specified amount of cells/faces in X and Y.
void EMesh::AddGrid(Vector3f upperLeft, Vector3f lowerLeft, Vector3f lowerRight, Vector3f upperRight, Vector2i gridSizeDivision)
{
	gridSizeDivision += Vector2i(1,1);

	/// For all allocated tiles/elements.
	vertexMatrix.SetDefaultValue(0);
	vertexMatrix.Allocate(gridSizeDivision);

	Vector3f up = upperLeft - lowerLeft;
	Vector3f upStep = up / (gridSizeDivision.y - 1);
	Vector3f right = lowerRight - lowerLeft;
	Vector3f rightStep = right / (gridSizeDivision.x - 1);
	// First create the necessary vertices.
	for (int x = 0; x < gridSizeDivision.x; ++x)
	{
		for (int y = 0; y < gridSizeDivision.y; ++y)
		{
			// Create the vertex
			EVertex * vertex = new EVertex();
			// Add it to the mesh's list of vertices.
			vertices.Add(vertex);

			*vertex = lowerLeft + rightStep * x + upStep * y;
			
			assert(vertexMatrix.At(Vector2i(x,y)) == 0);
			vertexMatrix.Set(Vector2i(x,y), vertex);

			// Create a UV-coordinate for each vertex too, I guess?
			EUV * uv = new EUV();
			uv->x = x / (float) (gridSizeDivision.x - 1);
			uv->y = y / (float) (gridSizeDivision.y - 1);
			uvs.Add(uv);
			// Associate it with the vertex.
			vertex->uvCoord = uv;
			uv->vertex = vertex;
		}
	}
	/// From the matrix of vertices, create a matrix of faces!
	faceMatrix.SetDefaultValue(0);
	faceMatrix.Allocate(gridSizeDivision - Vector2i(1,1));
	for (int x = 0; x < gridSizeDivision.x - 1; ++x)
	{
		for (int y = 0; y < gridSizeDivision.y - 1; ++y)
		{
			EFace * face = new EFace();
			/// Add, in counter-clockwise order, the 4 neighbouring vertices, making this a quad.
			// Top left first..
			face->AddVertex(vertexMatrix.At(x,y+1));
			face->AddVertex(vertexMatrix.At(x,y));
			face->AddVertex(vertexMatrix.At(x+1,y));
			face->AddVertex(vertexMatrix.At(x+1,y+1));

			// Calculate an average normal straight away?
			face->CalculateNormal();
			faces.Add(face);
		}
	}
	// .. assert shit.
}


void EMesh::SetSource(String str){
	source = str;
}

void EMesh::SetName(String str){
	name = str;
}
