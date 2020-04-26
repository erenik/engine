// Emil Hedemalm
// 2013-03-20

//#include <windows.h>
#include "Mesh/Mesh.h"

#include "Mesh/EFace.h"
#include "Mesh/EMesh.h"
#include "Mesh/EVertex.h"

#include "Graphics/OpenGL.h"
#include "Graphics/GLBuffers.h"
#include "Graphics/GraphicsManager.h"
//#include "GraphicsState.h"

#include "Material.h"
#include "OS/Sleep.h"

#include <fstream>
#include <iostream>
#include <Util.h>

#include "Matrix/Matrix.h"

#include "PhysicsLib/Shapes/Triangle.h"
#include "PhysicsLib/Shapes/AABB.h"

#include "GraphicsState.h"
#include "Graphics/Shader.h"

#include "Model/SkeletalAnimationNode.h"
#include "TextureManager.h"

#include "File/LogFile.h"

Mesh::Mesh()
{
	Nullify();
}

	/// Nullifies all default variables!
void Mesh::Nullify()
{
	numVertices = numUVs = numFaces = numNormals = 0;
	textureID = 0;
	floatsPerVertex = 0;
	triangulated = false;
	loadedFromCompactObj = false;
	aabb = NULL;
	radiusOrigo = 0;
	radiusCentered = 0;

	skeleton = NULL;

	// Standard position/normal/uv/tanget buffer.
	vertexBuffer = -1;
	// Bone index/weights buffer.
	boneIndexBuffer = boneWeightsBuffer = -1;

	/// Raw buffer data before sending into GL.
	vertexData = NULL;
	vertexDataSize = 0;

	boneIndexData = NULL;
	boneWeightData = NULL;
}

Mesh::~Mesh()
{
	/*
	// Deallocate the lists.
	vertices.Deallocate();
	uvs.Deallocate();
	normals.Deallocate();
	faces.Deallocate();
	*/
	if (aabb)
		delete aabb;

	// Delete buffer data arrays if any remain.
	if (vertexData)
		delete[] vertexData;

	if (boneIndexData)
		delete[] boneIndexData;
	if (boneWeightData)
		delete[] boneWeightData;
}

/// Yus.
bool Mesh::SaveObj(String path)
{
	if (!path.Contains(".obj"))
		path = path + ".obj";
	File objFile(path);
	bool ok = objFile.OpenForWritingText();
	if (!ok)
		return false;
	std::fstream & fstream = objFile.GetStream();
	fstream<<"o "<<name;
	for (int i = 0; i < vertices.Size(); ++i)
	{
		Vector3f & v = vertices[i];
		fstream<<"\nv "<<v;
	}
	for (int i = 0; i < uvs.Size(); ++i)
	{
		Vector2f & uv = uvs[i];
		fstream<<"\nvt "<<uv;
	}
	for (int i = 0; i < normals.Size(); ++i)
	{
		Vector3f & n = normals[i];
		fstream<<"\nvn "<<n;
	}
	for (int i = 0; i < faces.Size(); ++i)
	{
		MeshFace & f = faces[i];
		fstream<<"\nf";
		for (int i = 0; i < f.vertices.Size(); ++i)
		{
			fstream<<" "<<(f.vertices[i]+1);
			if (f.normals.Size() || f.uvs.Size())
				fstream<<"/";
			if (f.uvs.Size())
				fstream<<(f.uvs[i]+1);
			if (f.normals.Size())
				fstream<<"/"<<(f.normals[i]+1);
		}
	}
	objFile.Close();
	return true;
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
/*
void Mesh::AddPlane(ConstVec3fr upperLeft, ConstVec3fr lowerLeft, ConstVec3fr lowerRight, ConstVec3fr upperRight)
{
	
}

/// Adds a grid (basically a plane), with the specified amount of cells/faces in X and Y.
void Mesh::AddGrid(Vector3f upperLeft, Vector3f lowerLeft, Vector3f lowerRight, Vector3f upperRight, Vector2i gridSizeDivision)
{
	assert(false);
}
*/


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

#define MESH_CURRENT_VERSION 2

/// Load from customized compressed data form. Returns true upon success.
bool Mesh::SaveCompressedTo(String compressedPath)
{
	std::fstream file;
	file.open(compressedPath.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return false;
	String about = "Erenik Engine Compressed mesh.";
	int version = MESH_CURRENT_VERSION;

	about.WriteTo(file);
	file.write((char*)&version, sizeof(int));
	this->name.WriteTo(file);
	this->source.WriteTo(file);

	// Write extra data so that they do not have to be re-calculated.?
	centerOfMesh.WriteTo(file);
	file.write((char*)&radiusOrigo, sizeof(float));
	file.write((char*)&triangulated, sizeof(bool));
	// Write AABB data so that it is pre-loaded.
	assert(aabb && "No aabb when trying to save compressed mesh. Calculate it NOW!");
	if (!aabb)
		return false;
	assert(aabb->scale.MaxPart());
	aabb->WriteTo(file);

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
	String about;
	int version;
	about.ReadFrom(file);
	file.read((char*)&version, sizeof(int));
	if (version != MESH_CURRENT_VERSION)
		return false;
	this->name.ReadFrom(file);
	this->source.ReadFrom(file);
	
	assert(source.Length());
	assert(name.Length());

	// Write extra data so that they do not have to be re-calculated.?
	centerOfMesh.ReadFrom(file);
	file.read((char*)&radiusOrigo, sizeof(float));
	file.read((char*)&triangulated, sizeof(bool));
	// Write AABB data so that it is pre-loaded.
	if (!aabb) 
		aabb = new AABB();
	aabb->ReadFrom(file);
	
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

	// Load all numFaces.
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace & mf = faces[i];
		mf.ReadFrom(file);
//		mf.Print();	
	}
	
	this->CalculateUVTangents();

	loadedFromCompactObj = true;
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
bool Mesh::LoadDataFrom(const Mesh * otherMesh, bool nullify /*= false*/)
{
	// std::cout<<"\nLoadDataFrom mesh constructor begun...";
	if (!aabb)
		aabb = new AABB();
	assert(otherMesh->aabb);
	*aabb = *otherMesh->aabb;
	// Update update time.
	lastUpdate = otherMesh->lastUpdate;

	// Why.. nullify.... _T_
	if (nullify)
	    Nullify();
//	return true;

	assert(otherMesh->numVertices);
//	std::cout<<"\nMesh numVertices: "<<otherMesh->numVertices;
	name = otherMesh->name;
	source = otherMesh->source;
	numVertices = otherMesh->numVertices;
	numUVs = otherMesh->numUVs;
	numNormals = otherMesh->numNormals;
	numFaces = otherMesh->numFaces;
	loadedFromCompactObj = otherMesh->loadedFromCompactObj;
	this->skeleton = otherMesh->skeleton;
	vertexWeights = otherMesh->vertexWeights;
	
	vertices = otherMesh->vertices;
	originalVertexPositions = otherMesh->originalVertexPositions;
	uvs = otherMesh->uvs;
	normals = otherMesh->normals;
	faces = otherMesh->faces;
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
	for (int i = 0; i < otherMesh->normals.Size(); ++i)
	{
		Vector3f data = *(Vector3f*)otherMesh->normals[i];
		normals[i] = data;
	}
	if (generateNormals)
	{
		numNormals = 1;
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
			int normalIndex = 0;
			ENormal * normal = eFace->normal;
			if (normal)
				normalIndex = otherMesh->normals.GetIndexOf(normal);
			face.normals[j] = normalIndex;
			face.uvs[j] = uvIndex;
		}
	}
	std::cout<<" loaded.";
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
void Mesh::Render(GraphicsState & graphicsState)
{
	assert(vertexBuffer != -1);
//	LogGraphics("Mesh::Render", EXTENSIVE_DEBUG);
	Shader * shader = ActiveShader();
	// Check for valid buffer before rendering
	if (graphicsState.BoundVertexArrayBuffer() != vertexBuffer)
	{
		assert(floatsPerVertex >= 8 && "Bad float-count per vertices, ne?!");
		// Set VBO and render
		if (logLevel <= DEBUG)
			CheckGLError("Mesh::Render - before binding stuff");
		BindVertexBuffer(graphicsState);
		if (logLevel <= DEBUG)
			CheckGLError("Mesh::Render - binding stuff");
	}

	int numVertices = vertexDataCount;
	// Render normally
	int glError = glGetError();
//	LogGraphics("Mesh::Render - glDrawArrays pre", DEBUG);
	glDrawArrays(GL_TRIANGLES, 0, vertexDataCount);        // Draw All Of The Triangles At Once
//	LogGraphics("Mesh::Render - glDrawArrays post", DEBUG);
	if (logLevel <= DEBUG)
		CheckGLError("Mesh::Render - glDrawArrays");
}

void Mesh::BindVertexBuffer(GraphicsState & graphicsState)
{
//	LogGraphics("Mesh::BindVertexBuffer", EXTENSIVE_DEBUG);
	// Bind the vertex buffer.
	graphicsState.BindVertexArrayBuffer(vertexBuffer);
	Shader * shader = ActiveShader();
	assert(shader);

	glVertexAttribPointer(shader->attributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, 0);		// Position
	
	// Bind Normals
	static const GLint offsetN = 3 * sizeof(GLfloat);		// Buffer already bound once at start!
	if (shader->attributeNormal != -1)
		glVertexAttribPointer(shader->attributeNormal, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetN);		// Normals
	
	// Bind UVs
	static const GLint offsetU = 6 * sizeof(GLfloat);		// Buffer already bound once at start!
	if (shader->attributeUV != -1)
		glVertexAttribPointer(shader->attributeUV, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetU);		// UVs
	
	// Bind Tangents
	static const GLint offsetT = 8 * sizeof(GLfloat);		// Buffer already bound once at start!
	if (shader->attributeTangent != -1)
		glVertexAttribPointer(shader->attributeTangent, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetT);		// Tangents

	// Bind BiTangents
	static const GLint offsetB = 11 * sizeof(GLfloat);		// Buffer already bound once at start!
	if (shader->attributeBiTangent != -1)
		glVertexAttribPointer(shader->attributeBiTangent, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * floatsPerVertex, (void *)offsetB);		// Tangents
}

/// Re-skins the mesh's vertices to match the current skeletal animation. Every single vertex will be re-calculated and then re-buffered.
void Mesh::SkinToCurrentSkeletalAnimation()
{
	/// Save original vertices if not already done so.
	if (originalVertexPositions.Size() != vertices.Size())
		originalVertexPositions = vertices;
	
	for (int i = 0; i < vertices.Size(); ++i)
	{
		Vector3f & vertexPosition = vertices[i];
		vertexPosition = Vector3f();
		Vector3f & originalVertexPosition = originalVertexPositions[i];
		List<VertexWeight> & vertex_weights = vertexWeights[i];
		for (int i = 0; i < vertex_weights.Size(); ++i)
		{
			/**	The skinning calculation for each vertex v in a bind shape is as follows.

				for (int i = 0; i < numVertices; ++i)
					skinnedVertexPosition += {[(vertexPosition * BSM) * IBMi * JMi] * JW}

				• n: The number of joints that influence vertex v
				• BSM: Bind-shape matrix
				• IBMi: Inverse bind-pose matrix of joint i
				• JMi: Transformation matrix of joint i
				• JW: Weight of the influence of joint i on vertex v
			*/
			int boneIndex = vertex_weights[i].boneIndex;
			Matrix4f & inverseBindMatrix = invBindPoseMatrices[boneIndex];
			Bone * bone = skeleton->GetBoneByIndex(boneIndex);
			assert(bone);
			float & weight = vertex_weights[i].weight;
			
			Matrix4f skinningMatrix = bone->nodeModelMatrix * inverseBindMatrix;
			Vector3f attempt2 = skinningMatrix.Product(originalVertexPosition) * weight;
			Vector3f skinning2 = bone->nodeModelMatrix.Product(inverseBindMatrix.Product(bindShapeMatrix.Product(originalVertexPosition))) * weight;
			vertexPosition += skinning2;
		}
	}
	this->lastUpdate = Time::Now();
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
			mesh.vertices[cIndex][0] = 1 * sin((j) / dSections * PI * 2) * sin((i) / dSections * PI);
			mesh.vertices[cIndex][1] = 1 * cos((i) / dSections * PI);
			mesh.vertices[cIndex][2] = 1 * cos((j) / dSections * PI * 2) * sin((i) / dSections * PI);

			mesh.uvs[cIndex][0] = (j / (float)sections);
			mesh.uvs[cIndex][1] = (1 - i / (float) sections);

			mesh.normals[cIndex] = Vector3f(mesh.vertices[cIndex][0], mesh.vertices[cIndex][1], mesh.vertices[cIndex][2]).NormalizedCopy();

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
			glNormal3f(vertices[index+j][0], vertices[index+j][1], vertices[index+j][2]);

			// 1
			//	glNormal3f(vertices[index+j+1+sections][0], vertices[index+j+1+sections][1], vertices[index+j+1+sections][2]);
			glTexCoord2f(j / (float)sections, 1 - (i+1) / (float) sections);
			glVector3f(vertices[index+j+1+sections][0], vertices[index+j+1+sections][1], vertices[index+j+1+sections][2]);

			// 2
			//	glNormal3f(vertices[index+j+1+sections+1][0], vertices[index+j+1+sections+1][1], vertices[index+j+1+sections+1][2]);
			glTexCoord2f((j+1) / (float)sections, 1 - (i+1) / (float) sections);
			glVector3f(vertices[index+j+1+sections+1][0], vertices[index+j+1+sections+1][1], vertices[index+j+1+sections+1][2]);

			// 3
			//	glNormal3f(vertices[index+j+1][0], vertices[index+j+1][1], vertices[index+j+1][2]);
			glTexCoord2f((j+1) / (float)sections, 1 - i / (float) sections);
			glVector3f(vertices[index+j+1][0], vertices[index+j+1][1], vertices[index+j+1][2]);

			// 4
			glTexCoord2f(j / (float)sections, 1 - i / (float) sections);
			glVector3f(vertices[index+j][0], vertices[index+j][1], vertices[index+j][2]);
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
//	std::cout<<"\nMeshFaces: "<<numFaces;
	for (int i = 0; i < numFaces; ++i)
	{
	 //   std::cout<<"D:";
		MeshFace * currentMeshFace = &faces[i];
		if (currentMeshFace->numVertices > 3)
		{
			int meshFacesToAdd = currentMeshFace->numVertices - 3;
		//	std::cout<<"\nMeshFaces to add in faces "<<i<<": "<<MeshFacesToAdd;
			newMeshFacesNeeded += currentMeshFace->numVertices - 3;
			if (meshFacesToAdd > 20){
				std::cout<<"\nMeshFace vertices amount very large("<<(int)currentMeshFace->numVertices<<"), printing debug info:";
				assert(currentMeshFace->numVertices - 3 < 18);
				std::cout<<"\nAborting Triangularization since mesh data seems faulty!";
				return;
			}
		}
	}
//	std::cout<<"\nNew numFaces needed:"<<newMeshFacesNeeded;

	// Already triangulated..
	if (newMeshFacesNeeded == faces.Size())
	{
		triangulated = true;
		return;
	}

	// Allocate new MeshFace array
	int meshFaceSize = sizeof(faces);
	List<MeshFace> newMeshFaces;
	newMeshFaces.Allocate(newMeshFacesNeeded, true);
	int meshFacesAddedSoFar = 0;
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace * newTri = &newMeshFaces[meshFacesAddedSoFar];
		MeshFace & oldMeshFace = faces[i];
		// Just copy
		if (oldMeshFace.numVertices == 3)
		{
			newTri->numVertices = 3;
		//	newTri->AllocateArrays();
			newTri->normals = oldMeshFace.normals;
			newTri->uvs = oldMeshFace.uvs;
			newTri->vertices = oldMeshFace.vertices;
			meshFacesAddedSoFar++;
		}
		else {
			int numVertices = oldMeshFace.numVertices;
	//		std::cout<<"\nConverting old faces...";
	//		std::cout<<"\n";
	//		oldMeshFace->Print();
	//		std::cout<<"\nInto new numFaces...!";
			for (int j = 0; j < numVertices-2; ++j)
			{
				newTri = &newMeshFaces[meshFacesAddedSoFar];
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
				meshFacesAddedSoFar++;
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
	radiusOrigo = 0;
	float newRadius = 0;
	for (int i = 1; i < numVertices; ++i)
	{
		newRadius = vertices[i].LengthSquared();
		if (newRadius > radiusOrigo)
			radiusOrigo = newRadius;
		// Get min
		min = Vector3f::Minimum(min, vertices[i]);
		// Get max
		max = Vector3f::Maximum(max, vertices[i]);
	}
	// Update related values.
	radiusOrigo = sqrt(radiusOrigo); // why sqrt? since LengthSquared above.
	centerOfMesh = (max - min)/2.0f + min;
	// Update more AABB values.
	aabb->scale = max - min;
	aabb->position = (max + min) * 0.5;
}

/** Centerizes the model by pushing all numVertices by the length of the the centerOfMesh vector.
	Resets centerOfMesh afterward. */
void Mesh::Center()
{
	for (int i = 0; i < numVertices; ++i){
		vertices[i] -= centerOfMesh;
	}
	centerOfMesh = Vector3f();
}

/// For NormalMapping~
void Mesh::CalculateUVTangents()
{
	bool defaultCalcUVTangents = false;
	if (!defaultCalcUVTangents)
		return; 
	// Not working anyway.. fit it.
	if (this->uvs.Size() == 0)
	{
		std::cout<<"\nNo UVs found. Skipping.";
		return;
	}
	for (int i = 0; i < faces.Size(); ++i)
	{
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

		Vector2f uv1, uv2, uv3;
		uv1 = uvs[f->uvs[0]];
		uv2 = uvs[f->uvs[1]];
		uv3 = uvs[f->uvs[2]];

		/// Diff-vectors.
		Vector3f v1ToV2 = v2 - v1,
			v1ToV3 = v3 - v1;
		Vector2f uv1ToUV2 = uv2 - uv1,
			uv1ToUV3 = uv3 - uv1;

		Vector2f deltaUV1 = uv1ToUV2,
			deltaUV2 = uv1ToUV3;
		Vector3f deltaPos1 = v1ToV2;
		Vector3f deltaPos2 = v1ToV3;
	  // Edges of the triangle : postion delta
 		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		Vector3f tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
		Vector3f biTangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

		/// Check handed-ness.
		Vector3f crossProduct =  normals[f->normals[0]].CrossProduct(tangent);
		float dotProduct = crossProduct.DotProduct(biTangent);
//		std::cout<<"\nHandednesS: "<<dotProduct > 0;
		if (dotProduct < 0)
			tangent *= -1;

		/// Add the tanget and bitangnet
		f->uvTangent = tangent;
		f->uvBiTangent = biTangent;


		/// Help http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/		
	}
}

/// Will bufferize the mesh. If force is true it will re-done even if it has already been bufferized once.
void Mesh::Bufferize(bool force /* = false */)
{
	// Check if already buffered, stupid..
	if (this->vertexBuffer != -1 && !force // && originalPositionsBuffered == useOriginalPositions
		)
	{
//		std::cout<<"\nINFO: Mesh "<<name<<" already buffered, skipping!";
		return;
	}
	LogGraphics("Mesh::Bufferize", EXTENSIVE_DEBUG);

	/// Used for skeletal, re-examine later.
//	originalPositionsBuffered = useOriginalPositions;
	
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
		; // std::cout<<"\nBuffering mesh \""<<name<<"\"";

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
	// Pos (3) + Normal (3) + UV (2) + UV-Tangent (3) + UV-Bi-Tangent (3)
	floatsPerVertex = 3 + 3 + 2 + 3 + 3;  
	int vertexDataSizeNeeded = vertexCount * floatsPerVertex;
	if (vertexDataSize != vertexDataSizeNeeded)
	{
		if (vertexData)
			delete[] vertexData;
		vertexData = new float[vertexDataSizeNeeded];
		vertexDataSize = vertexDataSizeNeeded;
	}

	// Create buffer for bone-index and weights data.
	int bonesPerVertex = 4;
//	int * boneIndexData = new int[vertexCount * bonesPerVertex];
//	float * boneWeightsData = new float[vertexCount * bonesPerVertex];

 //   std::cout<<"\nVertexCount: "<<vertexCount<<" MeshFaceCount: "<<numFaces;

	// Generate And bind The Vertex Buffer
	/// Check that the buffer isn't already generated
	if (vertexBuffer == -1)
	{
		vertexBuffer = GLBuffers::New();
	}
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	// Load all data in one big fat array, yo Data
	unsigned int vertexDataCounted = 0;
	// Reset vertices count
	vertexDataCount = 0;
	int numBoneIndices = 0, numBoneWeights = 0;
	// Bone index data size. Also used for bone weight data.
	int boneIndexDataSize = faces.Size() * 3 * 4;
	if (boneIndexData == 0)
	{
		boneIndexData = new int[boneIndexDataSize];
		boneWeightData = new float[boneIndexDataSize];
	}

	List<Vector3f> * vertexPositionData = &vertices;
	assert(vertexPositionData->Size() == vertices.Size());
	/// Used for skeletal, re-examine later.
/*
	if (useOriginalPositions && originalVertexPositions.Size())
	{
		vertexPositionData = &originalVertexPositions;
	}
	*/
	for (int i = 0; i < numFaces; ++i)
	{
		MeshFace * face = &faces[i];
//		face->Print();
		// Count numVertices in all triangles
		for (int j = 0; j < 3; ++j)
		{
			// Add standard vertex data.
			int currentVertex = face->vertices[j];
			assert(currentVertex < 3000000 && currentVertex >= 0);
			// Position
			vertexData[vertexDataCounted + 0] = (*vertexPositionData)[currentVertex][0];
			vertexData[vertexDataCounted + 1] = (*vertexPositionData)[currentVertex][1];
			vertexData[vertexDataCounted + 2] = (*vertexPositionData)[currentVertex][2];
			// Normal
			if (numNormals)
			{
				int currentNormal = face->normals[j];
				vertexData[vertexDataCounted + 3] = normals[currentNormal][0];
				vertexData[vertexDataCounted + 4] = normals[currentNormal][1];
				vertexData[vertexDataCounted + 5] = normals[currentNormal][2];
			}
			else
			{
				vertexData[vertexDataCounted + 3] =
				vertexData[vertexDataCounted + 4] =
				vertexData[vertexDataCounted + 5] = 0;
			}
			// UV
			if (numUVs)
			{
				int currentUV = face->uvs[j];
				vertexData[vertexDataCounted + 6] = uvs[currentUV][0];
				vertexData[vertexDataCounted + 7] = uvs[currentUV][1];
			}
			else 
			{
				vertexData[vertexDataCounted + 6] =
				vertexData[vertexDataCounted + 7] = 0.0f;
			}
			/// UV-Tangents
			if (true) 
			{
				vertexData[vertexDataCounted + 8] = face->uvTangent[0];
				vertexData[vertexDataCounted + 9] = face->uvTangent[1];
				vertexData[vertexDataCounted + 10] = face->uvBiTangent.x;
			}
			/// UV-Bi-Tangents
			if (true) 
			{
				vertexData[vertexDataCounted + 11] = face->uvBiTangent.x;
				vertexData[vertexDataCounted + 12] = face->uvBiTangent.y;
				vertexData[vertexDataCounted + 13] = face->uvBiTangent.z;
			}
			vertexDataCounted += floatsPerVertex;
			++vertexDataCount;
			
			// Add skeletal data as needed
			if (skeleton)
			{
				assert(vertexWeights.Size() == vertices.Size());
				List<VertexWeight> & vertex_weights = vertexWeights[currentVertex];
				for (int w = 0; w < 4; ++w)
				{
					if (w < vertex_weights.Size())
					{
						VertexWeight & weight = vertex_weights[w];
						boneIndexData[numBoneIndices] = weight.boneIndex;
						boneWeightData[numBoneIndices] = weight.weight;
					}
					else {
						boneIndexData[numBoneIndices] = -1;
						boneWeightData[numBoneIndices] = 0;
					}
					++numBoneIndices;
				}
			}
			
			/*
			
			/// Save original vertices if not already done so.
			if (originalVertexPositions.Size() != vertices.Size())
				originalVertexPositions = vertices;
	
				for (int i = 0; i < vertices.Size(); ++i)
				{
					Vector3f & vertexPosition = vertices[i];
					vertexPosition = Vector3f();
					Vector3f & originalVertexPosition = originalVertexPositions[i];
					List<VertexWeight> & vertex_weights = vertexWeights[i];
					for (int i = 0; i < vertex_weights.Size(); ++i)
					{
						//	The skinning calculation for each vertex v in a bind shape is as follows.

						//	for (int i = 0; i < numVertices; ++i)
						//		skinnedVertexPosition += {[(vertexPosition * BSM) * IBMi * JMi] * JW}

						//	• n: The number of joints that influence vertex v
						//	• BSM: Bind-shape matrix
						//	• IBMi: Inverse bind-pose matrix of joint i
						//	• JMi: Transformation matrix of joint i
						//	• JW: Weight of the influence of joint i on vertex v
						//
						int boneIndex = vertex_weights[i].boneIndex;
						Matrix4f & inverseBindMatrix = invBindPoseMatrices[boneIndex];
						Bone * bone = skeleton->GetBoneByIndex(boneIndex);
						assert(bone);
						Matrix4f & skinningMatrix = inverseBindMatrix * bone->nodeModelMatrix;
						float & weight = vertex_weights[i].weight;
			
						Vector3f previousAttempt = skinningMatrix.Product(originalVertexPosition) * weight;
						Vector3f skinning2 = bone->nodeModelMatrix.Product(inverseBindMatrix.Product(bindShapeMatrix.Product(originalVertexPosition))) * weight;
						vertexPosition += skinning2;
					}
				}
				this->lastUpdate = Time::Now();
			*/


		}
	}
	if ((unsigned int)vertexDataCount >= vertexCount * floatsPerVertex)
		throw 3;

	// Enter the data too, fucking moron Emil-desu, yo!
	glBufferData(GL_ARRAY_BUFFER, vertexDataCounted*sizeof(float), vertexData, GL_STATIC_DRAW);

	// Save buffer-time.
	lastBufferization = Time::Now();

	// Check for errors before we proceed.
	GLuint error = glGetError();
	if (error != GL_NO_ERROR)
		throw 6;

	// Buffer bone indices
	if (skeleton)
	{
		if (boneIndexBuffer == -1)
			boneIndexBuffer = GLBuffers::New();
		glBindBuffer(GL_ARRAY_BUFFER, boneIndexBuffer);
		glBufferData(GL_ARRAY_BUFFER, boneIndexDataSize * sizeof(int), boneIndexData, GL_STATIC_DRAW);

		// Buffer bone weights
		if (boneWeightsBuffer == -1)
			boneWeightsBuffer = GLBuffers::New();
		glBindBuffer(GL_ARRAY_BUFFER, boneWeightsBuffer);
		glBufferData(GL_ARRAY_BUFFER, boneIndexDataSize * sizeof(float), boneWeightData, GL_STATIC_DRAW);

		/*
		std::cout<<"\nBone buffer data. ";
		for (int i = 0; i < boneIndexDataSize; ++i)
		{
			std::cout<<"\nBone i: "<<boneIndexData[i]<<" w: "<<boneWeightData[i];
			if ((i + 1) % 4 == 0)
				std::cout<<"\n";
		}
		std::cout<<"\nBone buffer data.. ";
		*/
	}
	// Deletion moved to destructor!
//	delete[] vertexData;
//	vertexData = NULL;

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
		if (AbsoluteValue(mesh->vertices[i][0]) > maxX)
			maxX = mesh->vertices[i][0];
		if (AbsoluteValue(mesh->vertices[i][1]) > maxY)
			maxY = mesh->vertices[i][1];
		if (AbsoluteValue(mesh->vertices[i][2]) > maxZ)
			maxZ = mesh->vertices[i][2];
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
