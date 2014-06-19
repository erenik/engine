// Emil Hedemalm
// 2013-03-20

//#include <windows.h>

#include <GL/glew.h>
#include "Mesh.h"
#include "GraphicsState.h"
#include "Material.h"
#include "OS/Sleep.h"
#include "Graphics/GraphicsManager.h"
#include "PhysicsLib/Shapes.h"
#include "Graphics/GLBuffers.h"
extern GraphicsManager graphics;

#include <iostream>
#include <Util.h>

MeshFace::MeshFace(){
	Nullify();
}
void MeshFace::Nullify(){
    numVertices = 0;
	vertex = uv = normal = NULL;
};
MeshFace::~MeshFace(){
//	std::cout<<"\nMesh face destructor.";
	if (vertex)
		delete[] vertex;
	if (uv)
		delete[] uv;
	if (normal)
		delete[] normal;
	vertex = uv = normal = NULL;
}

/// Debug
void MeshFace::Print(){
	std::cout<<"v:";
	for (int i = 0; i < numVertices; ++i){
		std::cout<<" "<<vertex[i];
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
void MeshFace::operator = (const MeshFace & otherMeshFace){
   //  std::cout<<"op1";
  //   std::cout<<"hai.";
	numVertices = otherMeshFace.numVertices;
	assert(otherMeshFace.numVertices);
	vertex = new unsigned int[numVertices];
	uv = new unsigned int[numVertices];
	normal = new unsigned int[numVertices];
	for (int i = 0; i < numVertices; ++i){
		vertex[i] = otherMeshFace.vertex[i];
	}
	if (otherMeshFace.uv){
		for (int i = 0; i < numVertices; ++i){
			uv[i] = otherMeshFace.uv[i];
		}
	}
	if (otherMeshFace.normal){
		for (int i = 0; i < numVertices; ++i){
			normal[i] = otherMeshFace.normal[i];
		}
	}
	uvTangent = otherMeshFace.uvTangent;
 }
/// Copy CONSTRUCTOR
/*
face::face(const face & otherMeshFace){
	std::cout<<"hai.";
	numVertices = otherMeshFace.numVertices;
	assert(otherMeshFace.numVertices);
	vertex = new unsigned int[numVertices];
	uv = new unsigned int[numVertices];
	normal = new unsigned int[numVertices];
	for (int i = 0; i < numVertices; ++i){
		vertex[i] = otherMeshFace.vertex[i];
		uv[i] = otherMeshFace.uv[i];
		normal[i] = otherMeshFace.normal[i];
	}
	uvTangent = otherMeshFace.uvTangent;
}*/
/*
void face::operator = (const face & otherMeshFace){
	std::cout<<"hai.";
	std::cout<<"hai. ho?";
	return;
	std::cout<<"Vertices: "<<otherMeshFace.numVertices;
	numVertices = otherMeshFace.numVertices;
	if (vertex){
	  delete[] vertex;
	  vertex = NULL;
	}
	if (uv){
	  delete[] uv;
	  uv = NULL;
	}
	if (normal){
	  delete[] normal;
	  normal = NULL;
	}
	if (otherMeshFace.numVertices <= 0){
	  std::cout<<"0 or less vertices.. f this.";
	  return;
	}
	vertex = new unsigned int[numVertices];
	uv = new unsigned int[numVertices];
	normal = new unsigned int[numVertices];
	for (int i = 0; i < numVertices; ++i){
		vertex[i] = otherMeshFace.vertex[i];
		std::cout<<"\nv: "<<vertex[i]<< " " << otherMeshFace.vertex[i];
		uv[i] = otherMeshFace.uv[i];
		normal[i] = otherMeshFace.normal[i];
	}
	uvTangent = otherMeshFace.uvTangent;
}
void face::operator = (const MeshFace * otherMeshFace){
	assert(false && "Implement");
	std::cout<<"yo.";
}
*/


Mesh::Mesh(){
	Nullify();
}

/// Nullifies all default variables!
void Mesh::Nullify(){
	vertices = uvs = faces = normals = 0;
	vertex = NULL;
	u = NULL;
	v = NULL;
	face = NULL;
	textureID = 0;
	vboBuffer = NULL;
	floatsPerVertex = 0;
	triangulated = false;
}

Mesh::~Mesh(){
//	std::cout<<"\nMesh destructor.";
	if (vertex != NULL){
		delete[] vertex;
		vertex = NULL;
    }
	if (u != NULL)
		delete[] u;
		u = NULL;
	if (v != NULL)
		delete[] v;
		v = NULL;
	if (face != NULL){
		delete[] face;
		face = NULL;
	}
}

/// Mostly for debug
void Mesh::PrintContents(){

    for (int i = 0; i < faces; ++i){
        MeshFace * f = &face[i];
        std::cout<<"\nMeshFace "<<i<<": vertices("<<(int)f->numVertices<<"): ";
        for (int v = 0; v < f->numVertices; ++v){
            if (f->vertex)
             std::cout<<"v"<<f->vertex[v];
            if (f->uv)
             std::cout<<"t"<<f->uv[v];
            if (f->normal)
             std::cout<<"n"<<f->normal[v];
        }
    }
}

/// Replaces copy-constructor.
bool Mesh::LoadDataFrom(const Mesh * otherMesh){
    std::cout<<"\nLoadDataFrom mesh constructor begun...";

    Nullify();
//	return true;

	assert(otherMesh->vertices);
	std::cout<<"\nMesh vertices: "<<otherMesh->vertices;
	name = otherMesh->name;
	source = otherMesh->source;
	vertices = otherMesh->vertices;
	uvs = otherMesh->uvs;
	normals = otherMesh->normals;
	faces = otherMesh->faces;
	std::cout<<"\nMesh faces: "<<otherMesh->faces;
//	return true;
	vertex = new Vector3f[vertices];
	for (int i = 0; i < vertices; ++i){
		vertex[i] = otherMesh->vertex[i];
	}
//	return true;
	if (uvs){
		u = new float[uvs];
		v = new float[uvs];
		for (int i = 0; i < uvs; ++i){
			u[i] = otherMesh->u[i];
			v[i] = otherMesh->v[i];
		}
	}
	else {
		u = NULL;
		v = NULL;
	}
//	return true;
	if (normals){
		normal = new Vector3f[normals];
		for (int i = 0; i < normals; ++i){
			normal[i] = otherMesh->normal[i];
		}
	}
	else
		normal = NULL;
 //   return true;
	if (faces){
		face = new MeshFace[faces];
	//	std::cout<<"\nMeshFaces: "<<faces;
    //    std::cout<<"\nOtherMeshFace: "<<&otherMesh->face[0]<<" "<<otherMesh->face[0].numVertices;

		for (int i = 0; i < faces; ++i){
		//    std::cout<<"\nCopying face "<<i<<" ";
         //   face[i].uvTangent = otherMesh->face[i].uvTangent;
			face[i] = otherMesh->face[i];
		//	std::cout<<"v0: "<<face[i].vertex[0]<<" Original: "<<otherMesh->face[i].vertex;
		//	assert(face[i].vertex != otherMesh->face[i].vertex);
		//	if (face[i].vertex[0] > 3000000)
         //       ; //Sleep(5000);
		//	assert(face[i].vertex[0] < 3000000);
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

		assert(floatsPerVertex >= 8 && "Bad float-count per vertex, ne?!");

		// Set VBO and render

		// Bind vertices
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

		checkGLError();
		int vertices = vboVertexCount;

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
		checkGLError();
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
	if (mesh.vertices)
		throw 1;

	// Give it an automized name based on the sections
	mesh.SetName("Sphere");
	mesh.SetSource("Generated.");

	float dSections = (float)sections;
	float halfSections = dSections / 2.0f;
	int vertexCount = (sections + 1) * (sections + 1);

	// Default to 0
	mesh.uvs = mesh.normals = mesh.vertices = 0;

	// Allocate arrays
	mesh.vertex = new Vector3f[vertexCount];
	mesh.u = new float[vertexCount];
	mesh.v = new float[vertexCount];
	mesh.normal = new Vector3f[vertexCount];

	// For each row
	for (int i = 0; i < sections+1; ++i){
		// For each vertex in the row
		for (int j = 0; j < sections+1; ++j){


			int cIndex = i * (sections + 1) + j;
			//						Regular sinus for x				multiply with sine of row to get the relative size.
			mesh.vertex[cIndex].x = 1 * sin((j) / dSections * PI * 2) * sin((i) / dSections * PI);
			mesh.vertex[cIndex].y = 1 * cos((i) / dSections * PI);
			mesh.vertex[cIndex].z = 1 * cos((j) / dSections * PI * 2) * sin((i) / dSections * PI);

			mesh.u[cIndex] = (j / (float)sections);
			mesh.v[cIndex] = (1 - i / (float) sections);

			mesh.normal[cIndex] = Vector3f(mesh.vertex[cIndex].x, mesh.vertex[cIndex].y, mesh.vertex[cIndex].z).Normalize();

			++mesh.vertices;
			++mesh.uvs;
			++mesh.normals;
		}
	}


	// Triangulate straight away.
	mesh.face = new MeshFace[sections * sections * 2];
	// Default to 0.
	mesh.faces = 0;
	// Index to start looking at for each row.
	int index = 0;
	// Create faces
	for (int i = 0; i < sections; ++i){
		for (int j = 0; j < sections; ++j){
			// 3 vertices
			MeshFace * face = &mesh.face[mesh.faces];
			face->numVertices = 3;
			// Allocate
			face->vertex = new unsigned int[face->numVertices];
			face->uv = new unsigned int[face->numVertices];
			face->normal = new unsigned int[face->numVertices];
			int v;
			v = 0; face->vertex[v] = face->normal[v] = face->uv[v] = index + j + 1 + sections;
			v = 1; face->vertex[v] = face->normal[v] = face->uv[v] = index + j + 1 + sections + 1;
			v = 2; face->vertex[v] = face->normal[v] = face->uv[v] = index + j + 1;
			++mesh.faces;

			// 3 vertices
			face = &mesh.face[mesh.faces];
			face->numVertices = 3;
			// Allocate
			face->vertex = new unsigned int[face->numVertices];
			face->uv = new unsigned int[face->numVertices];
			face->normal = new unsigned int[face->numVertices];
			v = 0; face->vertex[v] = face->normal[v] = face->uv[v] = index + j + 1;
			v = 1; face->vertex[v] = face->normal[v] = face->uv[v] = index + j;
			v = 2; face->vertex[v] = face->normal[v] = face->uv[v] = index + j + 1 + sections;
			++mesh.faces;

	/*		// Use one normal per face so we can see each face nao
			glNormal3f(vertex[index+j].x, vertex[index+j].y, vertex[index+j].z);

			// 1
			//	glNormal3f(vertex[index+j+1+sections].x, vertex[index+j+1+sections].y, vertex[index+j+1+sections].z);
			glTexCoord2f(j / (float)sections, 1 - (i+1) / (float) sections);
			glVector3f(vertex[index+j+1+sections].x, vertex[index+j+1+sections].y, vertex[index+j+1+sections].z);

			// 2
			//	glNormal3f(vertex[index+j+1+sections+1].x, vertex[index+j+1+sections+1].y, vertex[index+j+1+sections+1].z);
			glTexCoord2f((j+1) / (float)sections, 1 - (i+1) / (float) sections);
			glVector3f(vertex[index+j+1+sections+1].x, vertex[index+j+1+sections+1].y, vertex[index+j+1+sections+1].z);

			// 3
			//	glNormal3f(vertex[index+j+1].x, vertex[index+j+1].y, vertex[index+j+1].z);
			glTexCoord2f((j+1) / (float)sections, 1 - i / (float) sections);
			glVector3f(vertex[index+j+1].x, vertex[index+j+1].y, vertex[index+j+1].z);

			// 4
			glTexCoord2f(j / (float)sections, 1 - i / (float) sections);
			glVector3f(vertex[index+j].x, vertex[index+j].y, vertex[index+j].z);
			++mesh.faces;
			*/
		}
		// Increment sections + 1 since we're using extra vertices for simplicities sake.
		index += sections + 1;
	}
}


/// Triangulates the mesh.
void Mesh::Triangulate()
{
	triangulated = true;
	int newMeshFacesNeeded = faces;
    if (faces == 0)
        return;
	std::cout<<"\nMeshFaces: "<<faces;
	for (int i = 0; i < faces; ++i){
	 //   std::cout<<"D:";
		MeshFace * currentMeshFace = &face[i];
		if (currentMeshFace->numVertices > 3){
			int MeshFacesToAdd = currentMeshFace->numVertices - 3;
		//	std::cout<<"\nMeshFaces to add in face "<<i<<": "<<MeshFacesToAdd;
			newMeshFacesNeeded += currentMeshFace->numVertices - 3;
			if (MeshFacesToAdd > 20){
				std::cout<<"\nMeshFace vertex amount very large("<<(int)currentMeshFace->numVertices<<"), printing debug info:";
				assert(currentMeshFace->numVertices - 3 < 18);
				std::cout<<"\nAborting Triangularization since mesh data seems faulty!";
				return;
			}
		}
	}
	std::cout<<"\nNew faces needed:"<<newMeshFacesNeeded;
	// Allocate new MeshFace array
	int MeshFaceSize = sizeof(face);
	MeshFace * newMeshFace = new MeshFace[newMeshFacesNeeded];
	int MeshFacesAddedSoFar = 0;
	for (int i = 0; i < faces; ++i){
		MeshFace * newTri = &newMeshFace[MeshFacesAddedSoFar];
		MeshFace * oldMeshFace = &face[i];
		// Just copy
		if (face[i].numVertices == 3){
			newTri->normal = new unsigned int[3];
			newTri->uv = new unsigned int[3];
			newTri->vertex = new unsigned int[3];
			newTri->numVertices = 3;
			for (int j = 0; j < 3; ++j){
				newTri->normal[j] = face[i].normal[j];
				newTri->uv[j] = face[i].uv[j];
				newTri->vertex[j] = face[i].vertex[j];
			}
			MeshFacesAddedSoFar++;
		}
		else {
			int numVertices = face[i].numVertices;
	//		std::cout<<"\nConverting old face...";
	//		std::cout<<"\n";
	//		oldMeshFace->Print();
	//		std::cout<<"\nInto new faces...!";
			for (int j = 0; j < numVertices-2; ++j){
				/// Create new triangle for every extra vertex!
				newTri->normal = new unsigned int[3];
				newTri->uv = new unsigned int[3];
				newTri->vertex = new unsigned int[3];
				newTri->numVertices = 3;
				newTri->normal[0] = face[i].normal[0];
				newTri->uv[0] = face[i].uv[0];
				newTri->vertex[0] = face[i].vertex[0];
				for (int k = j+1; k < j+3; ++k){
					newTri->normal[k - j] = face[i].normal[k];
					newTri->uv[k - j] = face[i].uv[k];
					newTri->vertex[k - j] = face[i].vertex[k];
				}
				MeshFacesAddedSoFar++;
	//			std::cout<<"\n";
	//			newTri->Print();
				newTri = &newMeshFace[MeshFacesAddedSoFar];

			}
		}
	}
	delete[] face;
	face = newMeshFace;
	faces = newMeshFacesNeeded;
}

/// Deletes and recalculates ALL normals
void Mesh::RecalculateNormals(){
	std::cout<<"\nRecalculating normals...";
	if (normal)
		delete[] normal;
	normals = faces;
	normal = new Vector3f[normals];
	/// For every face...
	for (int i = 0; i < faces; ++i){
		assert(face[i].numVertices >= 3);
		Vector3f line1 = vertex[face[i].vertex[1]] - vertex[face[i].vertex[0]];
		Vector3f line2 = vertex[face[i].vertex[2]] - vertex[face[i].vertex[0]];
		normal[i] = line1.CrossProduct(line2).NormalizedCopy();
		if (face[i].normal == NULL)
			face[i].normal = new unsigned int[face[i].numVertices];
		for (int v = 0; v < face[i].numVertices; ++v){
			face[i].normal[v] = i;
		}
	}
}

/// Calculates radial and AABB boundaries.
void Mesh::CalculateBounds(){
	min = vertex[0];
	max = vertex[0];
	radius = 0;
	float newRadius = 0;
	for (int i = 0; i < vertices; ++i){
		newRadius = vertex[i].LengthSquared();
		if (newRadius > radius)
			radius = newRadius;

		// Get min
		if (vertex[i].x < min.x)
			min.x = vertex[i].x;
		if (vertex[i].y < min.y)
			min.y = vertex[i].y;
		if (vertex[i].z < min.z)
			min.z = vertex[i].z;
		// Get max
		if (vertex[i].x > max.x)
			max.x = vertex[i].x;
		if (vertex[i].y > max.y)
			max.y = vertex[i].y;
		if (vertex[i].z > max.z)
			max.z = vertex[i].z;
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
	for (int i = 0; i < vertices; ++i){
		// Radius
		newRadius = (vertex[i] - centerOfMesh).Length();
		if (newRadius > radius)
			radius = newRadius;
	}*/
}

/** Centerizes the model by pushing all vertices by the length of the the centerOfMesh vector.
	Resets centerOfMesh afterward. */
void Mesh::Center(){
	for (int i = 0; i < vertices; ++i){
		vertex[i] -= centerOfMesh;
	}
	centerOfMesh = Vector3f();
}

/// For NormalMapping~
void Mesh::CalculateUVTangents(){

	if (!u)
		return;
	for (int i = 0; i < faces; ++i){
		MeshFace * f = &face[i];
		if (f->numVertices < 3)
            continue;

		// Reference:
		// http://www.terathon.com/code/tangent.html

		/// 3 Points on the face
		Vector3f v1, v2, v3;
		v1 = vertex[f->vertex[0]];
		v2 = vertex[f->vertex[1]];
		v3 = vertex[f->vertex[2]];

		float v1u, v2u, v3u, v1v, v2v, v3v;
		v1u = u[f->uv[0]];
		v2u = u[f->uv[1]];
		v3u = u[f->uv[2]];
		v1v = v[f->uv[0]];
		v2v = v[f->uv[1]];
		v3v = v[f->uv[2]];

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

		// face UV "up"- and "right"-tangent respectively.
		Vector3f tan1f = sdir;
		Vector3f tan2f = tdir;

		for (int j = 0; j < 3 && j < f->numVertices; ++j){
			// Only check the 3 first vertices.
			const Vector3d& n = normal[f->normal[j]];
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

}

void Mesh::Bufferize()
{
	/// Always triangulate before buffering.
	if (!triangulated)
	{
		Triangulate();
	}
	

    if (Graphics.GL_VERSION_MAJOR < 2 && Graphics.GL_VERSION_MINOR < 5){
        std::cout<<"\nUnable to buffer mesh since GL version below is 1.5.";
        return;
    }

	// Check if already buffered, stupid..
	if (this->vboBuffer){
//		std::cout<<"\nINFO: Mesh "<<name<<" already buffered, skipping!";
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

	if (Graphics.GL_VERSION_MAJOR >= 3){
		// Generate VAO and bind it straight away if we're above GLEW 3.0
	//	glGenVertexArrays(1, vertexArrayObject);
	//	glBindVertexArray(vertexArrayObject[0]);
	}

	// Check for errors before we proceed.
	GLuint err = glGetError();
	if (err != GL_NO_ERROR)
		std::cout<<"\nUnknown error?";

	// Count total vertex/texture point pairs without any further optimization.
	unsigned int vertexCount = faces * 3;
	floatsPerVertex = 3 + 3 + 2 + 4;  // Pos + Normal + UV + NormalMappingTangent
	float * vboVertexData = new float[vertexCount * floatsPerVertex];

    std::cout<<"\nVertexCount: "<<vertexCount<<" MeshFaceCount: "<<faces;

	// Generate And bind The Vertex Buffer
	/// Check that the buffer isn't already generated
	if (vboBuffer == 0)
	{
		vboBuffer = GLBuffers::New();
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);

	// Load all data in one big fat array, yo Data
	unsigned int vertexDataCounted = 0;
	// Reset vertex count
	vboVertexCount = 0;
	for (int i = 0; i < faces; ++i){
		// Count vertices in all triangles
		for (int j = 0; j < 3; ++j){
			MeshFace * f = &face[i];
		//	std::cout<<"\nCurrentVertex";
			int currentVertex = face[i].vertex[j];
			assert(face[i].vertex[j] < 3000000);
			// Position
			vboVertexData[vertexDataCounted + 0] = vertex[currentVertex].x;
			vboVertexData[vertexDataCounted + 1] = vertex[currentVertex].y;
			vboVertexData[vertexDataCounted + 2] = vertex[currentVertex].z;
			// Normal
			if (normal){
				int currentNormal = face[i].normal[j];
				vboVertexData[vertexDataCounted + 3] = normal[currentNormal].x;
				vboVertexData[vertexDataCounted + 4] = normal[currentNormal].y;
				vboVertexData[vertexDataCounted + 5] = normal[currentNormal].z;
			}
			else
				vboVertexData[vertexDataCounted + 3] =
				vboVertexData[vertexDataCounted + 4] =
				vboVertexData[vertexDataCounted + 5] = 0;
			// UV
			if (u && v){
				int currentUV = face[i].uv[j];
				vboVertexData[vertexDataCounted + 6] = u[currentUV];
				vboVertexData[vertexDataCounted + 7] = v[currentUV];
			}
			else {
				vboVertexData[vertexDataCounted + 6] =
				vboVertexData[vertexDataCounted + 7] = 0.0f;
			}
			/// Tangents for NormalMapping
			if (true) {
				vboVertexData[vertexDataCounted + 8] = f->uvTangent.x;
				vboVertexData[vertexDataCounted + 9] = f->uvTangent.y;
				vboVertexData[vertexDataCounted + 10] = f->uvTangent.z;
				vboVertexData[vertexDataCounted + 11] = f->uvTangent.w;
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

/// Searches through the mesh's vertices and returns the maximum bounding radius required by the Entity.
float getMaxBoundingRadius(Mesh * mesh){
/*	if (mesh == NULL){
		std::cout<<"WARNING: Mesh NULL in getMaxBoundingRadius!";
		return 0;
	}
	*/
	float maxX = 0.0f, maxY = 0.0f, maxZ = 0.0f;
	for (int i = 0; i < mesh->vertices; ++i){
		if (AbsoluteValue(mesh->vertex[i].x) > maxX)
			maxX = mesh->vertex[i].x;
		if (AbsoluteValue(mesh->vertex[i].y) > maxY)
			maxY = mesh->vertex[i].y;
		if (AbsoluteValue(mesh->vertex[i].z) > maxZ)
			maxZ = mesh->vertex[i].z;
	}
	float max = sqrt(pow(maxX, 2) + pow(maxY, 2) + pow(maxZ, 2));
	return max;
}

List<Triangle> Mesh::GetTris(){
	List<Triangle> triangles;
	for (int i = 0; i < faces; ++i){
		// Just copy shit from it
		MeshFace * f = &face[i];
		assert((f->numVertices <= 4 || f->numVertices >= 3) && "Bad vertex count in face");

		Vector3f p1 = vertex[f->vertex[0]],
			p2 = vertex[f->vertex[1]],
			p3 = vertex[f->vertex[2]];

		if (f->numVertices == 4){
			assert(false && "face has quads!");
// 			Vector3f p4 = vertex[face->vertex[3]];
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
