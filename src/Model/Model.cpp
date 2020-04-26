/// Emil Hedemalm
/// 2014-07-27 (header added recently, original was much older)
/// A complete model, which may have multiple mesh-parts.

#include "Model/Model.h"
#include "Mesh/Mesh.h"

#include <cstdlib>
#include <cstring>

#include "PhysicsLib/Shapes/AABB.h"
#include "PhysicsLib/Shapes/Triangle.h"

#include "Graphics/GLBuffers.h"

#include "Model/SkeletalAnimationNode.h"

char * Model::defaultMesh = NULL;
Texture * Model::defaultTexture = NULL;


Model::Model()
{
	users = 0;
	triangleList = NULL;

	mesh = NULL;
	triangulatedMesh = NULL;

	boneSkinningMatrixMap = -1;
	boneSkinningMatrixBuffer = NULL;
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
	
	if (triangulatedMesh == mesh)
		triangulatedMesh = 0;
	if (mesh)
		delete mesh;
	if (triangulatedMesh)
		delete triangulatedMesh;

	if (boneSkinningMatrixBuffer)
	{
		delete[] boneSkinningMatrixBuffer;
		boneSkinningMatrixBuffer = NULL;
	}


	mesh = triangulatedMesh = NULL;
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

void Model::BufferizeIfNeeded()
{
	Mesh * triangulated = this->GetTriangulatedMesh();
	if (triangulated->vertexBuffer == -1)
		triangulated->Bufferize();
}


/// Calls render on the triangulized mesh parts within.
void Model::Render(GraphicsState & graphicsState)
{
	// IF we have a triangulated mesh, ensure it is up to date. <- no? reload only when queried.
	/*
	if (triangulatedMesh)
	{
		if (triangulatedMesh->lastUpdate < mesh->lastUpdate)
		{
			triangulatedMesh->LoadDataFrom(mesh);

			triangulatedMesh->Triangulate();
			if (true)
				goto render;
		}
	}
render:
*/
	Mesh * triangulatedMesh = GetTriangulatedMesh();
	if (!triangulatedMesh)
	{
		std::cout<<"\nUnable to render model: "<<name;
		return;
	}
	triangulatedMesh->Render(graphicsState);
}

void Model::SetName(String i_name){
	// Deallocate if needed, then re-allocate.
	name = i_name;
}


/// Returns all triangles in this model.
List<Triangle> Model::GetTris()
{
	List<Triangle> triangleList;
	triangulatedMesh = GetTriangulatedMesh();
	if (triangulatedMesh)
	{
		List<Triangle> triMeshTris = triangulatedMesh->GetTris();
		triangleList += triMeshTris;
	}
	assert(triangleList.Size() > 0);
	return triangleList;
}

/// Updates skinning matrix map as described by the original mesh.
void Model::UpdateSkinningMatrixMap()
{
	/// Update skinning matrix map
	if (boneSkinningMatrixBuffer)
	{
		delete[] boneSkinningMatrixBuffer;
		boneSkinningMatrixBuffer = NULL;
	}
	int numBones = 0;
	if (boneSkinningMatrixBuffer == NULL)
	{
		List<Bone*> allBones;	
		Bone * skeleton = mesh->skeleton;
		assert(skeleton);
		skeleton->FetchBones(allBones);
		numBones = allBones.Size();
		int boneSkinningMatrixBufferSize = numBones * 16;
		boneSkinningMatrixBuffer = new float[boneSkinningMatrixBufferSize];
		memset(boneSkinningMatrixBuffer, 0, boneSkinningMatrixBufferSize * sizeof(float));
		for (int i = 0; i < numBones; ++i)
		{
			Bone * bone = allBones[i];
			// Check index.
			int boneIndex = bone->boneIndex;
			if (boneIndex < 0)
				continue;
			Matrix4f skinningMatrix = bone->nodeModelMatrix * mesh->invBindPoseMatrices[boneIndex];
			// Insert data appropriately.
			memcpy(&boneSkinningMatrixBuffer[boneIndex * 16], skinningMatrix.getPointer(), 16 * sizeof(float));
		}
		float * f = boneSkinningMatrixBuffer;
		if (false)
		{
			for (int i = 0; i < boneSkinningMatrixBufferSize; i += 4)
			{
				std::cout<<"\n"<<f[i]<<" "<<f[i+1]<<" "<<f[i+2]<<" "<<f[i+3];
				if ((i + 4) % 16 == 0)
					std::cout<<"\n";
			}
		}
	}
	// Update the actual texture.
	if (numBones > 0)
	{
		if (boneSkinningMatrixMap == -1)
			boneSkinningMatrixMap = GLTextures::New();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, boneSkinningMatrixMap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Single 32-bit floating point red channel texture!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 16, numBones, 0, GL_LUMINANCE, GL_FLOAT, boneSkinningMatrixBuffer);
		
		float fData[2048];

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, fData); 
		CheckGLError("UpdateSkinningMatrixMap");
	}
}


/// Returns the triangulized mesh, which may or may not be the original mesh depending on.. stuff.
Mesh * Model::GetTriangulatedMesh()
{
	if (mesh->IsTriangulated())
		return mesh;
	else if (triangulatedMesh)
		return triangulatedMesh;
	else {
		std::cout<<"\n"<<name<<" model not triangulating. Trying to do it for you.";
		mesh->Triangulate();
		if (mesh->IsTriangulated())
			return mesh;
	}
	assert(false && "Lacking triangulized mesh.");
	return NULL;
}

/// Re-creates the triangulized mesh. Call after changes have been made to the base mesh.
bool Model::RegenerateTriangulizedMesh()
{
	std::cout<<"\nRegenerateTriangulizedMesh...";
	// Hope noone's using this model.. maybe flag it somehow first..
	if (triangulatedMesh)
		// Delete all but any potential GL buffers we were using.
		triangulatedMesh->Delete();
	else
		triangulatedMesh = new Mesh();

	triangulatedMesh->LoadDataFrom(mesh);
	triangulatedMesh->Triangulate();
	std::cout<<" done.";
	return true;
}

float Model::Radius() const
{
	return mesh->radiusOrigo;
}


/// Returns the AABB.
const AABB & Model::GetAABB()
{
	return *mesh->aabb;
}
