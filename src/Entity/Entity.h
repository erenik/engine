// Emil Hedemalm
// 2013-06-07

#ifndef ENTITY_H
#define ENTITY_H

#include "Physics/PhysicsSettings.h"
#include "../MathLib.h"
#include "String/AEString.h"

class AABB;
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
class Message;
class Map;
class CollisionCallback;

enum class TextState;

struct LifeAttribute{
	int HP, maxHP;
};

//
//#define Entity* std::weak_ptr<Entity>

/** An encapsulation Entity for objects, including their corresponding transforms, textures, Mesh, etc.
	An entiy with an ID of 0 can be considered invalid.
*/
class Entity
{
	friend class EntityManager;
	friend class RenderPass;
	friend class PhysicsManager;
	friend class EntityManager;
	friend class BlueprintManager;
	/// Default constructor, only available to the EntityManager!
	Entity(int id = 0);
public:
	/// Default destructor
	~Entity();

	void Initialize();

	/// Removes links to parents/children as needed, prepares for deletion. Take care to call from render/physics thread.
	void RemoveLinks();
	/// If reacting to collisions within the physics thread.,.. pokes all properties about it too.
	virtual void OnCollision(Collision & data);
	/// If reacting to collisions within the main state thread (callbacks).
	virtual void OnCollisionCallback(CollisionCallback * cc);

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

	/// For new control method of simulating/rendering entities per map.
	Map * map;

	/// Getter.
	template<class T>
	T * GetProperty();
	EntityProperty * GetProperty(String byName);
	/// Getter.
	EntityProperty * GetProperty(int byID);

	/// Is then re-directed to the properties in most cases.
	void ProcessMessage(Message * message);

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
	void CreateCompactEntity(CompactEntity* cEntity);
	/// Loads data from the file compact entity format
	void LoadCompactEntityData(CompactEntity* cEntity);

	/// Default material granted to all new objects if a specific one isn't assigned.
	static const Material defaultMaterial;

	/** Buffers this entities' models into graphics memory.
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	void Bufferize();
	/** Rendering method using legacy code
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	//void RenderOld(GraphicsState & state);
	/** Rendering method
		Should only be used by the graphics manager. USE WITH CAUTION.
	*/
	void Render(GraphicsState & graphicsState);
	// Renders the text associated with this entity, format and location depends on variables set in GraphicsProperty.
	void RenderText(GraphicsState& graphicsState);


	/// Gets velocity, probably from the PhysicsState
	Vector3f Velocity();
	/// Mostly just checking graphics->visibility and if registered for rendering.
	bool IsVisible();


	/** Sets position */
	void SetPosition(const Vector3f & position);
	/** Sets position */
	void SetPosition(float x, float y, float z);
	/// New rotation. Should hopefully make old rotatoin system obsolete... maybe :P
	/// Rotates around the globally defined quaternion axis.
	void RotateGlobal(const Quaternion & withQuaternion);
	/// Rotates the Entity
	void Rotate(const Vector3f & rotation);
	/// Quaternion initial rotation.
	void SetRotation(const Quaternion & quat);
	/// Rotation from the default (looking into -Z
	void SetRotation(const Vector3f & rotationFromZMinus1);
	/// Scales all axes to target value.
	void SetScale(float scale);
	/// Sets scale of the entity, updates transform.
	void SetScale(const Vector3f & scale);
	/// Scales the Entity, updates transform.
	void Scale(const Vector3f & scale);
	/// Scales the Entity, updates transform.
	void Scale(float scale);
	/// Translates the Entity
	void Translate(float x, float y, float z);
	/// Translates the Entity   
	void Translate(const Vector3f & translation);

	enum {
		TRANSLATION_ONLY,
		ALL_PARTS,
		ALL_BUT_ROTATION,
	};
	/// Recalculates the transformation matrix. All parts by default. If recursively, will update children (or parents?) recursively upon update.
	/// 0 = Just translation adjustment, 1 = ALL, 2 = Re-build all but rotation. 
	void RecalculateMatrix(int whichParts = ALL_PARTS, bool recursively = false);
	/// If force, recalculates no matter wat.
	void RecalcRotationMatrix(bool force = false);

	/// Recalculates the matrix this entity should have, using given position.
	void RecalculateMatrix(Matrix4f & matrix, Vector3f * position);	
//	static void RecalculateMatrix(Matrix4f & givenMatrix, Vector3f * position, );

	/// Recalculates a transformation matrix using argument vectors for position, rotation and translation.
	static Matrix4f RecalculateMatrix(const Vector3f & position, const Vector3f & rotation, const Vector3f & scale);

	/// Recalculates the radius of the entity, both in the upper level radius as well as the physics-property variable one if applicable.
	void RecalculateRadius();

	/// Returns the center of this entity, determined by position, rotation, and current model.
	Vector3f CenterOfGravityWorldSpace();

	/// Checks with Rotation matrix.
	Vector3f LookAt();
	Vector3f UpVec();
	Vector3f RightVec();

	/// If true, updates all children once this entity is transformed. Default true.
	bool updateChildrenOnTransform;
	/// Recalculated in RecalculateMatrix. Used to get child positions correctly.
	Vector3f worldPosition;

	/// Local position. If no parent, world-positions. If has parent, see worldPosition.
	Vector3f localPosition;
	/// Scale in x,y,z
	Vector3f scale;
	/// Rotation in pitch, yaw, roll (x,y,z)
	Vector3f rotation;

	/// Rotation matrix that is calculated while transforming.
	Matrix4f rotationMatrix;
	/// The transformation matrix that is applied when rendering. Do note that RecalculateMatrix has to be called to update this.
	Matrix4f transformationMatrix;
	/** Transform matrix used to render this entity. For default entities it will point to the transformationMatrix above, but for all entities featuring a GraphicsProperty
		it will point to its 'transform' matrix, in order to combat temporal aliasing.
	*/
	Matrix4f * renderTransform;
	/// Similar to above, used for when temporal aliasing is an issue. Works in combination with camera n stuff.
	Vector3f * renderPosition;
	/// Transforms as calculated if this were not child of any other entity.
	Matrix4f localRotation, localTransform;
	Matrix4f scalingMatrix;
	Matrix4f normalMatrix;

	/// Material to be used for this Entity.
	Material * material;

	/// Bounding volumes, spherical for now!
//	Vector3f absolutePosition;
	/// Radius of the bounding sphere.
	float Radius() const;

	/// o.o Links child and parent for both.
	void AddChild(Entity* child);

	/** Child entities, for example wheels for a bike, etc.
		All child-entities are merely here by relation, and should not be processed (in general) when the parent is processed!
	*/
	List< Entity* > children;
	/// Parent entity. Helps dictate how the transformation-matrix will be calculated.
	Entity* parent;
	bool inheritPositionOnly; // Default false.

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
	List<Texture*> GetTextures(int targetFlags);
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

	/// Default false. If true, the entity shares properties with other entities, and Process should thus not be called for it.
	bool sharedProperties;

	/// Axis-aligned bounding box. <- why pointer? Always used now.
	AABB * aabb;

	/// Texture to be used for this Entity. TODO: Rename to DiffuseMap?
	Texture * diffuseMap;
	/// Normalmap texture for more surface~
	Texture * specularMap;
	/// Normalmap texture for more surface~
	Texture * normalMap;
	/// For "Glow" effects disregarding lighting conditions.
	Texture * emissiveMap;

	TextState textState;

private:
	// For distribution of SharedPtr of self. Assign this upon construction where you called make_shared
	//Entity* selfPtr;

	/// If non-identity scale.
	bool relevantScale;
	/// Used internally.
	Matrix4f preTranslateMat;
	/// Calculated after transform or rotation is done.
	Vector3f lookAt, rightVec, upVec;
	int deletionTimeMs;
};

//Node * createScenegraphNode

class EntityGroup : public List< Entity* > 
{
public:
	virtual ~EntityGroup(){};
	/// Inherit the rest from list.
	String name;
};

#endif
