/// Emil Hedemalm
/// 2014-07-27 (header added recently, original was much older)
/// A complete model, which may have multiple mesh-parts.

#ifndef MODEL_H
#define MODEL_H

#include "Util.h"
#include "MathLib.h"
#include "Graphics/OpenGL.h"

class AABB;
class Mesh;
class Texture;
class Triangle;
class GraphicsState;

/** Defines a single object-type that may be used by several instanced entities
	and include several kinds of meshes, default textures, etc.
*/
class Model {
	friend class ModelManager;
public:
	Model();
	~Model();

	void BufferizeIfNeeded();
	/// Calls render on the triangulized mesh parts within.
	void Render(GraphicsState& graphicsState);

	void SetName(String name);
	String Name(){ return name; };
	String Source();
	/// Returns the source location using relative path.
	String RelativePath();

	/// Returns a list of all triangles in this model.
	List<Triangle> GetTris();

	/// Updates skinning matrix map as described by the original mesh.
	void UpdateSkinningMatrixMap();

//	void SetTexture(Texture * newTexture){ texture = newTexture; };
//	Texture * GetTextureBySource) { return texture; };

	static Texture * GetDefaultTexture() {return defaultTexture; };

	/// Default texture to be used for rendering.
	Texture * texture;

	/** Mesh just as it was loaded from file, as can be used for creating physics mesh,
		debug-rendering, etc.
	*/
	Mesh * mesh;

	/// Returns the triangulized mesh, which may or may not be the original mesh depending on.. stuff.
#define GetTriangulizedMesh GetTriangulatedMesh
	Mesh * GetTriangulatedMesh();

	/// Re-creates the triangulized mesh. Call after changes have been made to the base mesh.
	bool RegenerateTriangulizedMesh();


	/// Keeps track of active amount of users for this model.
	int users;

	/// Center of mesh (measured from internal structure), calculated after it's been loaded.
	Vector3f centerOfModel;
	
	/// Radius as based in origo (0,0,0). If a local radius is desired, recalculate it using the centerOfModel. Refers to base mesh radius.
//	float radius;
	float Radius() const;

	/// Returns the AABB.
	const AABB & GetAABB();


	// o.o
	GLuint boneSkinningMatrixMap;
	/// Compact data of the bone skinning matrices as will be loaded in the shaders, in the form of a 16xN texture while shading. single 16 x N long array in CPU.
	float * boneSkinningMatrixBuffer;
private:

	/// Triangulized version of the mesh, for rendering!
	Mesh * triangulatedMesh;

	/// Dynamically allocated, this list should be updated when queried only.
	List<Triangle*> * triangleList;

	/// TODO: Add handling for animated meshes.

	/// Pointer to basic texture for this object
//	Texture * texture;

	/// Unique name assigned the object. Null-name is considered invalid.
	String name;
	String source;
	static char * defaultMesh;		/// Source for default mesh
	static Texture * defaultTexture;	/// Source for default texture
};

#endif
