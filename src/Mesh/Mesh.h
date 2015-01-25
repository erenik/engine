/// Emil Hedemalm
/// 2014-08-01
/// Old contents, but new file separation. 

#ifndef MESH_H
#define MESH_H

#include "Graphics/OpenGL.h"

#include "MathLib.h"
#include "List/List.h"
#include "String/AEString.h"
#include "MeshFace.h"
#include "Time/Time.h"

class Triangle;
class GraphicsState;
class EMesh;
class AABB;

class SkeletalAnimationNode;

class VertexWeight
{
public:
	float weight;
	int boneIndex;
};

/** A struct for a geometric mesh, containing vertices, UV-coordinates, MeshFaces, material properties and a single texture ID.
Mesh using different texture will have to be defined later. */
class Mesh 
{
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

	/// Deletes all parts within this mesh (vertices, faces, edges, etc.)
	void Delete();

	/// Adds a plane, creating 2 faces in a counter-clockwise manner.
//	void AddPlane(const Vector3f & upperLeft, const Vector3f & lowerLeft, const Vector3f & lowerRight, const Vector3f & upperRight);
	/// Adds a grid (basically a plane), with the specified amount of cells/faces in X and Y.
//	void AddGrid(const Vector3f & upperLeft, const Vector3f & lowerLeft, const Vector3f & lowerRight, const Vector3f & upperRight, Vector2i gridSizeDivision);


	// Allocates the vertex, u,v and normal arrays
	void AllocateArrays();
	void DeallocateArrays();
	
	/// Load from customized compressed data form. Returns true upon success.
	bool SaveCompressedTo(String compressedPath);
	/// Load from customized compressed data form. Returns true upon success.
	bool LoadCompressedFrom(String compressedPath);

    /// Mostly for debug
	void PrintContents();
	/// Replaces copy-constructor. Set nullify to true if you are creating the new mesh for the first time.
	bool LoadDataFrom(const Mesh * otherMesh, bool nullify = false);
	/// For creating an optimized version of the more complex/pointer-oriented E(ditable)-Mesh.
	bool LoadDataFrom(const EMesh * otherMesh);
	/// Triangulates the mesh.
	void Triangulate();
	/// Deletes and recalculates ALL normals
	void RecalculateNormals();
	/// Calculates radial and AABB boundaries.
	void CalculateBounds();
	/** Centerizes the model by pushing all vertices by the length of the the centerOfMesh vector.
		Resets centerOfMesh afterward. */
	void Center();
	/// Buffers the mesh into graphics memory. Use force to make it re-bufferize already buffered meshes.
	void Bufferize(bool useOriginalPositions, bool force);

	/// Renders the meshi-mesh :3
	void Render(GraphicsState & graphicsState);
	/// Re-skins the mesh's vertices to match the current skeletal animation. Every single vertex will be re-calculated and then re-buffered.
	void SkinToCurrentSkeletalAnimation();
	/// Updates the skinning matrix map to be used for skinning using shaders.
	void UpdateSkinningMatrixMap();

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
	/// Radius, global such.
	float radius;
	/// AABB
//	Vector3f max, min;

	/// Set upon loading and while animating, to make sure the triangulated/rendered version is up to date.
	Time lastUpdate, lastBufferization;

	/// Amount of vertices in the mesh.
	int numVertices;
	/// Amount of UV-coordinates in the mesh.
	int numUVs;
	/// Amount of MeshFaces in the mesh.
	int numFaces;
	/// Amount of normals in the mesh.
	int numNormals;

	/// Carteesian coordinates for the vertices
	List<Vector3f> vertices;
	/// When skeleton animation is initiated, the vertices list will be updated. Therefore, all original vertex positions will be stored here henceforth.
	List<Vector3f> originalVertexPositions;
	/// Joint UV-coordinates replacing the old separate u/v arrays!
	List<Vector2f> uvs;
	/// Normals for each vertex. Pointer is null if
	List<Vector3f> normals;
	/// MeshFaces that define the mesh, using the provided vertices and UV-coordinates
	List<MeshFace> faces;

	/// Vertex-bone weights in compact form. Indices to the bones.
	List<Vector4i> boneIndices;
	/// Vertex-bone weights in compact form. Weights of said indices.
	List<Vector4f> boneWeights;
	
	/// Skinning data! o.o
	List<int> weightsPerVertex;	
	/// Contains max weights per vertex for this mesh. May vary depending on the DCC tool used to generate it.
	int maxWeightsPerVertex;
	/// Weights for each vertex. Each vertex will have a list of weights.
	List<List<VertexWeight>> vertexWeights;
	/// Inverse bind pose matrices for all bones/joints.
	List<Matrix4f> invBindPoseMatrices;
	// Bind-shape matrix (relationship between skeleton and skin when it was bound/defined)
	Matrix4f bindShapeMatrix;

	/// Texture ID used when rendering the MeshFaces.
	GLuint textureID;
	/// Identifier for the Vertex Buffer, containing one or more of the following data: positions, normals, uvs, tangents, bone indices and bone weights. 
	GLuint vertexBuffer;
	/// Buffer for the bone indices data. This will never change during animation and may be loaded once.
	GLuint boneIndexBuffer;
	/// Buffer for the bone weights. This will never change during animatoin either and may be loaded once too.
	GLuint boneWeightsBuffer;

	/// Sets source of the texture.
	void SetSource(String str);
	/// Sets name of the texture.
	void SetName(String str);

	/// Source of the OBJ file.
	String source;
	/// Name/Identifier of the mesh.
	String name;

	/// Vertex count for the generated buffer objects.
	unsigned int vertexDataCount;

	bool IsTriangulated(){return triangulated;};

	/// size of it all.
	AABB * aabb;

	// Whole skeleton for a mesh! o.o D: :D
	SkeletalAnimationNode * skeleton;

protected:
	/// Set after calling Triangulate only.
	bool triangulated;
	// How many floats were buffered, per vertex.
	int floatsPerVertex;

	bool originalPositionsBuffered;
	/// own format.
	bool loadedFromCompactObj;

	/// Used during buffering operations. Stored here so that it may be re-used o.o;
	float * vertexData; 
	int vertexDataSize; // Size of said array.

	int * boneIndexData;
	float * boneWeightData;

};

class GraphicsState;

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
