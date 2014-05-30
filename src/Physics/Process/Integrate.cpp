/// Emil Hedemalm
/// 2013-10-23
/// The main integrator-handling function that increases accelerations, momentums, 
/// re-calculates velocities, moves entities and re-places them in the collission detection structures.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Springs/Spring.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"

void PhysicsManager::Integrate(float timeSinceLastUpdate)
{
    assert(timeSinceLastUpdate > 0);
	// Process dynamic entities
	for (int i = 0; i < dynamicEntities.Size(); ++i){
		Entity * dynamicEntity = dynamicEntities[i];
		PhysicsProperty * physics = dynamicEntity->physics;

		/// If it uses estimation, process that now.
		if (dynamicEntity->physics->estimationEnabled){	
			dynamicEntity->physics->estimator->Process();
		}


		/// If simulation disabled, skip it.
		if (!dynamicEntity->physics->simulationEnabled)
			continue;

        bool own = false;
        bool coursePhysics = !own;
        bool customPhysics = own;
        /// Physics as calculated using simplified physics of only velocities and no acceleration forces whatsoever.
        if (integrator == Integrator::SIMPLIFIED_PHYSICS){
          //  std::cout<<"\nSIMPLIFIED_PHYSICS";
            /// Grab pointers
            PhysicsProperty * physics = dynamicEntity->physics;
            if (physics->velocity.MaxPart()){
            //    std::cout<<"\nVelocity: "<<physics->velocity;
            }
            dynamicEntity->position += physics->velocity * timeSinceLastUpdate;
            dynamicEntity->RecalculateMatrix();
        }
        /// Physics as calculated with strict Rigid body physics
        else if (integrator == Integrator::LAB_PHYSICS){
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
				physics->totalForce += dynamicEntity->rotationMatrix.product(physics->acceleration) * physics->mass;
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
			if (physics->state & PhysicsState::IN_REST){
				if (physics->totalForce.MaxPart() < 1.0f && 
					physics->totalTorque.MaxPart() < 1.0f && 
					physics->velocity.MaxPart() < 0.05f &&
					physics->angularVelocity.MaxPart() < 0.2f){
					inRest = true;
				}
				else {
					// Disable the rest-thingy, maybe.
					physics->state &= ~PhysicsState::IN_REST;
				}
			}

            /// Add gravitation!
			physics->totalForce += gravitation * physics->mass;

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
            Matrix3f wtStar(0, a.z, -a.y, -a.z, 0, a.x, a.y, -a.x, 0);
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
				Quaternion rotate(physics->angularVelocity.x, physics->angularVelocity.y, physics->angularVelocity.z);
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
					physics->orientation.Normalize();
				}
			}
			// Euclidean rotations
			else 
			{
				physics->angularVelocity *= pow(0.95f, timeSinceLastUpdate);
				dynamicEntity->rotation += physics->angularVelocity * timeSinceLastUpdate;
				dynamicEntity->rotation.x = dynamicEntity->rotation.z = 0;
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
				assert(physics->velocity.x == physics->velocity.x);
			}
        }
		//////////////////////////////////////////////////////////////////////////////////////////////////
        /// Physics as I designed it earlier.
		//////////////////////////////////////////////////////////////////////////////////////////////////
		else if (integrator == Integrator::APPROXIMATE)
		{
			ApproximateIntegrate(dynamicEntity, timeSinceLastUpdate);
        } /// End of custom physics step calculation

		// Recalculate the matrix!
		dynamicEntity->RecalculateMatrix();


        Vector3f vel = dynamicEntity->physics->velocity;
        /// Update octree or AABB position as needed.
        if (dynamicEntity->physics->collissionsEnabled){
            if (checkType == OCTREE)
                this->entityCollissionOctree->RepositionEntity(dynamicEntity);
            else if (checkType == AABB_SWEEP){
                /// Recalculate AABB
                dynamicEntity->physics->aabb.Recalculate(dynamicEntity);
                dynamicEntity->physics->obb.Recalculate(dynamicEntity);
            }
        }
    //    std::cout<<"\nPost-positioning Velocity: "<<dynamicEntity->physics->velocity;
        /// Ensure that the movement didn't adjust the velocity...
        assert(vel.x == dynamicEntity->physics->velocity.x);
	}
}