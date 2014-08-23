// Emil Hedemalm
// 2013-03-20

//#include <windows.h>
#include "Mesh/Mesh.h"

#include "Mesh/EFace.h"
#include "Mesh/EMesh.h"
#include "Mesh/EVertex.h"

#include "Graphics/OpenGL.h"
#include "Graphics/GLBuffers.h"
//#include "Graphics/GraphicsManager.h"
//#include "GraphicsState.h"

#include "Material.h"
#include "OS/Sleep.h"

#include <fstream>
#include <iostream>
#include <Util.h>

#include "Matrix/Matrix.h"

#include "PhysicsLib/Shapes/Triangle.h"
#include "PhysicsLib/Shapes/AABB.h"

Mesh::Mesh()
{
	Nullify();
}

	/// Nullifies all default variables!
void Mesh::Nullify()
{
	numVertices = numUVs = numFaces = numNormals = 0;
	textureID = 0;
	vboBuffer = NULL;
	floatsPerVertex = 0;
	triangulated = false;
	loadedFromCompactObj = false;
	aabb = NULL;
}

Mesh::~Mesh()
{
	/*
	// Deallocate the lists.
	vertices.Deallocate();
	std::cout<<"lall";
	uvs.Deallocate();
	std::cout<<"lall";
	normals.Deallocate();
	std::cout<<"lall";
	faces.Deallocate();
	*/
	if (aabb)
		delete aabb;
}

/// Deletes all parts within this mesh (numVertices, numFaces, edges, etc.)
void Mesh::Delete()
{
	DeallocateArrays();
	numVertices = numUVs = numNormals = numFaces = 0;
//	min = max = Vector3f();
	triangulated = false;
}

/// Adds a plane, creating 2 numFaces in a counter-clockwise manner.
void Mesh::AddPlane(Vector3f upperLeft, Vector3f lowerLeft, Vector3f lowerRight, Vector3f upperRight)
{
	
}

/// Adds a grid (basically a plane), with the specified amount of cells/faces in X and Y.
void Mesh::AddGrid(Vector3f upperLeft, Vector3f lowerLeft, Vector3f lowerRight, Vector3f upperRight, Vector2i gridSizeDivision)
{
	assert(false);
}


// Allocates the vertices, u,v and normals arrays
void Mesh::AllocateArrays()
{
	if (numVertices)
		vertices.Allocate(numVertices, true);
	if (numUVs)
		uvs.Allocate(numUVs, true);
	if (numNormals)
		normals.Allocate(numNormals, true);
	if (numFaces)
		faces.Allocate(numFaces, true);
}

void Mesh::DeallocateArrays()
{
//	std::cout<<"\nMesh destructor.";
	vertices.Clear();
	uvs.Clear();
	normals.Clear();
	faces.Clear();
}

	struct p1 {
		float a;
	};
	struct p2 {
		float a,b;
	};
	struct p3 {
		float a,b,c;
	};


/// Load from customized compressed data form. Returns true upon success.
bool Mesh::SaveCompressedTo(String compressedPath)
{
	std::fstream file;
	file.open(compressedPath.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return false;
	String about = "Erenik Engine Compressed mesh. Version:";
	String version = "1.0";

	about.WriteTo(file);
	version.WriteTo(file);
	this->name.WriteTo(file);
	this->source.WriteTo(file);

	// Write extra data so that they do not have to be re-calculated.?
	centerOfMesh.WriteTo(file);
	file.write((char*)&radius, sizeof(float));
	file.write((char*)&triangulated, sizeof(bool));

	// Write number of each specific array.
	file.write((char*)&numVertices, sizeof(int));
	file.write((char*)&numUVs, sizeof(int));
	file.write((char*)&numNormals, sizeof(int));
	file.write((char*)&numFaces, sizeof(int));
	
	// Then start writing the data of the arrays.
	int sizeOfUV = sizeof(Vector2f);

	int sizeOfVector3i = sizeof(Vector3i);
	int sizeOfVector3f = sizeof(Vector3f);
	int sizeOfVector3d = sizeof(Vector3d);

	int sizeOfP1 = sizeof(p1);
	int sizeOfP2 = sizeof(p2);
	int sizeOfP3 = sizeof(p3);


	int sizeOfVertex = sizeOfVector3f;

	Vector3f * vertexArrayPtr = vertices.GetArray();

	int size = numVertices * sizeOfVertex;

	// Write data.
	
	for (int i = 0; i < numVertices; ++i)
		vertices[i].WriteTo(file);
	for (int i = 0; i < numUVs; ++i)
		uvs[i].WriteTo(file);
	for (int i = 0; i < numNormals; ++i)
		normals[i].WriteTo(file);
	/*
	if (numVertices)
		file.write((char*)vertices.GetArray(), numVertices * sizeOfVertex);
	if (numUVs)
		file.write((char*)uvs.GetArray(), numUVs * sizeOfUV);
	if (numNormals)
		file.write((char*)normals.GetArray(), numNormals * sizeOfVertex);
//*/

	// Save all numFaces.
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace & mf = faces[i];
		mf.WriteTo(file);
	}
	std::cout<<"\nMesh saved in compressed form to file: "<<compressedPath;
	return true;
}
	
/// Load from customized compressed data form.
bool Mesh::LoadCompressedFrom(String compressedPath)
{
	std::fstream file;
	file.open(compressedPath.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
		return false;
	String about = "Erenik Engine Compressed mesh. Version:";
	String version = "1.0";

	about.ReadFrom(file);
	version.ReadFrom(file);
	this->name.ReadFrom(file);
	this->source.ReadFrom(file);
	
	assert(source.Length());
	assert(name.Length());


	// Write extra data so that they do not have to be re-calculated.?
	centerOfMesh.ReadFrom(file);
	file.read((char*)&radius, sizeof(float));
	file.read((char*)&triangulated, sizeof(bool));
	
	// Write number of each specific array.
	file.read((char*)&numVertices, sizeof(int));
	file.read((char*)&numUVs, sizeof(int));
	file.read((char*)&numNormals, sizeof(int));
	file.read((char*)&numFaces, sizeof(int));
	
	// Allocate arrays.
	AllocateArrays();


	// Then start writing the data of the arrays.
	int sizeOfVertex = sizeof(Vector3f);
	int sizeOfUV = sizeof(Vector2f);


	// Load 'em.


	int sizeOfP1 = sizeof(p1);
	int sizeOfP2 = sizeof(p2);
	int sizeOfP3 = sizeof(p3);

	Vector3f * vertexArrayPtr = vertices.GetArray();

	int size = numVertices * sizeOfVertex;



	// Read data.
	
	for (int i = 0; i < numVertices; ++i)
		vertices[i].ReadFrom(file);
	for (int i = 0; i < numUVs; ++i)
		uvs[i].ReadFrom(file);
	for (int i = 0; i < numNormals; ++i)
		normals[i].ReadFrom(file);

	/*
	if (numVertices)
		file.read((char*)vertices.GetArray(), numVertices * sizeOfVertex);
	if (numUVs)
		file.read((char*)uvs.GetArray(), numUVs * sizeOfUV);
	if (numNormals)
		file.read((char*)normals.GetArray(), numNormals * sizeOfVertex);
//*/

	// Load all numFaces.
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace & mf = faces[i];
		mf.ReadFrom(file);
//		mf.Print();	
	}
	loadedFromCompactObj = true;

	std::cout<<"\nCompressed mesh loaded from file: "<<compressedPath;
	return true;
}


/// Mostly for debug
void Mesh::PrintContents()
{
	assert(false);
	return;
	/*
    for (int i = 0; i < numFaces; ++i)
	{
        MeshFace * f = &faces[i];
        std::cout<<"\nMeshFace "<<i<<": numVertices("<<(int)f->numVertices<<"): ";
        for (int v = 0; v < f->numVertices; ++v){
            if (f->vertices)
             std::cout<<"v"<<f->vertices[v];
            if (f->uvs)
             std::cout<<"t"<<f->uvs[v];
            if (f->normals)
             std::cout<<"n"<<f->normals[v];
        }
    }
	*/
}

/// Replaces copy-constructor.
bool Mesh::LoadDataFrom(const Mesh * otherMesh)
{
    std::cout<<"\nLoadDataFrom mesh constructor begun...";

    Nullify();
//	return true;

	assert(otherMesh->numVertices);
	std::cout<<"\nMesh numVertices: "<<otherMesh->numVertices;
	name = otherMesh->name;
	source = otherMesh->source;
	numVertices = otherMesh->numVertices;
	numUVs = otherMesh->numUVs;
	numNormals = otherMesh->numNormals;
	numFaces = otherMesh->numFaces;
	loadedFromCompactObj = otherMesh->loadedFromCompactObj;

	std::cout<<"\nMesh numFaces: "<<otherMesh->numFaces;

	AllocateArrays();
	// Copy numVertices.
	for (int i = 0; i < numVertices; ++i)
	{
		vertices[i] = otherMesh->vertices[i];
	}
	// Copy numUVs.
	if (numUVs){
		for (int i = 0; i < numUVs; ++i){
			uvs[i] = otherMesh->uvs[i];
		}
	}
	// Copy numNormals.
	if (numNormals){
		for (int i = 0; i < numNormals; ++i){
			normals[i] = otherMesh->normals[i];
		}
	}
	// Copy numFaces.
	if (numFaces)
	{
	//	std::cout<<"\nMeshFaces: "<<numFaces;
    //    std::cout<<"\nOtherMeshFace: "<<&otherMesh->faces[0]<<" "<<otherMesh->faces[0].numVertices;

		for (int i = 0; i < numFaces; ++i){
		//    std::cout<<"\nCopying faces "<<i<<" ";
         //   faces[i].uvTangent = otherMesh->faces[i].uvTangent;
			faces[i] = otherMesh->faces[i];
		//	std::cout<<"v0: "<<faces[i].vertices[0]<<" Original: "<<otherMesh->faces[i].vertices;
		//	assert(faces[i].vertices != otherMesh->faces[i].vertices);
		//	if (faces[i].vertices[0] > 3000000)
         //       ; //Sleep(5000);
		//	assert(faces[i].vertices[0] < 3000000);
		}
	}
	return true;
}

/// For creating an optimized version of the more complex/pointer-oriented E(ditable)-Mesh.
bool Mesh::LoadDataFrom(const EMesh * otherMesh)
{
	std::cout<<"\nLoadDataFrom mesh constructor begun...";
    /// Deallocate as needed first?
	DeallocateArrays();
	/// Nullify stuff.
	Nullify();

	// Fetch numbers
	name = otherMesh->name;
	source = otherMesh->source;
	numVertices = otherMesh->vertices.Size();
	numUVs = otherMesh->uvs.Size();
	numNormals = otherMesh->normals.Size();
	numFaces = otherMesh->faces.Size();

	// If no normals, generate 1.
	bool generateNormals = false;
	if (numNormals == 0)
	{
		numNormals = 1;
		generateNormals = true;
	}

	// Allocate
	AllocateArrays();
	
	/// Extract all actual data.
	for (int i = 0; i < otherMesh->vertices.Size(); ++i)
	{
		Vector3f data = *(Vector3f*)otherMesh->vertices[i];
		vertices[i] = data;
	}
	// Load UVs
	for (int i = 0; i < otherMesh->uvs.Size(); ++i)
	{
		Vector2f data = *(Vector2f*)otherMesh->uvs[i];
		uvs[i] = data;
	}
	if (generateNormals)
	{
		normals[0] = Vector3f(0,1,0);
	}

	for (int i = 0; i < otherMesh->faces.Size(); ++i)
	{
		MeshFace & face = faces[i];
		EFace * eFace = otherMesh->faces[i];
		face.numVertices = eFace->vertices.Size();
		face.AllocateArrays();
		for (int j = 0; j < face.numVertices; ++j)
		{
			EVertex * vertex = eFace->vertices[j];
			// Now the bothersome part, fetch the indices of the vertices..
			int index = otherMesh->vertices.GetIndexOf(vertex);
			// Fetch UV if possible.
			EUV * uv = 0;
			uv = vertex->uvCoord;

			int uvIndex = 0;
			if (uv)
			{
				uvIndex = otherMesh->uvs.GetIndexOf(uv);
			}
			// .. and decrement it? or increment..? Nah. Should be 0-based!
			face.vertices[j] = index;
			face.normals[j] = 0;
			face.uvs[j] = uvIndex;
		}
	}
	
	return true;
}


/*
Mesh::Mesh(const Mesh * mesh){

}
*/

void Mesh::SetSource(String str){
	source = str;
}

void Mesh::SetName(String str){
	name = str;
}

#include <iostream>

/// Renders the meshi-mesh :3
void Mesh::Render(){

	int error = glGetError();

	// Check for valid buffer before rendering
	if (vboBuffer != 0 && vboBuffer < 3000000){

		assert(floatsPerVertex >= 8 && "Bad float-count per vertices, ne?!");

		// Set VBO and render

		// Bind numVertices
		glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);
		glVertexAttribPointer((GLuint) 0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, 0);		// Position
		glEnableVertexAttribArray(0);

		// Bind Normals
		static const GLint offsetN = 3 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer((GLuint) 2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetN);		// UVs
		glEnableVertexAttribArray(2);

		// Bind UVs
		static const GLint offsetU = 6 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer((GLuint) 1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetU);		// UVs
		glEnableVertexAttribArray(1);

		// Bind Tangents
		static const GLint offsetT = 8 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer((GLuint) 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetT);		// UVs
		glEnableVertexAttribArray(3);

		CheckGLError("Mesh::Render");
		int numVertices = vboVertexCount;

		// Render normally
		int glError = glGetError();
		glDrawArrays(GL_TRIANGLES, 0, vboVertexCount);        // Draw All Of The Triangles At Once
		glError = glGetError();
		if (glError != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: "<<glError<<" when rendering Mesh: ";
			if (name)
				std::cout<<name;
			else
				std::cout<<"Unnamed";
			Sleep(100);
		}

		// Unbind buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CheckGLError("Mesh::Render");
	}
	else if (vboBuffer > 3000000){
		std::cout<<"\nVBO buffer unreasonably high: "<<vboBuffer<<" Make sure it was buffered correctly?";
		assert(false && "VBO buffer unreasonably high");
	}
	else {
		std::cout<<"\nTrying to render mesh "<<name<<" with buffer "<<vboBuffer<<". Attempting to bufferize it.";
		this->Bufferize();
	}
}

/// Returns the relative path to the resource the mesh was loaded from.
String Mesh::RelativePath() {
	FilePath path(source);
	return path.RelativePath();
}

// Creates a sphere with specified amount of sections (both longitude and latitude).
void CreateSphere(Mesh &mesh, int sections){
	// Check that the mesh isn't already allocated and being used.
	if (mesh.numVertices)
		throw 1;

	// Give it an automized name based on the sections
	mesh.SetName("Sphere");
	mesh.SetSource("Generated.");

	float dSections = (float)sections;
	float halfSections = dSections / 2.0f;
	int vertexCount = (sections + 1) * (sections + 1);

	// Default to 0
	mesh.numUVs = mesh.numNormals = mesh.numVertices = 0;

	// Allocate arrays
	mesh.vertices.Allocate(vertexCount);
	mesh.uvs.Allocate(vertexCount);
	mesh.normals.Allocate(vertexCount);

	// For each row
	for (int i = 0; i < sections+1; ++i){
		// For each vertices in the row
		for (int j = 0; j < sections+1; ++j){


			int cIndex = i * (sections + 1) + j;
			//						Regular sinus for x				multiply with sine of row to get the relative size.
			mesh.vertices[cIndex].x = 1 * sin((j) / dSections * PI * 2) * sin((i) / dSections * PI);
			mesh.vertices[cIndex].y = 1 * cos((i) / dSections * PI);
			mesh.vertices[cIndex].z = 1 * cos((j) / dSections * PI * 2) * sin((i) / dSections * PI);

			mesh.uvs[cIndex].x = (j / (float)sections);
			mesh.uvs[cIndex].y = (1 - i / (float) sections);

			mesh.normals[cIndex] = Vector3f(mesh.vertices[cIndex].x, mesh.vertices[cIndex].y, mesh.vertices[cIndex].z).Normalize();

			++mesh.numVertices;
			++mesh.numUVs;
			++mesh.numNormals;
		}
	}


	// Triangulate straight away.
	mesh.faces.Allocate(sections * sections * 2);
	// Default to 0.
	mesh.numFaces = 0;
	// Index to start looking at for each row.
	int index = 0;
	// Create numFaces
	for (int i = 0; i < sections; ++i){
		for (int j = 0; j < sections; ++j){
			// 3 numVertices
			MeshFace * faces = &mesh.faces[mesh.numFaces];
			faces->numVertices = 3;
			// Allocate
			faces->AllocateArrays();
			int v;
			v = 0; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1 + sections;
			v = 1; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1 + sections + 1;
			v = 2; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1;
			++mesh.numFaces;

			// 3 numVertices
			faces = &mesh.faces[mesh.numFaces];
			faces->numVertices = 3;
			// Allocate
			faces->AllocateArrays();
			v = 0; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1;
			v = 1; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j;
			v = 2; faces->vertices[v] = faces->normals[v] = faces->uvs[v] = index + j + 1 + sections;
			++mesh.numFaces;

	/*		// Use one normals per faces so we can see each faces nao
			glNormal3f(vertices[index+j].x, vertices[index+j].y, vertices[index+j].z);

			// 1
			//	glNormal3f(vertices[index+j+1+sections].x, vertices[index+j+1+sections].y, vertices[index+j+1+sections].z);
			glTexCoord2f(j / (float)sections, 1 - (i+1) / (float) sections);
			glVector3f(vertices[index+j+1+sections].x, vertices[index+j+1+sections].y, vertices[index+j+1+sections].z);

			// 2
			//	glNormal3f(vertices[index+j+1+sections+1].x, vertices[index+j+1+sections+1].y, vertices[index+j+1+sections+1].z);
			glTexCoord2f((j+1) / (float)sections, 1 - (i+1) / (float) sections);
			glVector3f(vertices[index+j+1+sections+1].x, vertices[index+j+1+sections+1].y, vertices[index+j+1+sections+1].z);

			// 3
			//	glNormal3f(vertices[index+j+1].x, vertices[index+j+1].y, vertices[index+j+1].z);
			glTexCoord2f((j+1) / (float)sections, 1 - i / (float) sections);
			glVector3f(vertices[index+j+1].x, vertices[index+j+1].y, vertices[index+j+1].z);

			// 4
			glTexCoord2f(j / (float)sections, 1 - i / (float) sections);
			glVector3f(vertices[index+j].x, vertices[index+j].y, vertices[index+j].z);
			++mesh.numFaces;
			*/
		}
		// Increment sections + 1 since we're using extra numVertices for simplicities sake.
		index += sections + 1;
	}
}


/// Triangulates the mesh.
void Mesh::Triangulate()
{
	int newMeshFacesNeeded = numFaces;
    if (numFaces == 0)
        return;
	std::cout<<"\nMeshFaces: "<<numFaces;
	for (int i = 0; i < numFaces; ++i)
	{
	 //   std::cout<<"D:";
		MeshFace * currentMeshFace = &faces[i];
		if (currentMeshFace->numVertices > 3)
		{
			int MeshFacesToAdd = currentMeshFace->numVertices - 3;
		//	std::cout<<"\nMeshFaces to add in faces "<<i<<": "<<MeshFacesToAdd;
			newMeshFacesNeeded += currentMeshFace->numVertices - 3;
			if (MeshFacesToAdd > 20){
				std::cout<<"\nMeshFace vertices amount very large("<<(int)currentMeshFace->numVertices<<"), printing debug info:";
				assert(currentMeshFace->numVertices - 3 < 18);
				std::cout<<"\nAborting Triangularization since mesh data seems faulty!";
				return;
			}
		}
	}
	std::cout<<"\nNew numFaces needed:"<<newMeshFacesNeeded;

	// Already triangulated..
	if (newMeshFacesNeeded == faces.Size())
	{
		triangulated = true;
		return;
	}

	// Allocate new MeshFace array
	int MeshFaceSize = sizeof(faces);
	List<MeshFace> newMeshFaces;
	newMeshFaces.Allocate(newMeshFacesNeeded, true);
	int MeshFacesAddedSoFar = 0;
	for (int i = 0; i < numFaces; ++i){
		MeshFace * newTri = &newMeshFaces[MeshFacesAddedSoFar];
		MeshFace & oldMeshFace = faces[i];
		// Just copy
		if (oldMeshFace.numVertices == 3)
		{
			newTri->numVertices = 3;
		//	newTri->AllocateArrays();
			newTri->normals = oldMeshFace.normals;
			newTri->uvs = oldMeshFace.uvs;
			newTri->vertices = oldMeshFace.vertices;
			MeshFacesAddedSoFar++;
		}
		else {
			int numVertices = oldMeshFace.numVertices;
	//		std::cout<<"\nConverting old faces...";
	//		std::cout<<"\n";
	//		oldMeshFace->Print();
	//		std::cout<<"\nInto new numFaces...!";
			for (int j = 0; j < numVertices-2; ++j)
			{
				newTri = &newMeshFaces[MeshFacesAddedSoFar];
				/// Create new triangle for every extra vertices!
				newTri->numVertices = 3;
				newTri->AllocateArrays();
				newTri->normals[0] = oldMeshFace.normals[0];
				newTri->uvs[0] = oldMeshFace.uvs[0];
				newTri->vertices[0] = oldMeshFace.vertices[0];
				for (int k = j+1; k < j+3; ++k)
				{
					newTri->normals[k - j] = oldMeshFace.normals[k];
					newTri->uvs[k - j] = oldMeshFace.uvs[k];
					newTri->vertices[k - j] = oldMeshFace.vertices[k];
				}
				MeshFacesAddedSoFar++;
	//			std::cout<<"\n";
	//			newTri->Print();
			}
		}
	}
	faces.Clear();
	faces = newMeshFaces;
	numFaces = newMeshFacesNeeded;
	triangulated = true;
}

/// Deletes and recalculates ALL numNormals
void Mesh::RecalculateNormals()
{
	std::cout<<"\nRecalculating numNormals...";
	numNormals = numFaces;
	normals.Allocate(numNormals);
	/// For every faces...
	for (int i = 0; i < numFaces; ++i){
		assert(faces[i].numVertices >= 3);
		Vector3f line1 = vertices[faces[i].vertices[1]] - vertices[faces[i].vertices[0]];
		Vector3f line2 = vertices[faces[i].vertices[2]] - vertices[faces[i].vertices[0]];
		normals[i] = line1.CrossProduct(line2).NormalizedCopy();
		if (faces[i].normals.Size() == NULL)
			faces[i].normals.Allocate(faces[i].numVertices);
		for (int v = 0; v < faces[i].numVertices; ++v){
			faces[i].normals[v] = i;
		}
	}
}

/// Calculates radial and AABB boundaries.
void Mesh::CalculateBounds()
{
	if (!aabb)
		aabb = new AABB();

	if (!vertices.Size())
		return;

	Vector3f & min = aabb->min;
	Vector3f & max = aabb->max;
	min = vertices[0];
	max = vertices[0];
	radius = 0;
	float newRadius = 0;
	for (int i = 0; i < numVertices; ++i){
		newRadius = vertices[i].LengthSquared();
		if (newRadius > radius)
			radius = newRadius;

		// Get min
		if (vertices[i].x < min.x)
			min.x = vertices[i].x;
		if (vertices[i].y < min.y)
			min.y = vertices[i].y;
		if (vertices[i].z < min.z)
			min.z = vertices[i].z;
		// Get max
		if (vertices[i].x > max.x)
			max.x = vertices[i].x;
		if (vertices[i].y > max.y)
			max.y = vertices[i].y;
		if (vertices[i].z > max.z)
			max.z = vertices[i].z;
	}
	radius = sqrt(radius);
	centerOfMesh = (max - min)/2.0f + min;

	// Static radius on load (relative to 0,0,0)
	// Note that this method will absolutely guarantee a radius that is larger than needed.
	Vector3f extremePoints(
		abs(min.x) > max.x ? min.x : max.x,
		abs(min.y) > max.y ? min.y : max.y,
		abs(min.z) > max.z ? min.z : max.z
	);
//	radius = extremePoints.Length();


	// Below assumes centered mesh. This may not always be the case!
	/*
	for (int i = 0; i < numVertices; ++i){
		// Radius
		newRadius = (vertices[i] - centerOfMesh).Length();
		if (newRadius > radius)
			radius = newRadius;
	}*/

	/// Create AABB if needed?
	if (!aabb)
		aabb = new AABB(min, max);

}

/** Centerizes the model by pushing all numVertices by the length of the the centerOfMesh vector.
	Resets centerOfMesh afterward. */
void Mesh::Center(){
	for (int i = 0; i < numVertices; ++i){
		vertices[i] -= centerOfMesh;
	}
	centerOfMesh = Vector3f();
}

/// For NormalMapping~
void Mesh::CalculateUVTangents()
{
	// Not working anyway.. fit it.
	assert(false);
	/*
	if (!uvs)
		return;
	for (int i = 0; i < numFaces; ++i){
		MeshFace * f = &faces[i];
		if (f->numVertices < 3)
            continue;

		// Reference:
		// http://www.terathon.com/code/tangent.html

		/// 3 Points on the faces
		Vector3f v1, v2, v3;
		v1 = vertices[f->vertices[0]];
		v2 = vertices[f->vertices[1]];
		v3 = vertices[f->vertices[2]];

		float v1u, v2u, v3u, v1v, v2v, v3v;
		v1u = u[f->uvs[0]];
		v2u = u[f->uvs[1]];
		v3u = u[f->uvs[2]];
		v1v = v[f->uvs[0]];
		v2v = v[f->uvs[1]];
		v3v = v[f->uvs[2]];

		float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = v2u - v1u;
        float s2 = v3u - v1u;
        float t1 = v2v - v1v;
        float t2 = v3v - v1v;

		float r = 1.0F / (s1 * t2 - s2 * t1);
        Vector3d sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);
        Vector3d tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);

		// faces UV "up"- and "right"-tangent respectively.
		Vector3f tan1f = sdir;
		Vector3f tan2f = tdir;

		for (int j = 0; j < 3 && j < f->numVertices; ++j){
			// Only check the 3 first numVertices.
			const Vector3d& n = normals[f->normals[j]];
			const Vector3d& t = tan1f;

			Vector4d vertexTangent;
			// Gram-Schmidt orthogonalize
			vertexTangent = (t - n * n.DotProduct(t));
			vertexTangent.Normalize3();
			// Calculate handedness
			vertexTangent.w = (n.CrossProduct(t).DotProduct(tan2f) < 0.0F) ? -1.0F : 1.0F;

			/// Lazy first-try!
			f->uvTangent = vertexTangent;
			break;
		}
	}
	*/

}

/// Will bufferize the mesh. If force is true it will re-done even if it has already been bufferized once.
void Mesh::Bufferize(bool force)
{
	// Check if already buffered, stupid..
	if (this->vboBuffer){
//		std::cout<<"\nINFO: Mesh "<<name<<" already buffered, skipping!";
		return;
	}
	
	assert(source.Length());
	assert(name.Length());

	/// Always triangulate before buffering.
	if (!triangulated)
	{
		Triangulate();
	}
	

    if (GL_VERSION_MAJOR < 2 && GL_VERSION_MINOR < 5){
        std::cout<<"\nUnable to buffer mesh since GL version below is 1.5.";
        return;
    }

	/// Check for mesh-Entity
	if (!name)
		std::cout<<"\nBuffering un-named Mesh.";
	else
		std::cout<<"\nBuffering mesh \""<<name<<"\"";

	/// Create VAO
//	GLuint vertexArrayObject[1];

	// Check for errors before we proceed.
	GLuint err2 = glGetError();
	if (err2 != GL_NO_ERROR){
		std::cout<<"\nUnknown error?";
	}

	if (GL_VERSION_MAJOR >= 3){
		// Generate VAO and bind it straight away if we're above GLEW 3.0
	//	glGenVertexArrays(1, vertexArrayObject);
	//	glBindVertexArray(vertexArrayObject[0]);
	}

	// Check for errors before we proceed.
	GLuint err = glGetError();
	if (err != GL_NO_ERROR)
		std::cout<<"\nUnknown error?";

	// Count total vertices/texture point pairs without any further optimization.
	unsigned int vertexCount = numFaces * 3;
	floatsPerVertex = 3 + 3 + 2 + 4;  // Pos + Normal + UV + NormalMappingTangent
	float * vboVertexData = new float[vertexCount * floatsPerVertex];

    std::cout<<"\nVertexCount: "<<vertexCount<<" MeshFaceCount: "<<numFaces;

	// Generate And bind The Vertex Buffer
	/// Check that the buffer isn't already generated
	if (vboBuffer == 0)
	{
		vboBuffer = GLBuffers::New();
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);

	// Load all data in one big fat array, yo Data
	unsigned int vertexDataCounted = 0;
	// Reset vertices count
	vboVertexCount = 0;
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace * face = &faces[i];
//		face->Print();
		// Count numVertices in all triangles
		for (int j = 0; j < 3; ++j)
		{
		//	std::cout<<"\nCurrentVertex";
			int currentVertex = face->vertices[j];
			assert(currentVertex < 3000000 && currentVertex >= 0);
			// Position
			vboVertexData[vertexDataCounted + 0] = vertices[currentVertex].x;
			vboVertexData[vertexDataCounted + 1] = vertices[currentVertex].y;
			vboVertexData[vertexDataCounted + 2] = vertices[currentVertex].z;
			// Normal
			if (numNormals){
				int currentNormal = face->normals[j];
				vboVertexData[vertexDataCounted + 3] = normals[currentNormal].x;
				vboVertexData[vertexDataCounted + 4] = normals[currentNormal].y;
				vboVertexData[vertexDataCounted + 5] = normals[currentNormal].z;
			}
			else
				vboVertexData[vertexDataCounted + 3] =
				vboVertexData[vertexDataCounted + 4] =
				vboVertexData[vertexDataCounted + 5] = 0;
			// UV
			if (numUVs){
				int currentUV = face->uvs[j];
				vboVertexData[vertexDataCounted + 6] = uvs[currentUV].x;
				vboVertexData[vertexDataCounted + 7] = uvs[currentUV].y;
			}
			else {
				vboVertexData[vertexDataCounted + 6] =
				vboVertexData[vertexDataCounted + 7] = 0.0f;
			}
			/// Tangents for NormalMapping
			if (true) {
				vboVertexData[vertexDataCounted + 8] = face->uvTangent.x;
				vboVertexData[vertexDataCounted + 9] = face->uvTangent.y;
				vboVertexData[vertexDataCounted + 10] = face->uvTangent.z;
				vboVertexData[vertexDataCounted + 11] = face->uvTangent.w;
			}
			else {
				vboVertexData[vertexDataCounted + 8] =
				vboVertexData[vertexDataCounted + 9] =
				vboVertexData[vertexDataCounted + 10] =
				vboVertexData[vertexDataCounted + 11] = 0;
			}
			vertexDataCounted += floatsPerVertex;
			++vboVertexCount;
		}
	}
	if ((unsigned int)vboVertexCount >= vertexCount * floatsPerVertex)
		throw 3;

	// Enter the data too, fucking moron Emil-desu, yo!
	glBufferData(GL_ARRAY_BUFFER, vertexDataCounted*sizeof(float), vboVertexData, GL_STATIC_DRAW);

	// Check for errors before we proceed.
	GLuint error = glGetError();
	if (error != GL_NO_ERROR)
		throw 6;
	delete[] vboVertexData;
	vboVertexData = NULL;
}

/// Searches through the mesh's numVertices and returns the maximum bounding radius required by the Entity.
float getMaxBoundingRadius(Mesh * mesh){
/*	if (mesh == NULL){
		std::cout<<"WARNING: Mesh NULL in getMaxBoundingRadius!";
		return 0;
	}
	*/
	float maxX = 0.0f, maxY = 0.0f, maxZ = 0.0f;
	for (int i = 0; i < mesh->numVertices; ++i){
		if (AbsoluteValue(mesh->vertices[i].x) > maxX)
			maxX = mesh->vertices[i].x;
		if (AbsoluteValue(mesh->vertices[i].y) > maxY)
			maxY = mesh->vertices[i].y;
		if (AbsoluteValue(mesh->vertices[i].z) > maxZ)
			maxZ = mesh->vertices[i].z;
	}
	float max = sqrt(pow(maxX, 2) + pow(maxY, 2) + pow(maxZ, 2));
	return max;
}

List<Triangle> Mesh::GetTris(){
	List<Triangle> triangles;
	for (int i = 0; i < numFaces; ++i){
		// Just copy shit from it
		MeshFace * f = &faces[i];
		assert((f->numVertices <= 4 || f->numVertices >= 3) && "Bad vertices count in faces");

		Vector3f p1 = vertices[f->vertices[0]],
			p2 = vertices[f->vertices[1]],
			p3 = vertices[f->vertices[2]];

		if (f->numVertices == 4){
			assert(false && "faces has quads!");
// 			Vector3f p4 = vertices[faces->vertices[3]];
// 			Quad * quad = new Quad();
// 			quad->Set4Points(p1, p2, p3, p4);
// 			quads.Add(quad);
		}
		else if (f->numVertices == 3){
			Triangle tri;
			tri.Set3Points(p1, p2, p3);
			triangles.Add(tri);
		}
	}
	assert(triangles.Size() > 0);
	return triangles;
}
