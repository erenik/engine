// Emil Hedemalm
// 2013-03-24

#include "PhysicsOctree.h"
#include "PhysicsProperty.h"
#include "CompactPhysics.h"
#include "Model.h"
#include "Contact/Contact.h"

#include "PhysicsLib/Shapes/AABB.h"
#include "PhysicsLib/Shapes/OBB.h"

PhysicsProperty::PhysicsProperty()
{
	Nullify();
};

PhysicsProperty::~PhysicsProperty() {
	/// Contacts will be deleted in the physics manager!
	if (shape)
	{
		delete shape;
		shape = NULL;
	}
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
	noCollisionResolutions = other.noCollisionResolutions;
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
	noCollisionResolutions = compactPhysics->noCollisionResolutions;
}

/// Set default values.
void PhysicsProperty::Nullify()
{
	aabb = 0;
	obb = 0;
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
	linearDamping = 0.99f;
	collissionsEnabled = true;
	noCollisionResolutions = false;
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
	// Try it! o.o
	useQuaternions = true;	
	gravityMultiplier = 1.f;

	/// For separating categories and filters.
	/// 0x0000001 by default.
	collisionCategory = 0x00000001;
	/// 0x00000001 by default. is binary AND-ed to the opposing entities' category to see if a collision should occur or not.
	collisionFilter = 0x00000001;

	paused = false;
}

/// Sets mass, re-calculates inverse mass.
void PhysicsProperty::SetMass(float mass)
{
	this->mass = mass;
	this->inverseMass = 1.f / mass;
}

/// Calculates inertia tensor matrix and its inversion.
void PhysicsProperty::CalculateInertiaTensor()
{
	/// Calculate it's mass too while we're at it.
	Vector3f scale = aabb->scale;
	/// TODO: Move mass settings elsewhere! And have it depend on a density parameter so it can scale well?
	/// Density in kg/m³ or g/dm³
	float defaultDensity = 500;
	mass = scale.x * scale.y * scale.z * defaultDensity;
	assert(mass > 0);
	if (mass <= 0)
		return;
	inverseMass = 1 / mass;
	std::cout<<"\nMass: "<<mass<<" and inverseMass: "<<inverseMass;
	/// Calculate intertia tensor!
	/// Ref: http://en.wikipedia.org/wiki/List_of_moment_of_inertia_tensors
	float m = 1.0f / 12.0f * mass;
	float h2 = pow(aabb->scale.y, 2);
	float w2 = pow(aabb->scale.x, 2);
	float d2 = pow(aabb->scale.z, 2);

	#define PRINT(m) std::cout<<"\n m: "<<m;
	std::cout<<"\nm: "<<m;
	std::cout<<"\nh2: "<<h2;
	std::cout<<"\nw2: "<<w2;
	std::cout<<"\nd2: "<<d2;
	float v = m*(h2 + d2);
	std::cout<<"\nv: "<<v;

	// Dude... http://en.wikipedia.org/wiki/List_of_moment_of_inertia_tensors
	// uses 1/12 applied to the angles. Why not done it here? Stupid emil! o.o
	inertiaTensorBody = Matrix3f(m*(h2 + d2),0,0,0,m*(w2+d2),0,0,0,m*(w2+h2));
	/// Approximation: http://en.wikipedia.org/wiki/List_of_moments_of_inertia
	// Use the largest value..?
	float longestSide = h2 + d2 + w2;
	momentOfInertia = longestSide * longestSide * mass * 0.165f;

	//   physics->inertiaTensorBody = Matrix3f();
	const float * elements = inertiaTensorBody.getPointer();
	std::cout<<"\nInertia tensor for "<<owner->name<<": ";
	for (int i = 0; i < 9; ++i)
		std::cout<<" "<<elements[i];
	inertiaTensorBodyInverted = inertiaTensorBody.InvertedCopy();
	elements = inertiaTensorBodyInverted.getPointer();
	std::cout<<"\nInverted inertia tensor for "<<owner->name<<": ";
	for (int i = 0; i < 9; ++i)
		std::cout<<" "<<elements[i];

	/// Woo!
	inertiaTensorCalculated = true;
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
	Vector3f centerToPosition = position - obb->position;
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
void PhysicsProperty::UpdateProperties(Entity * entity)
{
	if (entity->model)
	{
		physicalRadius = entity->model->radius * entity->scale.MaxPart();
		aabb->Recalculate(entity);
		obb->Recalculate(entity);
	}
	else 
		physicalRadius = 0;
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
void PhysicsProperty::CalculateVelocity()
{
//	assert(false);
	if (type == PhysicsType::STATIC)
		return;

	// Linear
    velocity = linearMomentum * inverseMass;

	if (useQuaternions){
		// Angular - assumes the proper matrices have already been set up.
		if (!inertiaTensorCalculated)
			CalculateInertiaTensor();
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
}

/// Saves away current linear/angular velocity to the their 'previous' variable counterparts.
void PhysicsProperty::StorePreviousVelocities(){
	if (previousVelocity != velocity)
		previousVelocity = velocity;
	if (previousAngularVelocity != angularVelocity)
		previousAngularVelocity = angularVelocity;
}