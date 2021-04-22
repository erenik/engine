// Emil Hedemalm
// 2013-03-17

#ifndef PHYSICS_PROPERTY_H
#define PHYSICS_PROPERTY_H

#include "PhysicsLib.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsSettings.h"

class Estimator;
class PhysicsOctree;
struct CompactPhysics;
class PhysicsMesh;
class AABBSweepNode;
class AABBSweepAxis;
struct Contact;
class Spring;
class EntityPhysicsEstimator;
class AABB;
class OBB;
class Force;
class Entity;

enum collissionStates {
    COLLISSION_STATE_NULL,
    AABB_IDLE,
    AABB_INTERSECTING,
    COLLIDING,
};

///	Flags for various states
namespace CollisionState {
	/** In rest, applied after collissions if the resultant velocity is close to 0, acceleration is 0,
		and the collission normal was close to or exactly (0,-1,0) (only gravitation affecting).
		Entities in rest are not applied gravitation. Any collission with other results resets this flag.
	*/
	const int IN_REST	= 0x00000001;
#define AT_REST IN_REST
	/** Colliding, applied if a collission occurs. This is reset if no collissions occur next iteration. */
	const int COLLIDING = 0x00000002;
};

/// For the locks (PhysicsProperty::locks)
#define POSITION_LOCKED   0x00000001

// A property attachable to entities.
struct PhysicsProperty {
	/// Use the physicsmanager to properly handle physicsproperties!
	friend class PhysicsManager;
	friend class PhysicsOctree;
public:
	/// Default constructor
	PhysicsProperty();
	~PhysicsProperty();
	/// Copy constructor
	PhysicsProperty(const PhysicsProperty& other);
	PhysicsProperty(const PhysicsProperty* other);
	PhysicsProperty(const CompactPhysics * compactPhysics);
	/// Set default values.
	void Nullify();

	/// Sets mass, re-calculates inverse mass.
	void SetMass(float mass);
	/// Calculates inertia tensor matrix and its inversion.
	void CalculateInertiaTensor();

	/// Calculated using velocity, mass and angular velocity..?
	float KineticEnergy();
	/// Applies target impulse at specified position to this entity. Impulse in Ns (kg*m/s)
	void ApplyImpulse(const Vector3f & impulse, const Vector3f & position);

	/// Updates physical radius, Bounding box size, etc. Should be called from the physics thread only!
	void UpdateProperties(Entity* owner);

	void SetLinearDamping(float newD);
	// Sets if the entity should only collide using the Tri/Quads planes or including the edges and points (corners) too. Default is false. True for optimization may yield bugs.
	void SetPlaneCollisionsOnly(bool bValue);

	/// Sets state to IN_REST. Nullifies velocity.
	void Sleep();
	/// Remove IN_REST. 
	void Activate();

	/** Default true. All entities with this have their matrices updated each physics frame, 
		in order for collision simulation, etc. to work properly. As an optimization feature
		entities may be flagged false here, making them only have their matrices updated once 
		at the end of all physics simulation (before the next graphics- frame).

		It can also be used by those entities which have some simulation and collision interaction,
		but use a simplified collision shape/type/mesh.
	*/
	bool fullyDynamic;

	/** Minimum amount of time between one collision being resolved to the next. Added as an alternative to sorting 
		collisions, and will also help with applying friction over time (maybe). Hmm.. Default.. 5 ms?
	*/
	int minCollisionIntervalMs;
	static int defaultMinCollisionIntervalMs; // Default setting. Default 5. Override in Application::SetApplicationDefaults?
	/// Used to help the above.
	int64 lastCollisionMs;

	/// Basic physics type, defined where? STATIC DYNAMIC KINEMATIC anyway.
	int type;
	/** Collision detection shape type.
		Different types infer using various methods of storage for the actual shape type (non-transformed).
		AABB uses the entity's raw AABB (no transforms necessary?).
		SPHERE uses the physical radius and entity position.

	*/
	int shapeType;
	/// Pointer to the mesh of type defined in physicsShape. (e.g. Plane, Sphere, Mesh, etc.)
	void * shape;

	/// For separating categories and filters.
	/// 0x0000001 by default.
	int collisionCategory;
	/// 0x0000001 by default. is binary AND-ed to the opposing entities' category to see if a collision should occur or not.
	int collisionFilter;
	// the defaults, 1 both.
	static int defaultCollisionCategory;
	static int defaultCollisionFilter;


	/// See state enumerations above. TODO: Consider making the state-variable below private.
	void SetPhysicsState(int state);
	/**	Flags for various states
		0x00000001	- In rest, applied after collissions if the resultant velocity is close to 0, acceleration is 0,
					and the collission normal was close to or exactly (0,-1,0) (only gravitation affecting).
					Entities in rest are not applied gravitation. Any collission with other results resets this flag.
		0x00000002	- Colliding, applied if a collission occurs. This is reset if no collissions occur next iteration.
	*/
	int state;

    /** Boolean locks
        0x00000001  - Lock position - Does what exactly?
    */
	int locks;
	/// If true, this means that this entity's movement is bound to follow (strictly or not) the active navmesh provided by the WaypointManager.
	bool boundToNavMesh;

	/// Pointer to the physicsOctree node where this entity is currently positioned.
	PhysicsOctree * octreeNode;
	/// Flag for if a CollisionShapeOctree should be used when doing collission tests with the mesh.
	bool usesCollisionShapeOctree;

	/// Flaggetiflag o-o;
	bool collisionsEnabled;
	// Default.. false?
	bool useQuaternions;
	static bool defaultUseQuaternions;

	/// Default true. Set to false if giving entity's custom radii which should not change dynamically.
	bool recalculatePhysicalRadius;
	// Current physical properties, affected by a number of properties elsewhere in Entity.
	float physicalRadius;

	/// Damping applied on a per-second basis for this entity only.
	float linearDamping,  
		linearDampingPerPhysicsFrame;
	// Default usually 0.99, change to 1.0 for games which have stricter controls/custom integrators.
	static float defaultLinearDamping; 
	float angularDamping,
		angularDampingPerPhysicsFrame;

	/** Obsolete! All positional updates should use the entity's own position vector.
		If a "center of mass" is wanted later on, name it "centerOfMass" or something then...!
		By default all centers of mass should be at 0,0,0.
	*/
//	Vector3f physicalPosition;

    /// Coordinates for axis-aligned bounding box (if used)
    OBB * obb;
	// 6 nodes. for XYZ start and end each.
    List<AABBSweepNode*> aabbSweepNodes;
	List<AABBSweepAxis*> axes;

    /// NOTE: Position, x(t), is defined in the base Entity-class!

    /// Orientatin quaternion, q(t),
    Quaternion orientation;
	Quaternion preTranslateRotationQ;
	/// Angular velocity as expressed by a quaternion! (refers to global rotations) <- there is no such thing.
	// Use the angularAcceleration or angularForce if so.
//	Quaternion angularVelocityQuaternion;

    /// Orientation, R(t),  (or rotation for the noobs (me))
    Matrix3f orientationMatrix;

	/// If true, rotation should follow velocity (or relative velocity, at least).
	bool faceVelocityDirection;

    /// More ;___;
    /// Linear momentum, or P(t)
    /// Mass * velocity
    /// d/dt P(t) = F(t)
    Vector3f linearMomentum;
    /// Total force, F(t)
    /// Should be reset at the beginning of each frame.
    Vector3f totalForce;
    List<Force*> forces;

	/// Default false. If true, uses forces, momentum, etc.
	bool useForces;

	/// Moment of inertia, for calculating rotational kinetic energy. It is a scalar (kg*m^2) compared to the matrices.
	float momentOfInertia;
    /// I(body), Technically a 3x3 matrix, specifies how mass is ditributed in the body.
    /// In runtime, I(t) is calculated by R(t)I(body)R(t)^t   (last R(t) is transposed)
    /// The initial version is I(t), whislt the other two are I(body) and I(body)^-1
	Matrix3f inertiaTensor;
	Matrix3f inertiaTensorInverted;
    Matrix3f inertiaTensorBody;
    Matrix3f inertiaTensorBodyInverted;
    bool inertiaTensorCalculated;
    /// Angular momentum, or L(t)
    /// Calculated via: InteriaTensorMatrix * angularVelocity?
    /// d/dt of AngularMomentum = total torque τ(t)
    Vector3f angularMomentum;
    /// Torque, τ(t), is the sum of all angular forces that cause rotational(angular) momentum.
    Vector3f totalTorque;
    List<Force*> torques;

    /// See enum CollisionStates above
    int collissionState;
	/// Owner of this property.
	Entity* owner;
	/// List of (probably) resting contacts which are needed to invalidate all objects in rest later on.
	List<Contact*> contacts;
	/// List of actively connected springs.
	List<Spring*> springs;

	/// Applies damping to both linear and angular momentum.
	void ApplyDamping(float ratio);
	/// Calculates Linear and Angular velocity based on said momentum and other necessary variables.
	void CalculateVelocity();
	/// Saves away current linear/angular velocity to the their 'previous' variable counterparts.
	void StorePreviousVelocities();

	// Speeds - All velocities are global, whilst the accelerations are set in local space!
	/// LinearVelocity, v(t) = P(t) / M (= linear momentum divided by mass)
	Vector3f velocity;
	/// Velocity relative to current entity rotation. 
	Vector3f relativeVelocity;
	/// Used internally by the intergration system.
	Vector3f currentVelocity;
	Vector3f acceleration;
	/// Relative acceleration in the entity's current right/up/forward vector directions.
	Vector3f relativeAcceleration;
	/// Default false. If true, requires a collision with ground for relativeAcceleration to be added.
	bool requireGroundForLocalAcceleration;
	/// If requireGroundForLocalAcceleration is true, this specifies the amount of milliseconds within which a ground collision must have been detected.
	int isOnGroundThresholdMs;
	int64 lastGroundCollisionMs;
	/** Relative rotation compared to entity's current direction vectors. */
	Vector3f relativeRotationalVelocity;
	/// Angular velocity, ω(t), see Physically Based Modelling - David Baraff
	/// ω(t) = I(t)^-1 * L(t)   (inverse inertia tensor times angular momentum)
    /// Direction gives the axis about which the body is spinning,
    /// quantity (length) specifies spin velocity (in revolutions per time)!
    Vector3f angularVelocity;
	/// Global angular acceleration.
	Vector3f angularAcceleration;
	/// Angular acceleration relative to entity forward, up, right vectors.
	Vector3f relativeAngularAcceleration;

	/// Angular velocity, may only be changed and set with CONSTANT_ANGULAR_VEOCITY
	Vector3f constantAngularVelocity;

	/// Previous values, stored on a per-need basis (like in collission detection debugging...)
	Vector3f previousVelocity, previousAngularVelocity;

	/// Determines how much the current velocity changes in the local Z-axis when rotating with angularVelocity.
	float velocityRetainedWhileRotating;

	PhysicsMesh * physicsMesh;

	// Weights and such
#ifdef USE_MASS
	float mass;
	/// Will probably be used more. Set to 0 to simulate infinite mass.
	float inverseMass;
	float volume;
	float density;
#endif

	/// For special cases of floating, etc.
	float gravityMultiplier;
	/** Ratio at which momentum along the collission normal is contained (bounce)
		Default: 0.15f (15% bounce velocity is retained)
	*/
	float restitution;
	/** Ratio at which momentum in tangent to the collission is reduced upon contact ("real" friction)
		Default: 0.1f (-10% tangent speed upon collission)
	*/
	float friction;
	// Rotational
	// Quaternion + Matrices

	/// Set to any value as defined in Collision.h's enum "collisionCallbackRequirements"
	/// Paused o-o
	bool paused;
	/// Booleans.
	bool onCollision; // if OnCollision should be processed for this entity.
	bool collisionCallback; // If a CollisionCallback message should be generated.
	/// If non-negative, defiens max callbacks. Useful for things as projectiles which should only collide once (optimization). Default unlimited (-1) 
	int maxCallbacks;
	float collisionCallbackRequirementValue;
	/// Flagging this will make all collissions resolve as if there had been no collission at all. Should be coupled with the collisionCallback variable.
	bool noCollisionResolutions;
	/// For enabling custom (probably network-)based estimation. The Estimator* will then be checked and should be non-NULL.
	bool estimationEnabled;
	EntityPhysicsEstimator * estimator; 
	/// For disabling standard simulation. Default: true
	bool simulationEnabled;

	/// Dynamically created estimators. Delete when finished processing?
	List<Estimator*> estimators;
	/// Smoothing multiplier applied each frame (per second). Default defaultVelocitySmoothing.
	float velocitySmoothing;
	static float defaultVelocitySmoothing; // Default 0.2
	Vector3f smoothedVelocity;

};

#endif

