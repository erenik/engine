
#ifndef MESH_H
#define MESH_H

#include "MathLib.h"
#include <Util.h>

#include "AEGlew.h"

struct Triangle;
struct GraphicsState;

/// A struct for a geometric MeshFace, containing number of vertices and indices for which vertex is used and which UV-coordinate is used.
struct MeshFace {
	MeshFace();
	~MeshFace();
private:
    void Nullify();
public:
	/// Debug
	void Print();

    /// Copy CONSTRUCTOR
	MeshFace(const MeshFace & otherMeshFace);
	void operator = (const MeshFace * otherMeshFace);
	/// Copy CONSTRUCTOR
	void operator = (const MeshFace & otherMeshFace);

	/// Number of vertices in the MeshFace. A MeshFace will have no more than 255 vertices, and if it does: this program won't like it anyway :P
	unsigned char numVertices;
	/// Dynamic array for the vertices, since the MeshFace can be an arbitrary polygon.
	unsigned int * vertex;
	/// Dynamic array for uv-coordinates, since the MeshFace can be an arbitrary polygon.
	unsigned int * uv;
	/// Dynamic array for normal-coordinates, since the MeshFace can be an arbitrary polygon.
	unsigned int * normal;

	// MeshFace UV "up"- and "right"-tangent respectively for NormalMapping.
	Vector4f uvTangent;
	Vector3f uvBiTangent;
};

/** A struct for a geometric mesh, containing vertices, UV-coordinates, MeshFaces, material properties and a single texture ID.
Mesh using different texture will have to be defined later. */
class Mesh {
private:
	Mesh(const Mesh & mesh);
	/// Nullifies all default variables!
	void Nullify();
	void operator = (const Mesh * otherMeshFace);
	void operator = (const Mesh & otherMeshFace);
	Mesh(const Mesh * mesh);
public:
	/// Default constructor
	Mesh();
	/// Destructor
	virtual ~Mesh();

    /// Mostly for debug
	void PrintContents();
	/// Replaces copy-constructor.
	bool LoadDataFrom(const Mesh * otherMesh);
	/// Triangulates the mesh.
	void Triangulate();
	/// Deletes and recalculates ALL normals
	void RecalculateNormals();
	/// Calculates radial and AABB boundaries.
	void CalculateBounds();
	/** Centerizes the model by pushing all vertices by the length of the the centerOfMesh vector.
		Resets centerOfMesh afterward. */
	void Center();
	/// Buffers the mesh into graphics memory.
	void Bufferize();
	/// Renders the meshi-mesh :3
	void Render(GraphicsState &state);

	/// For NormalMapping~
	void CalculateUVTangents();

	/// Returns the relative path to the resource the mesh was loaded from.
	String RelativePath();

	/** Returns a list of all triangles in this mesh, using given MeshFace-data! NOTE: Triangulized?
		The triangles returned are dynamically allocated and MUST be deleted by the user! D:
	*/
	List<Triangle> GetTris();

	/// Center of mesh (measured from internal structure), calculated after it's been loaded.
	Vector3f centerOfMesh;
	/// Radius
	float radius;
	/// AABB
	Vector3f max, min;

	/// Amount of vertices in the mesh.
	int vertices;
	/// Amount of UV-coordinates in the mesh.
	int uvs;
	/// Amount of MeshFaces in the mesh.
	int faces;
	/// Amount of normals in the mesh.
	int normals;

	/// Carteesian coordinates for the vertices
	Vector3f * vertex;
	/// U-coordinates for the textures
	float * u;
	/// V-coordinates for the textures
	float * v;
	/// Normals for each vertex. Pointer is null if
	Vector3f * normal;
	/// MeshFaces that define the mesh, using the provided vertices and UV-coordinates
	MeshFace * face;
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
private:
	/// Set after calling Triangulate only.
	bool triangulated;
	// How many floats were buffered, per vertex.
	int floatsPerVertex;
};

struct GraphicsState;

/// Function to render a mesh easily with just a pointer to the active mesh.
//void RenderMesh(GraphicsState &state, Mesh * mesh);

/** Function to create a sphere
	Allocates vertices for a sphere into target mesh. Total MeshFaces will be sections^2 and vertices/UVs/Normals (sections+1)^2
*/
void CreateSphere(Mesh &mesh, int sections = 10);
/** Function to create a cube
	Allocates vertices for a sphere into target mesh. Total MeshFaces will be sections^2 and vertices/UVs/Normals (sections+1)^2
	If flattened is true, the UV-coordinate will be placed so that each side gets it's own MeshFace.
*/
//void CreateCube(Mesh &mesh, float size = 1.0f, bool flattened = false);
/// Buffers target mesh into video memory.
void bufferMesh(Mesh &mesh);

/// Searches through the mesh's vertices and returns the maximum bounding radius required by the Entity.
float getMaxBoundingRadius(Mesh * mesh);



#endif
