/// Emil Hedemalm
/// 2013-03-01

#ifndef PHYSICS_MESSAGE_H
#define PHYSICS_MESSAGE_H

#include "MathLib.h"
#include "Entity/Entities.h"
// #include "PhysicsLib/Shapes.h"

class Entity;
class Waypoint;
class EstimatorFloat;

//#include "PhysicsManager.h"
//class PhysicsManager;

enum physicsMessages {
	PM_NULL,
	PM_RECALCULATE_PHYSICS_PROPERTIES, // Recalculates physics properties for ALL registered entities.
	PM_SET_GRAVITY,			// Sets gravity vector
	PM_SET_VELOCITY,		// Set velocity of entity.
	PM_CLEAR_ALL_ENTITIES,	// Unregisters all entities.
	PM_UNREGISTER_ALL_ENTITIES = PM_CLEAR_ALL_ENTITIES,
	PM_UNREGISTER_ALL = PM_CLEAR_ALL_ENTITIES,
	PM_REGISTER_ENTITY,		// Registers an entity for physics
	PM_REGISTER_ENTITIES,	// Registers an entity for physics
	PM_UNREGISTER_ENTITY,	// Unregisters an entity from physics
	PM_UNREGISTER_ENTITIES,	// Unregisters target entities from physics
	PM_SET_PHYSICS_TYPE,	// Sets main physics type of target entities (static, dynamic, etc.)
	PM_SET_PHYSICS_SHAPE,	//
	PM_SET,					// General setter function for global physics attributes
	PM_SET_ENTITY,			// General setter function for entity physics attributes
	PM_SLIDE_ENTITY,	// For applying/attaching sliders/estimators for adjusting values over time.
	PM_CLEAR_ESTIMATORS, // For removing existing sliding effects.

	/// For setting waypoint attributes in real-time.
	PM_SET_WAYPOINT,

	/// Wosh!
	PM_APPLY_IMPULSE,
	
	/// For handling springs!
	PM_CREATE_SPRING,

	PM_SET_SPEED,			// Sets simulation speed ratio. Default is 1.0.
	PM_IGNORE_COLLISSIONS,	// Tells the physics manager to not process collissions of any kind until further notice.
	PM_RESET_SETTINGS,		// Resets settings like the ignore-collissions flag, etc.
};

enum physicsTargets{
	PT_NULL_PHYSICS_TARGET,

	// Global stuff
	PT_SIMULATION_SPEED,

	// Parenting
	PT_SET_PARENT,

	// Custom types.
	PT_PHYSICS_INTEGRATOR,
	PT_COLLISION_RESOLVER,
	PT_COLLISION_DETECTOR,

	/// Collisions
	PT_COLLISION_CATEGORY,
	PT_COLLISION_FILTER,

    /// For disabling stuff.
    PT_LOCK_POSITION,

	/// For what rotation system to use when re-calculating the rotation matrix part of the transform
	PT_USE_QUATERNIONS,

	// Floats
	PT_MASS,
	PT_LINEAR_DAMPING,
	PT_ANGULAR_DAMPING,
	PT_POSITION_Y,
	PT_POSITION_X,
	PT_POSITION_Z,

	// Separate float and Vector targets?
	PT_POSITION,
	PT_TRANSLATE,
	PT_SCALE,
	PT_ROTATE,
	PT_SET_POSITION,
	PT_SET_SCALE,
	PT_SET_PRE_TRANSLATE_ROTATION, // In order to use the translation as an arm-extension into a specified rotated direction.
	PT_SET_ROTATION, // In degrees or radians?
	PT_ROTATION_Y, PT_ROTATION_YAW = PT_ROTATION_Y, // For setting the yaw (y) component.
	PT_GRAVITY,
	PT_ACCELERATION, PT_ACCELERATION_MULTIPLIER, // <- Lazy me, but might be good, hm?
	PT_ANGULAR_ACCELERATION,
	PT_VELOCITY,

	// Relative acceleration, meaning acceleration in relation to the entity's current direction vectors (up, left, forward)
	PT_RELATIVE_ACCELERATION, 
	/// Velocity in the entity's current direction vectors (based on rotation)
	PT_RELATIVE_VELOCITY,
	// Relative rotation compared to entity's current direction vectors. 
	// The speed of these rotations will vary with the entity's rate/radius of turns (ROT) (turning rate), current air speed and time.
	// Mainly used for airplanes and similar vehicles.
	PT_RELATIVE_ROTATION,

	PT_RESET_ROTATION, // Non-argument message, resets angle/quaternion for rotation.
	PT_ANGULAR_VELOCITY, 	PT_ROTATIONAL_VELOCITY = PT_ANGULAR_VELOCITY,
	PT_CONSTANT_ROTATION_VELOCITY, 
	PT_CONSTANT_ROTATION_SPEED = PT_CONSTANT_ROTATION_VELOCITY,

	PT_FRICTION,
	PT_RESTITUTION,
	PT_ESTIMATION_MODE, /// For network-synchronization
	PT_ESTIMATION_DELAY, /// For properly setting up interpolation
	PT_ESTIMATION_SMOOTHING_DURATION, /// For extrapolation smoothing

	// Floaturs
	PT_VELOCITY_RETAINED_WHILE_TURNING,
	PT_AIR_DENSITY,
	PT_DEFAULT_DENSITY,
	PT_GRAVITY_MULTIPLIER, // Used when you want an entity to be extra affected or perhaps not at all by gravity (not affecting other entities)

	/// Boolean targets
	/** For pausing/freezing the actions of a single-entity without having to unregister/re-register or stuff. 
		Exactly how a paused entity is to react to collisions is up to the current collision detector and resolver. One may ignore it and another may treat it as being static.
	*/
	PT_PAUSED, 
	PT_COLLISIONS_ENABLED,
	PT_COLLISSION_CALLBACK,
	PT_NO_COLLISSION_RESOLUTION,
	PT_PAUSE_ON_COLLISSION,
	PT_SIMULATION_ENABLED, /// If disabled, no simulation will be done, including collissions, gravity etc. Used for network synchronization for example.
	PT_ESTIMATION_ENABLED, /// For network-synchronization
	PT_FACE_VELOCITY_DIRECTION,

	/// Waypoints, Pathfinding and NavMesh control. Mainly various pointer targets
	PT_ENTITY,
	PT_DESTINATION,
	PT_CURRENT_WAYPOINT,

	// Integer targets,
	PT_INTEGRATOR_TYPE,		// Global
	// Entity integrator targets
	PT_PHYSICS_TYPE,	// using this with PMSetEntity is the same as using PMSetPhysicsType
	PT_PHYSICS_SHAPE,

	PT_MAX_PHYSICS_TARGETS

};

// General message class for all messages not requiring additional arguments.
class PhysicsMessage {
public:
	PhysicsMessage();
	/** Explicitly declared destructor to avoid memory leaks.
		No explicit destructor may skip subclassed variable deallocation!
	*/
	virtual ~PhysicsMessage();
	PhysicsMessage(int messageType);
	virtual void Process();
    int Type() const { return type; };
protected:
	/// Type of message, if relevant
	int type;
};

class PMCreateSpring : public PhysicsMessage {
public:
	/// Creates a default spring between the entities in the list (linearly attached).
	PMCreateSpring(List<Entity*> targetEntities, float springConstant);
	/// Creates a default spring between the entities in the list (linearly attached).
	PMCreateSpring(List<Entity*> targetEntities, float springConstant, float springLength);
	void Process();
private:
	List<Entity*> entities;
	float springConstant;
	float springLength;
};

class PMApplyImpulse : public PhysicsMessage {
public:
    /// Standard 1 second impulse.
    PMApplyImpulse(List<Entity*> targetEntities, const Vector3f & force, const Vector3f & pointInSpace, float duration = 1.0f);
    void Process();
private:
    List<Entity*> entities;
    Vector3f force;
    Vector3f position;
    float duration;
};

class PMSetEntity : public PhysicsMessage {
public:
	// For resets and similar
	PMSetEntity(List<Entity*> targetEntities, int target);
	PMSetEntity(List<Entity*> targetEntities, int target, float value);
	PMSetEntity(List<Entity*> targetEntities, int target, Vector2f value, long long timeStamp = 0);
	PMSetEntity(List<Entity*> targetEntities, int target, const Vector3f & value, long long timeStamp = 0);
	PMSetEntity(List<Entity*> targetEntities, int target, const Quaternion & value, long long timeStamp = 0);
	PMSetEntity(List<Entity*> targetEntities, int target, bool value);
	PMSetEntity(List<Entity*> targetEntities, int target, int value);
	PMSetEntity(List<Entity*> targetEntities, int target, Waypoint * waypoint);
	PMSetEntity(List<Entity*> targetEntities, int target, Entity * referenceEntity);
	void Process();
protected:
	enum dataTypes{
		NULL_TYPE,
		INTEGER, FLOAT,
		BOOLEAN, VECTOR3F,
		VECTOR2F, QUATERNION
	};
	int dataType;
	int target;
	int iValue;
	bool bValue;
	long long timeStamp;
	List<Entity*> entities;
	float fValue;
	Vector3f vec3fValue;
	Vector2f vec2fValue;
	Quaternion qValue;
	Waypoint * waypoint;
	Entity * referenceEntity;
};

class PMSlideEntity : public PhysicsMessage 
{
public:
	PMSlideEntity(List<Entity*> targetEntities, int target, EstimatorFloat * estimatorFloat);
	virtual ~PMSlideEntity();
	virtual void Process();
private:
	Entities targetEntities;
	EstimatorFloat * estimatorFloat;
	int target;
};

class PMClearEstimators : public PhysicsMessage 
{
public:
	PMClearEstimators(Entities entities);
	virtual void Process();
private:
	Entities entities;
};

class Integrator;
class CollisionResolver;
class CollisionDetector;

class PMSet : public PhysicsMessage {
public:
	PMSet(int target, const Vector3f & value);
	PMSet(int target, float value);
	PMSet(int target, bool bValue);
	PMSet(int target, int iValue);
	PMSet(Integrator * integrator);
	PMSet(CollisionResolver * cr);
	PMSet(CollisionDetector * cd);
	virtual void Process();
private:
	enum {
		FLOAT,
		VEC3F,
		INT,
		BOOL,
	};
	int target;
	int dataType;
	int iValue;
	float floatValue;
	bool bValue;
	Vector3f vec3fValue;
	Integrator * i;
	CollisionResolver * cr;
	CollisionDetector * cd;
};

class PMSetWaypoint : public PhysicsMessage {
public:
	PMSetWaypoint(const Vector3f & position, int target, void * value);
	virtual void Process();
private:
	Vector3f position;
	int target;
	void * pValue;
};

class PMSetVelocity : public PhysicsMessage {
public:
	PMSetVelocity(Entity * entity, const Vector3f & newVelocity);
	void Process();
private:
	Entity * entity;
	Vector3f newVelocity;
};

class PMRegisterEntity : public PhysicsMessage {
public:
	PMRegisterEntity(Entity * entity);
	void Process();
private:
	Entity * entity;
};

class PMRegisterEntities : public PhysicsMessage {
public:
	PMRegisterEntities(List<Entity*> targetEntities);
	void Process();
private:
	List<Entity*> entities;
};

class PMUnregisterEntity : public PhysicsMessage {
public:
	PMUnregisterEntity(Entity * entity);
	void Process();
private:
	Entity * entity;
};


class PMUnregisterEntities : public PhysicsMessage {
public:
	PMUnregisterEntities(List<Entity*> targetEntities);
	void Process();
private:
	List<Entity*> entities;
};

class PMSetPhysicsType : public PhysicsMessage {
public:
	PMSetPhysicsType(List<Entity*> targetEntities, int physicsType);
	void Process();
private:
	List<Entity*> entities;
	int physicsType;
};

class PMSetPhysicsShape : public PhysicsMessage {
public:
	PMSetPhysicsShape(List<Entity*> targetEntities, int physicsShape);
	void Process();
private:
	List<Entity*> entities;
	int physicsShape;
};

class PMSetSpeed : public PhysicsMessage {
public:
	PMSetSpeed(float speedMultiplier) : PhysicsMessage(PM_SET_SPEED), speedMultiplier(speedMultiplier){};
	void Process();
private:
	float speedMultiplier;
};

;;;
#endif
