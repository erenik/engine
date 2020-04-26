/// Emil Hedemalm
/// 2014-08-01
/// Old contents, but new file separation. 

#include <fstream>
#include "List/List.h"
#include "MathLib.h"

class Mesh;

// A struct for a geometric MeshFace, containing number of vertices and indices for which vertex is used and which UV-coordinate is used.
// https://msdn.microsoft.com/en-us/library/83ythb65.aspx

struct  
#ifdef USE_SSE
	alignas(16)
#endif
	MeshFace
{
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

	// Call after setting numVertices
	void AllocateArrays();
	void DeallocateArrays();

	bool WriteTo(std::fstream & fileStream);
	bool ReadFrom(std::fstream & fileStream);

	/// Number of vertices in the MeshFace. A MeshFace will have no more than 255 vertices, and if it does: this program won't like it anyway :P
	int numVertices;
	/// Dynamic array for the vertices, since the MeshFace can be an arbitrary polygon.
	List<int> vertices;
	/// Dynamic array for uv-coordinates, since the MeshFace can be an arbitrary polygon.
	List<int> uvs;
	/// Dynamic array for normal-coordinates, since the MeshFace can be an arbitrary polygon.
	List<int> normals;

	// MeshFace UV "up"- and "right"-tangent respectively for NormalMapping.
	Vector4f uvTangent;
	Vector3f uvBiTangent;
};
