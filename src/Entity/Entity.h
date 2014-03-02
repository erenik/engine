// Emil Hedemalm
// 2013-06-07

#ifndef ENTITY_H
#define ENTITY_H

#include "Physics/PhysicsSettings.h"
#include "../MathLib.h"
#include "String/AEString.h"

#ifdef USE_QUATERNIONS
#include "MathLib/Quaternion.h"
#endif

class Model;
class Player;
struct Material;
struct GraphicsState;
class Texture;
struct GraphicsProperty;
struct PhysicsProperty;
struct StateProperty;
struct PathfindingProperty;
class EventProperty;
class CompactEntity;
struct Triangle;
/*
#include "GL/glew.h"
#include "Mesh.h"
#include "Texture.h"
*/

struct LifeAttribute{
	int HP, maxHP;
};

/** An encapsulation Entity for objects, including their corresponding transforms, textures, Mesh, etc.
	An entiy with an ID of 0 can be considered invalid.
*/
class Entity {
	friend class EntityManager;
	friend class BlueprintManager;
	/// Default constructor, only available to the EntityManager!
	Entity(int id = 0);
public:
	/// Default destructor
	~Entity();

	/// Optional name
	String name;
	/// Unique identifier for this Entity
	int id;
	/// Wosh.
	int flags;


	/// Contains additional render information for effects or attached entities.
	GraphicsProperty * graphics;
	/// Physics information. This is NOT created by default, but can be automatically attached by the PhysicsManager.
	PhysicsProperty * physics;
	/// Property closely related to both physics and pathfinding/AI/movement/input overall.
	PathfindingProperty * pathfindingProperty;
	/// StateMachine for gameplay
	StateProperty * state;
	/// Containing OnTrigger, OnInteract, On-- etc.
	EventProperty * events;
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
	void RenderOld(GraphicsState &state);
	/** Rendering method
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	void render(GraphicsState &state);

	/// Gets velocity, probably from the PhysicsState
	Vector3f Velocity();

	/** Sets position */
	void position(Vector3f position);
	/** Sets position */
	void position(float x, float y, float z);
	/** Rotates the Entity */
	void rotate(Vector3f rotation);
	/** Sets scale of the entity */
	void SetScale(Vector3f scale);
	/** Scales the Entity */
	void Scale(Vector3f scale);
	/** Scales the Entity */
	void Scale(float scale);
	/** Translates the Entity */
	void translate(float x, float y, float z);
	/** Translates the Entity */
	void translate(Vector3f translation);

	/// Recalculates the transformation matrix
#define recalculateMatrix RecalculateMatrix
	void RecalculateMatrix();

	/// World coordinate position
	Vector3f positionVector;
	/// Scale in x,y,z
	Vector3f scaleVector;
	/// Rotation in pitch, yaw, roll (x,y,z)
	Vector3f rotationVector;

	/// Rotation matrix that is calculated while transforming.
	Matrix4d rotationMatrix;
	/// The transformation matrix that is applied when rendering. Do note that recalculateMatrix has to be called to update this.
	Matrix4f transformationMatrix;

	/// Material to be used for this Entity.
	Material * material;

	/// Bounding volumes, spherical for now!
	Vector3f absolutePosition;
	/// Radius of the bounding sphere.
	float radius;

	/// Child objects, for example wheels for a bike, etc.
	Entity ** child;
	/// Keeps track of the amount of children this node has.
	int children;

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
private:
	/// Texture to be used for this Entity. TODO: Rename to DiffuseMap?
	Texture * diffuseMap;
	/// Normalmap texture for more surface~
	Texture * specularMap;
	/// Normalmap texture for more surface~
	Texture * normalMap;
};

//Node * createScenegraphNode

#endif
