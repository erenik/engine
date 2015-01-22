// Emil Hedemalm
// 2013-06-07

#ifndef ENTITY_H
#define ENTITY_H

#include "Physics/PhysicsSettings.h"
#include "../MathLib.h"
#include "String/AEString.h"

struct Collision;
class Model;
class Player;
struct Material;
class GraphicsState;
class Texture;
struct GraphicsProperty;
struct PhysicsProperty;
class PathfindingProperty;
class ScriptProperty;
class CompactEntity;
class Triangle;
class EntityProperty;
class Camera;
/*
#include "GL/glew.h"
#include "Mesh/Mesh.h"
#include "Texture.h"
*/

class AABB;

struct LifeAttribute{
	int HP, maxHP;
};

/** An encapsulation Entity for objects, including their corresponding transforms, textures, Mesh, etc.
	An entiy with an ID of 0 can be considered invalid.
*/
class Entity 
{
	friend class EntityManager;
	friend class BlueprintManager;
	/// Default constructor, only available to the EntityManager!
	Entity(int id = 0);
public:
	/// Default destructor
	~Entity();

	/// If reacting to collisions,.. pokes all properties about it too.
	virtual void OnCollision(Collision & data);

	/// Fetches an AABB encapsulating this entity, including any children?
	AABB * GetAABB();

	/// Deallocates additional points as needed.
	void Delete();

	/// Optional name
	String name;
	/// Unique identifier for this Entity
	int id;
	/// Wosh.
	int flags;

	/// Getter.
	template<class T>
	T * GetProperty();
	EntityProperty * GetProperty(String byName);
	/// Getter.
	EntityProperty * GetProperty(int byID);


	/// Contains additional render information for effects or attached entities.
	GraphicsProperty * graphics;
	/// Physics information. This is NOT created by default, but can be automatically attached by the PhysicsManager.
	PhysicsProperty * physics;
	/// Property closely related to both physics and pathfinding/AI/movement/input overall.
	PathfindingProperty * pathfindingProperty;

	/// StateMachines for gameplay?
	List<EntityProperty*> properties;
	
	/// Containing OnTrigger, OnInteract, On-- etc.
	ScriptProperty * scripts;
	/// Survival attributes if destroyable/killable  <-- Build into your StateProperties!
///	LifeAttribute * life;

	/// Creates a compact entity out of this Entity object
	void CreateCompactEntity(CompactEntity * cEntity);
	/// Loads data from the file compact entity format
	void LoadCompactEntityData(CompactEntity * cEntity);

	/// Default material granted to all new objects if a specific one isn't assigned.
	static const Material defaultMaterial;

	/** Buffers this entities' models into graphics memory.
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	void Bufferize();
	/** Rendering method using legacy code
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	void RenderOld(GraphicsState & state);
	/** Rendering method
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	void Render(GraphicsState & graphicsState);

	/// Gets velocity, probably from the PhysicsState
	Vector3f Velocity();

	/** Sets position */
	void SetPosition(Vector3f position);
	/** Sets position */
	void SetPosition(float x, float y, float z);
	/// New rotation. Should hopefully make old rotatoin system obsolete... maybe :P
	/// Rotates around the globally defined quaternion axis.
	void RotateGlobal(Quaternion withQuaternion);
	/** Rotates the Entity */
	void Rotate(Vector3f rotation);
	/// Quaternion initial rotation.
	void SetRotation(Quaternion quat);
	/// Rotation from the default (looking into -Z
	void SetRotation(Vector3f rotationFromZMinus1);
	/** Sets scale of the entity */
	void SetScale(Vector3f scale);
	/** Scales the Entity */
	void Scale(Vector3f scale);
	/** Scales the Entity */
	void Scale(float scale);
	/** Translates the Entity */
	void Translate(float x, float y, float z);
	/** Translates the Entity */
	void Translate(Vector3f translation);

	/// Recalculates the transformation matrix
	void RecalculateMatrix();
	/// Recalculates a transformation matrix using argument vectors for position, rotation and translation.
	static Matrix4f RecalculateMatrix(Vector3f & position, Vector3f & rotation, Vector3f & scale);

	/// Recalculates the radius of the entity, both in the upper level radius as well as the physics-property variable one if applicable.
	void RecalculateRadius();

	/// Returns the center of this entity, determined by position, rotation, and current model.
	Vector3f CenterOfGravityWorldSpace();

	/// Recalculated in RecalculateMatrix. Used to get child positions correctly.
	Vector4f worldPosition;

	/// Relative world coordinate position
	Vector3f position;
	/// Scale in x,y,z
	Vector3f scale;
	/// Rotation in pitch, yaw, roll (x,y,z)
	Vector3f rotation;

	/// Rotation matrix that is calculated while transforming.
	Matrix4f rotationMatrix;
	/// The transformation matrix that is applied when rendering. Do note that RecalculateMatrix has to be called to update this.
	Matrix4f transformationMatrix;

	/// Material to be used for this Entity.
	Material * material;

	/// Bounding volumes, spherical for now!
	Vector3f absolutePosition;
	/// Radius of the bounding sphere.
	float radius;

	/** Child entities, for example wheels for a bike, etc.
		All child-entities are merely here by relation, and should not be processed (in general) when the parent is processed!
	*/
	List<Entity*> children;
	/// Parent entity. Helps dictate how the transformation-matrix will be calculated.
	Entity * parent;

	/// Status, for whether it's part of rendering, physics, etc.
	bool registeredForRendering;
	bool registeredForPhysics;
	bool flaggedForDeletion;

	/// Sets name for this entity.
	bool SetName(const char * name);
	/// Sets model to be used by this entity.
	bool SetModel(Model * model);
	/** Sets texture to be used by this entity.
		Multiple targets can be set with binary and |, and 0xFFFFFFFF will replace all textures.
		Should only be used via the Graphics message GMSetEntityTexture! Be warned!
		DIFFUSE_MAP			0x0000001
		SPECULAR_MAP		0x0000002
		NORMAL_MAP			0x0000004
	*/
	bool SetTexture(int target, Texture * texture);
	/// Returns current texture bound to the target.
	Texture * GetTexture(int target);
	/// Returns path for current texture's source.
	String GetTextureSource(int target);

	/// Returns all faces of the entity, transformed with it's current transformation. COSTLY FUNCTION.
	List<Triangle> GetTris();

	/// Model with included meshes/subdivs/LoDs, etc.
	Model * model;
	/// Owner, or stuff.
	Player * player;
	/// If a camera is currently tracking this entity, it should be stored here. This will be set from the graphics-thread by the CameraManager ALONE!
	Camera * cameraFocus;

	/// Used in Recalc of AABB. Set to true if they have been.
	bool hasRescaled;
	bool hasRotated;

private:

	int64 deletionTimeMs;
	/// Texture to be used for this Entity. TODO: Rename to DiffuseMap?
	Texture * diffuseMap;
	/// Normalmap texture for more surface~
	Texture * specularMap;
	/// Normalmap texture for more surface~
	Texture * normalMap;

	/// Axis-aligned bounding box.
	AABB * aabb;
};

template<class T>
T * Entity::GetProperty()
{
	EntityProperty * prop = NULL;
	for (int i = 0; i < properties.Size(); ++i)
	{
		prop = properties[i];
		if (prop->GetID() == T::ID())
			return (T *) prop;
	}
	return (T *) prop;
}


//Node * createScenegraphNode

#endif
