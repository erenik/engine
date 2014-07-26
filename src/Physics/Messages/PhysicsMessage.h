/// Emil Hedemalm
/// 2013-03-01

#ifndef PHYSICS_MESSAGE_H
#define PHYSICS_MESSAGE_H

#include "MathLib.h"
#include "Entity/Entity.h"
#include "Selection.h"
#include "PhysicsLib/Shapes.h"
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
	NULL_PHYSICS_TARGET,

	// Global stuff
	SIMULATION_SPEED,

	// Custom types.
	PHYSICS_INTEGRATOR,
	COLLISION_RESOLVER,
	COLLISION_DETECTOR,

	/// Collisions
	COLLISION_CATEGORY,
	COLLISION_FILTER,

    /// For disabling stuff.
    LOCK_POSITION,

	// Floats
	MASS,
	LINEAR_DAMPING,
	ANGULAR_DAMPING,

	// Separate float and Vector targets?
	POSITION,
	POSITION_Y,
	POSITION_X,
	TRANSLATE,
	SCALE,
	ROTATE,
	SET_POSITION,
	SET_SCALE,
	SET_ROTATION, // In degrees or radians?
	GRAVITY,
	ACCELERATION, ACCELERATION_MULTIPLIER, // <- Lazy me, but might be good, hm?
	ANGULAR_ACCELERATION,
	VELOCITY,

	// Relative acceleration, meaning acceleration in relation to the entity's current direction vectors (up, left, forward)
	RELATIVE_ACCELERATION, 
	// Relative rotation compared to entity's current direction vectors. 
	// The speed of these rotations will vary with the entity's rate/radius of turns (ROT) (turning rate), current air speed and time.
	// Mainly used for airplanes and similar vehicles.
	RELATIVE_ROTATION,

	RESET_ROTATION, // Non-argument message, resets angle/quaternion for rotation.
	ANGULAR_VELOCITY,
	CONSTANT_ROTATION_VELOCITY, 
	CONSTANT_ROTATION_SPEED = CONSTANT_ROTATION_VELOCITY,

	FRICTION,
	RESTITUTION,
	ESTIMATION_MODE, /// For network-synchronization
	ESTIMATION_DELAY, /// For properly setting up interpolation
	ESTIMATION_SMOOTHING_DURATION, /// For extrapolation smoothing

	// Floaturs
	VELOCITY_RETAINED_WHILE_TURNING,
	AIR_DENSITY,
	DEFAULT_DENSITY,
	GRAVITY_MULTIPLIER, // Used when you want an entity to be extra affected or perhaps not at all by gravity (not affecting other entities)

	/// Boolean targets
	COLLISIONS_ENABLED,
	COLLISSION_CALLBACK,
	NO_COLLISSION_RESOLUTION,
	PAUSE_ON_COLLISSION,
	SIMULATION_ENABLED, /// If disabled, no simulation will be done, including collissions, gravity etc. Used for network synchronization for example.
	ESTIMATION_ENABLED, /// For network-synchronization

	/// Waypoint: Pointer targets
	ENTITY,
	DESTINATION,

	// Integer targets,
	INTEGRATOR_TYPE,		// Global
	// Entity integrator targets
	PHYSICS_TYPE,	// using this with PMSetEntity is the same as using PMSetPhysicsType
	PHYSICS_SHAPE,

	MAX_PHYSICS_TARGETS

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
    PMApplyImpulse(List<Entity*> targetEntities, Vector3f force, Vector3f pointInSpace, float duration = 1.0f);
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
	PMSetEntity(int target, List<Entity*> targetEntities);
	PMSetEntity(int target, List<Entity*> targetEntities, float value);
	PMSetEntity(int target, List<Entity*> targetEntities, Vector2f value, long long timeStamp = 0);
	PMSetEntity(int target, List<Entity*> targetEntities, Vector3f value, long long timeStamp = 0);
	PMSetEntity(int target, List<Entity*> targetEntities, Quaternion value, long long timeStamp = 0);
	PMSetEntity(int target, List<Entity*> targetEntities, bool value);
	PMSetEntity(int target, List<Entity*> targetEntities, int value);
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
};

class Integrator;
class CollisionResolver;
class CollisionDetector;

class PMSet : public PhysicsMessage {
public:
	PMSet(int target, float value);
	PMSet(int target, bool bValue);
	PMSet(int target, int iValue);
	PMSet(Integrator * integrator);
	PMSet(CollisionResolver * cr);
	PMSet(CollisionDetector * cd);
	virtual void Process();
private:
	int target;
	int iValue;
	float floatValue;
	bool bValue;
	Integrator * physicsIntegrator;
	CollisionResolver * cr;
	CollisionDetector * cd;
};

class PMSetWaypoint : public PhysicsMessage {
public:
	PMSetWaypoint(Vector3f position, int target, void * value);
	virtual void Process();
private:
	Vector3f position;
	int target;
	void * pValue;
};


class PMSetGravity : public PhysicsMessage {
public:
	PMSetGravity(Vector3f newGravity);
	void Process();
private:
	Vector3f newGravity;
};

class PMSetVelocity : public PhysicsMessage {
public:
	PMSetVelocity(Entity * entity, Vector3f newVelocity);
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
