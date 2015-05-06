/// Emil Hedemalm
/// 2014-07-03
/// Separating the integration variatns into separate files for clarity.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Springs/Spring.h"

#include "PhysicsLib/Force.h"


void PhysicsManager::LabPhysicsIntegrate(Entity * dynamicEntity, float timeSinceLastUpdate)
{
// std::cout<<"\nLAB_PHYSICS";
    /// Grab pointers
    PhysicsProperty * physics = dynamicEntity->physics;

    /// Grab matrices and convert as needed...
    Matrix3f rotationMatrix = dynamicEntity->rotationMatrix.GetMatrix3f();
    Matrix3f rotationMatrixTransposed = rotationMatrix.TransposedCopy();

    /// Reset..
    physics->totalTorque = Vector3f();
    physics->totalForce = Vector3f();
    /// ..and compute total force
    for (int i = 0; i < physics->forces.Size(); ++i){
        Force * force = physics->forces[i];
        physics->totalForce += force->amount;
        /// Do specific things with the force maybe here...
        force->duration += timeSinceLastUpdate;
        if (force->duration > force->lifeTime){
            physics->forces.Remove(force);
            --i;
            std::cout<<"\nKilling force: "<<force->amount;
            delete force;
            force = NULL;
        }
    }
	// Consider acceleration as a force in the direction of the entity?
	if (physics->acceleration.MaxPart())
	{
		physics->totalForce += dynamicEntity->rotationMatrix.Product(physics->acceleration) * physics->mass;
	}
	for (int i = 0; i < physics->springs.Size(); ++i){
		Spring * spring = physics->springs[i];
		Vector3f springForce = spring->GetForce(dynamicEntity);
		physics->totalForce += springForce;
	}
    /// ..and compute total torque
    for (int i = 0; i < physics->torques.Size(); ++i){
        Force * force = physics->torques[i];
        physics->totalTorque += force->amount;
        /// Do specific things with the force maybe here...
        force->duration += timeSinceLastUpdate;
        if (force->duration > force->lifeTime){
            physics->torques.Remove(force);
            --i;
            delete force;
        }
    }
    /// Add the angular acceleration that has been used to test the physics recently.
	if (physics->angularAcceleration.MaxPart())
	{
		if (physics->useQuaternions)
			physics->totalTorque += physics->angularAcceleration * physics->mass;
		else 
			physics->angularVelocity += physics->angularAcceleration * timeSinceLastUpdate;
	}
    if (physics->totalTorque.MaxPart() > ZERO)
        ;//std::cout<<"\nTotal torque: "<<physics->totalTorque;

	/// Check if the total force is above a pre-defined value, and skip applying it if the object is in rest?
	bool inRest = false;
	if (physics->state & CollisionState::IN_REST){
		if (physics->totalForce.MaxPart() < 1.0f && 
			physics->totalTorque.MaxPart() < 1.0f && 
			physics->velocity.MaxPart() < 0.05f &&
			physics->angularVelocity.MaxPart() < 0.2f &&
			physics->constantAngularVelocity.MaxPart() < 0.01f){
			inRest = true;
		}
		else {
			// Disable the rest-thingy, maybe.
			physics->state &= ~CollisionState::IN_REST;
		}
	}

    /// Add gravitation!
	physics->totalForce += gravitation * physics->mass * physics->gravityMultiplier;

    /// Add the angular acceleration that has been used to test the physics recently.
    if (physics->totalForce.MaxPart() > ZERO)
        ;//std::cout<<"\nTotal force: "<<physics->totalForce;

    /// Increment positions and stuff?
    /// Rotational.. change .. in matrix-form.. ?
    /// R(t) (dotted) = Star(w(t)) * R(t)

    /// Given vector a,  matrix a*   =
    /// [  0   -az   ay  ]
    /// [  az   0   -ax  ]
    /// [ -ay   ax   0   ]
    Vector3f a = physics->angularVelocity;
    Matrix3f wtStar(0, a[2], -a[1], -a[2], 0, a[0], a[1], -a[0], 0);
    Matrix3f Rdot = wtStar * rotationMatrix;

	// Apply stuff if not in rest
	if (!inRest){
		physics->linearMomentum += physics->totalForce * timeSinceLastUpdate;
		physics->angularMomentum += physics->totalTorque * timeSinceLastUpdate;
	}

    /// Apply some damping
    physics->linearMomentum *= pow(0.95f, timeSinceLastUpdate);
	physics->angularMomentum *= pow(0.95f, timeSinceLastUpdate);

    /// Applty MOAR DMPAING
//     physics->linearMomentum *= pow(0.9f, dt);
//     physics->angularMomentum *= pow(0.8f, dt);

    /// Recalculate 'auxiliary variables'...
    physics->velocity = physics->linearMomentum * physics->inverseMass;
    dynamicEntity->rotationMatrix = physics->orientation.Matrix();

    if (!physics->inertiaTensorCalculated){

		physics->CalculateInertiaTensor();
    }

    const float * matData;
/*
    std::cout<<"\nRotation matrix: ";
    matData = rotationMatrix.getPointer();
    for (int i = 0; i < 9; ++i){
        std::cout<<" "<<matData[i];
    }

    std::cout<<"\nRotation matrix transposed: ";
    matData = rotationMatrixTransposed.getPointer();
    for (int i = 0; i < 9; ++i){
        std::cout<<" "<<matData[i];
    }
    Matrix3f woo = rotationMatrix * rotationMatrixTransposed;
    std::cout<<"\nR(t) * R(t)^T: ";
    matData = woo.getPointer();
    for (int i = 0; i < 9; ++i){
        std::cout<<" "<<matData[i];
    }
*/
	if (physics->useQuaternions)
	{
		physics->inertiaTensorInverted = rotationMatrix * physics->inertiaTensorBodyInverted * rotationMatrixTransposed;
		physics->angularVelocity = physics->inertiaTensorInverted * physics->angularMomentum;
				
		Quaternion & orientation = physics->orientation;
		Quaternion rotate(physics->angularVelocity[0], physics->angularVelocity[1], physics->angularVelocity[2]);
		Quaternion r2(physics->angularVelocity * timeSinceLastUpdate, 1);

		if (physics->angularVelocity.MaxPart() > 0){
			/// In using quaternions to represent rotations,
			/// if q1 and q2 indicate rotations, then q2 q1 represents the
			/// composite rotation of q1 followed by q2 .4
			//  orientation = rotate * orientation;

			/// Change in quaternion. qDot(t) = 0.5 * angVel * q
			/// Where angVel * q is shorthand for [0, w(t)] and q
			//Quaternion qDot = 0.5f * Quaternion(physics->angularVelocity, 0) * orientation;
			orientation = orientation * r2;
			//  physics->orientation.Multiply(rotate);
			//  physics->orientation.ApplyAngularVelocity(dynamicEntity->physics->angularVelocity, timeSinceLastUpdate);
			orientation.Normalize();
		}

		/// Apply the extra constant angular rotation
		// First convert the requested angular velocity into some kind of quaternion equivalent?

		// Let the normalized version denote the direction, and the length denote the rotation around the normalized direction vector.
		// http://en.wikipedia.org/wiki/Unit_quaternion
		// http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
		Vector3f dir = physics->constantAngularVelocity.NormalizedCopy();
		float rotationDist = physics->constantAngularVelocity.Length();

		Vector3f testRot(0,0,0);
		Quaternion rotationQuaternion(physics->constantAngularVelocity * timeSinceLastUpdate, 1);
//		Quaternion rotationQuaternion(physics->angularVelocity * timeSinceLastUpdate, 1);
		orientation = orientation * rotationQuaternion;
		orientation.Normalize();

		

	}
	// Euclidean rotations
	else 
	{
		physics->angularVelocity *= pow(0.95f, timeSinceLastUpdate);
		dynamicEntity->rotation += physics->angularVelocity * timeSinceLastUpdate;
		dynamicEntity->rotation[0] = dynamicEntity->rotation[2] = 0;
		dynamicEntity->RecalculateMatrix();
	}
	/// Process movement if not in rest.
	if (!inRest){
		/// Increment all state variables?
		/// Evaluate locks for position before incrementing it.
		if (dynamicEntity->physics->locks & POSITION_LOCKED){

		}
		else
			dynamicEntity->position += physics->velocity * timeSinceLastUpdate;
		//  entity->rotation += physics->angularVelocity * timeSinceLastUpdate;

		/// Recalculate matrix after all movement is done.
		dynamicEntity->RecalculateMatrix();
		/// Ensure that the movement didn't adjust the velocity...
		assert(physics->velocity[0] == physics->velocity[0]);
	}
}

