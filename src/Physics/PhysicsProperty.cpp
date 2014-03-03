// Emil Hedemalm
// 2013-03-24

#include "PhysicsOctree.h"
#include "PhysicsProperty.h"
#include "CompactPhysics.h"
#include "Model.h"
#include "Contact/Contact.h"

PhysicsProperty::PhysicsProperty(){
	Nullify();
};

PhysicsProperty::~PhysicsProperty() {
	/// Contacts will be deleted in the physics manager!
};

/// Copy constructor
PhysicsProperty::PhysicsProperty(const PhysicsProperty& other) {
	Nullify();
	type = other.type;
	physicsShape = other.physicsShape;
	usesCollisionShapeOctree = other.usesCollisionShapeOctree;
	shape = other.shape;
#ifdef USE_MASS
	mass = other.mass;
	volume = other.volume;
	density = other.density;
#endif
//	octreeNode = NULL;
//	physicalRadius = 0;
	restitution = other.restitution;
	friction = other.friction;
	collissionCallback = other.collissionCallback;
	collissionCallbackRequirementValue = other.collissionCallbackRequirementValue;
	collissionsEnabled = other.collissionsEnabled;
	noCollissionResolutions = other.noCollissionResolutions;
	physicsMesh = other.physicsMesh;
}

PhysicsProperty::PhysicsProperty(const CompactPhysics * compactPhysics){
	Nullify();
	type = compactPhysics->type;
	physicsShape = compactPhysics->physicsShape;
//	usesCollisionShapeOctree = false;
	/// Shape will be generated right after this is created!
/*	shape = NULL;
	physicsMesh = NULL;
	*/
	state = compactPhysics->state;
	physicalRadius = compactPhysics->physicalRadius;
	velocity = compactPhysics->velocity;
	acceleration = compactPhysics->acceleration;
#ifdef USE_MASS
	mass = compactPhysics->mass;
	volume = compactPhysics->volume;
	density = compactPhysics->density;
#endif
//	octreeNode = NULL; // Node will assigned after being registered.
	restitution = compactPhysics->restitution;
	friction = compactPhysics->friction;
	collissionCallback = compactPhysics->collissionCallback;
	collissionCallbackRequirementValue = compactPhysics->collissionCallbackRequirementValue;
	collissionsEnabled = compactPhysics->collissionsEnabled;
	noCollissionResolutions = compactPhysics->noCollissionResolutions;
}

/// Set default values.
void PhysicsProperty::Nullify(){
    locks = 0;
    inertiaTensorCalculated = false;
	type = PhysicsType::NULL_TYPE;
	physicsShape = ShapeType::NULL_TYPE;
	usesCollisionShapeOctree = false;
	shape = NULL;
#ifdef USE_MASS
	mass = volume = density = 1.0f;
	inverseMass = 1.0f;
#endif
	octreeNode = NULL;
	physicalRadius = 0;
	state = 0;
	restitution = 0.15f;
	friction = 0.01f;
	collissionCallback = IMPACT_VELOCITY;
	collissionCallbackRequirementValue = 1.0f;
	collissionsEnabled = true;
	noCollissionResolutions = false;
	physicsMesh = NULL;
	velocityRetainedWhileRotating = 0.0f;
	aabbSweepNodes[0] = aabbSweepNodes[1] = NULL;
	owner = NULL;
	momentOfInertia = 0;
	boundToNavMesh = false;	
	simulationEnabled = true;
	/// For enabling custom (probably network-)based estimation. The Estimator* will then be checked and should be non-NULL.
	estimationEnabled = false;
	estimator = NULL; 
	
}

/// Calculated using velocity, mass and angular velocity..?
float PhysicsProperty::KineticEnergy(){
	/// Ref: http://en.wikipedia.org/wiki/Kinetic_energy
	/// http://en.wikipedia.org/wiki/Rotational_energy
	float linearEnergy = velocity.DotProduct(velocity) * mass * 0.5f;
	float angularKineticEnergy = momentOfInertia * angularVelocity.DotProduct(angularVelocity) * 0.5f;
	float totalKineticEnergy = linearEnergy + angularKineticEnergy;
	return totalKineticEnergy;
}

/// Applies target impulse at specified position to this entity. Impulse in Ns (kg*m/s)
void PhysicsProperty::ApplyImpulse(Vector3f impulse, Vector3f position){
	/// Static objects don't apply anything anyway.
	if (inverseMass == 0)
		return;
	/// Give it an increase to the linear momentum.
	/// Impulses translate directory into change in linear momentum.
	Vector3f deltaLinearMomentum = impulse;
	linearMomentum += deltaLinearMomentum;

	/// Give it an increase to the angular momentum.
	Vector3f centerToPosition = position - obb.position;
	Vector3f crossProduct = -centerToPosition.CrossProduct(impulse);
	/// Try only cross product..?
//	Vector3f deltaAngularMomentum = inertiaTensorInverted.product(crossProduct);

	angularMomentum += crossProduct;


	/// Testing: 
	/// Impulseive torque (u),  Inertiatensor (I) and angularVelocity (Ó):
	/// u = IÓ
	/// delta(Ó) = I^-1 * u
	/// Where the inertia tensor is in world-coordinates.
	/// Also, where p = point of application, and g is the impulse.
	/// u = p x g
	Vector3f impulsiveTorque = centerToPosition.CrossProduct(impulse);
	Vector3f deltaAngularVelocity = inertiaTensorInverted.product(impulsiveTorque);
	
}


/// Updates physical radius, Bounding box size, etc. Should be called from the physics thread only!
void PhysicsProperty::UpdateProperties(Entity * entity){
    physicalRadius = entity->model->radius * entity->scaleVector.MaxPart();
    aabb.Recalculate(entity);
	obb.Recalculate(entity);
}

/// See state enumerations above. TODO: Consider making the state-variable below private.
void PhysicsProperty::SetPhysicsState(int state){
	switch(state){
		case PhysicsState::IN_REST:

			break;
		case PhysicsState::COLLIDING:
			state &= ~PhysicsState::IN_REST;
			/// If static object, don't remove the resting contacts with the other entities.
			if (type == PhysicsType::STATIC)
				return;
			/// check if we've got any resting contacts that need activation/removal.
			for (int i = 0; i < contacts.Size(); ++i){
				Contact * ct = contacts[i];
				if (ct->IsPartOf(owner)){
					switch(ct->type){
						case RESTING_CONTACT:
							/// Will remove the IN_REST flags from both entities and remove the contact from the entities..?
							ct->Disconnect();
							break;
						default:
							assert(false && "Invalid contact type?");
					}
				}
			}
			break;
		default:
			assert(false && "Invalid state?");
	}
}

/// Applies damping to both linear and angular momentum.
void PhysicsProperty::ApplyDamping(float ratio){
	linearMomentum *= ratio;
	angularMomentum *= ratio;
}

/// Calculates Linear and Angular velocity based on said momentum and other necessary variables.
void PhysicsProperty::CalculateVelocity(){
//	assert(false);
	if (type == PhysicsType::STATIC)
		return;

	// Linear
    velocity = linearMomentum * inverseMass;

	// Angular - assumes the proper matrices have already been set up.
	assert(inertiaTensorCalculated);
	Matrix3f rotationMatrix = owner->rotationMatrix.GetMatrix3f();
	Matrix3f rotationMatrixTransposed = rotationMatrix.TransposedCopy();
	/// I(body), Technically a 3x3 matrix, specifies how mass is ditributed in the body.
    /// In runtime, I(t) is calculated by R(t)I(body)R(t)^t   (last R(t) is transposed)
    /// The initial version is I(t), whislt the other two are I(body) and I(body)^-1
	inertiaTensor = rotationMatrix * inertiaTensorBody * rotationMatrixTransposed;
	inertiaTensorInverted = rotationMatrix * inertiaTensorBodyInverted * rotationMatrixTransposed;
    angularVelocity = inertiaTensorInverted * angularMomentum;
}

/// Saves away current linear/angular velocity to the their 'previous' variable counterparts.
void PhysicsProperty::StorePreviousVelocities(){
	if (previousVelocity != velocity)
		previousVelocity = velocity;
	if (previousAngularVelocity != angularVelocity)
		previousAngularVelocity = angularVelocity;
}