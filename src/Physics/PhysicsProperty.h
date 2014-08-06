// Emil Hedemalm
// 2013-03-17

#ifndef PHYSICS_PROPERTY_H
#define PHYSICS_PROPERTY_H

#include "PhysicsLib.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsSettings.h"

class PhysicsOctree;
struct CompactPhysics;
class PhysicsMesh;
struct AABBSweepNode;
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
namespace PhysicsState {
	/** In rest, applied after collissions if the resultant velocity is close to 0, acceleration is 0,
		and the collission normal was close to or exactly (0,-1,0) (only gravitation affecting).
		Entities in rest are not applied gravitation. Any collission with other results resets this flag.
	*/
	const int IN_REST	= 0x00000001;
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
	void ApplyImpulse(Vector3f impulse, Vector3f position);

	/// Updates physical radius, Bounding box size, etc. Should be called from the physics thread only!
	void UpdateProperties(Entity * owner);

	/// Basic physics type, defined where? STATIC DYNAMIC KINEMATIC anyway.
	int type;
	/// Collision detection shape type
	int physicsShape;
	/// Pointer to the mesh of type defined in physicsShape. (e.g. Plane, Sphere, Mesh, etc.)
	void * shape;

	/// For separating categories and filters.
	/// 0x0000001 by default.
	int collisionCategory;
	/// 0x0000001 by default. is binary AND-ed to the opposing entities' category to see if a collision should occur or not.
	int collisionFilter;

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
	bool collissionsEnabled;
	// Default.. false?
	bool useQuaternions;

	// Current physical properties, affected by a number of properties elsewhere in Entity.
	float physicalRadius;

	/// Damping applied on a per-second basis for this entity only.
	float linearDamping;

	/** Obsolete! All positional updates should use the entity's own position vector.
		If a "center of mass" is wanted later on, name it "centerOfMass" or something then...!
		By default all centers of mass should be at 0,0,0.
	*/
//	Vector3f physicalPosition;

    /// Coordinates for axis-aligned bounding box (if used)
    AABB * aabb;
    OBB * obb;
    AABBSweepNode * aabbSweepNodes[2];

    /// NOTE: Position, x(t), is defined in the base Entity-class!

    /// Orientatin quaternion, q(t),
    Quaternion orientation;
	/// Angular velocity as expressed by a quaternion! (refers to global rotations)
	Quaternion angularVelocityQuaternion;

    /// Orientation, R(t),  (or rotation for the noobs (me))
    Matrix3f orientationMatrix;

    /// More ;___;
    /// Linear momentum, or P(t)
    /// Mass * velocity
    /// d/dt P(t) = F(t)
    Vector3f linearMomentum;
    /// Total force, F(t)
    /// Should be reset at the beginning of each frame.
    Vector3f totalForce;
    List<Force*> forces;

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
	Entity * owner;
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
	Vector3f acceleration;
	/// Relative acceleration in the entity's current right/up/forward vector directions.
	Vector3f relativeAcceleration;
	/** Relative rotation compared to entity's current direction vectors. 
		The speed of these rotations will vary with the entity's rate/radius of turns (ROT) (turning rate), current air speed and time.
		Mainly used for airplanes and similar vehicles.	*/
	Vector3f relativeRotation;
	/// Angular velocity, ω(t), see Physically Based Modelling - David Baraff
	/// ω(t) = I(t)^-1 * L(t)   (inverse inertia tensor times angular momentum)
    /// Direction gives the axis about which the body is spinning,
    /// quantity (length) specifies spin velocity (in revolutions per time)!
    Vector3f angularVelocity;
	Vector3f angularAcceleration;

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

	/// Set to any value as defined in Collision.h's enum "collissionCallbackRequirements"
	/// Paused o-o
	bool paused;
	int collissionCallback;
	float collissionCallbackRequirementValue;
	/// Flagging this will make all collissions resolve as if there had been no collission at all. Should be coupled with the collissionCallback variable.
	bool noCollisionResolutions;
	/// For enabling custom (probably network-)based estimation. The Estimator* will then be checked and should be non-NULL.
	bool estimationEnabled;
	EntityPhysicsEstimator * estimator; 
	/// For disabling standard simulation. Default: true
	bool simulationEnabled;
};

#endif

