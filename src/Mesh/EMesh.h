/// Emil Hedemalm
/// 2014-07-28
/// An editable mesh class, in contrast to the relatively optimized/compact EMesh class.

#ifndef EDITABLE_MESH_H
#define EDITABLE_MESH_H

#include "MathLib.h"
#include <Util.h>

#include "Graphics/OpenGL.h"
#include "Matrix/Matrix.h"

class Triangle;
class EVertex;
class EFace;
class ENormal;
class EUV;

/** A struct for an editable geometric mesh.
*/
class EMesh {
private:
	EMesh(const EMesh & mesh);
	/// Nullifies all default variables!
	void Nullify();
	void operator = (const EMesh * otherMeshFace);
	void operator = (const EMesh & otherMeshFace);
	EMesh(const EMesh * mesh);
public:
	/// Default constructor
	EMesh();
	/// Destructor
	virtual ~EMesh();

	/// Deletes all parts within this mesh (vertices, faces, edges, etc.)
	void Delete();

	/// Adds a plane, creating 2 faces in a counter-clockwise manner.
	void AddPlane(const Vector3f & upperLeft, const Vector3f & lowerLeft, const Vector3f & lowerRight, const Vector3f & upperRight);
	/// Adds a grid (basically a plane), with the specified amount of cells/faces in X and Y.
	void AddGrid(const Vector3f & upperLeft, const Vector3f & lowerLeft, const Vector3f & lowerRight, const Vector3f & upperRight, Vector2i gridSizeDivision);
	
	/// For when creating a grid or plane, these matrices will hold the data until another call is made.
	Matrix<EVertex*> vertexMatrix;
	Matrix<EFace*> faceMatrix;


	/// Carteesian coordinates for the vertices
	List<EVertex*> vertices;
	/// Joint UV-coordinates replacing the old separate u/v arrays!
	List<EUV*> uvs;
	/// Normals for each vertex. Pointer is null if
	List<ENormal*> normals;
	/// MeshFaces that define the mesh, using the provided vertices and UV-coordinates
	List<EFace*> faces;

	/// Texture ID used when rendering the MeshFaces.
	GLuint textureID;
	/// Identifier for the Vertex Buffer
	GLuint vboBuffer;


	/// Sets source of the texture.
	void SetSource(String str);
	/// Sets name of the texture.
	void SetName(String str);

	/// Source of the OBJ file.
	String source;
	/// Name/Identifier of the mesh.
	String name;

	/// Vertex count for the generated buffer objects.
	unsigned int vboVertexCount;

	bool IsTriangulated(){return triangulated;};
protected:
	/// Set after calling Triangulate only.
	bool triangulated;
	// How many floats were buffered, per vertex.
	int floatsPerVertex;
};

class GraphicsState;

/// Function to render a mesh easily with just a pointer to the active mesh.
//void RenderMesh(GraphicsState &state, EMesh * mesh);

/** Function to create a sphere
	Allocates vertices for a sphere into target mesh. Total MeshFaces will be sections^2 and vertices/UVs/Normals (sections+1)^2
*/
void CreateSphere(EMesh &mesh, int sections = 10);
/** Function to create a cube
	Allocates vertices for a sphere into target mesh. Total MeshFaces will be sections^2 and vertices/UVs/Normals (sections+1)^2
	If flattened is true, the UV-coordinate will be placed so that each side gets it's own MeshFace.
*/
//void CreateCube(EMesh &mesh, float size = 1.0f, bool flattened = false);
/// Buffers target mesh into video memory.
void bufferMesh(EMesh &mesh);

/// Searches through the mesh's vertices and returns the maximum bounding radius required by the Entity.
float getMaxBoundingRadius(EMesh * mesh);



#endif
