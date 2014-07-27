/// Emil Hedemalm
/// 2014-07-27 (header added recently, original was much older)
/// A complete model, which may have multiple mesh-parts.

#ifndef MODEL_H
#define MODEL_H

#include "Util.h"
#include "PhysicsLib/AxisAlignedBoundingBox.h"

class Mesh;
class Texture;
class Triangle;

/** Defines a single object-type that may be used by several instanced entities
	and include several kinds of meshes, default textures, etc.
*/
class Model {
	friend class ModelManager;
public:
	Model();
	~Model();

	/// Calls render on the triangulized mesh parts within.
	void Render();

	void SetName(String name);
	String Name(){ return name; };
	String Source();
	/// Returns the source location using relative path.
	String RelativePath();

	/// Returns a list of all triangles in this model.
	List<Triangle> GetTris();

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

	/// Keeps track of active amount of users for this model.
	int users;

	/// Center of mesh (measured from internal structure), calculated after it's been loaded.
	Vector3f centerOfModel;
	/// Radius
	float radius;
	/// AABB
	AxisAlignedBoundingBox aabb;

private:

	/// Triangulized version of the mesh, for rendering!
	Mesh * triangulizedMesh;

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
