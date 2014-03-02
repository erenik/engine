/// Emil Hedemalm
/// 2013-10-23
/// The main integrator-handling function that increases accelerations, momentums, 
/// re-calculates velocities, moves entities and re-places them in the collission detection structures.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"
#include "Physics/Springs/Spring.h"
#include "Physics/Calc/EntityPhysicsEstimator.h"

void PhysicsManager::Integrate(float timeSinceLastUpdate){
    assert(timeSinceLastUpdate > 0);
	// Process dynamic entities
	for (int i = 0; i < dynamicEntities.Size(); ++i){
		Entity * dynamicEntity = dynamicEntities[i];
		
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
            dynamicEntity->positionVector += physics->velocity * timeSinceLastUpdate;
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
            physics->totalTorque += physics->angularAcceleration;
            if (physics->totalTorque.MaxPart() > ZERO)
                ;//std::cout<<"\nTotal torque: "<<physics->totalTorque;

			/// Check if the total force is above a pre-defined value, and skip applying it if the object is in rest?
			bool inRest = false;
			if (physics->state & PhysicsState::IN_REST){
				if (physics->totalForce.MaxPart() < 1.0f && 
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

                /// Calculate it's mass too while we're at it.
                Vector3f scale = physics->aabb.scale;
                /// TODO: Move mass settings elsewhere! And have it depend on a density parameter so it can scale well?
                /// Density in kg/m³ or g/dm³
                physics->mass = scale.x * scale.y * scale.z * defaultDensity;
                assert(physics->mass > 0);
                physics->inverseMass = 1 / physics->mass;
                std::cout<<"\nMass: "<<physics->mass<<" and inverseMass: "<<physics->inverseMass;
                /// Calculate intertia tensor!
                /// Ref: http://en.wikipedia.org/wiki/List_of_moment_of_inertia_tensors
                float m = 1.0f / 12.0f * physics->mass;
                float h2 = pow(physics->aabb.scale.y, 2);
                float w2 = pow(physics->aabb.scale.x, 2);
                float d2 = pow(physics->aabb.scale.z, 2);

            #define PRINT(m) std::cout<<"\n m: "<<m;
                std::cout<<"\nm: "<<m;
                std::cout<<"\nh2: "<<h2;
                std::cout<<"\nw2: "<<w2;
                std::cout<<"\nd2: "<<d2;
                float v = m*(h2 + d2);
                std::cout<<"\nv: "<<v;

				// Dude... http://en.wikipedia.org/wiki/List_of_moment_of_inertia_tensors
				// uses 1/12 applied to the angles. Why not done it here? Stupid emil! o.o
                physics->inertiaTensorBody = Matrix3f(m*(h2 + d2),0,0,0,m*(w2+d2),0,0,0,m*(w2+h2));
				/// Approximation: http://en.wikipedia.org/wiki/List_of_moments_of_inertia
				// Use the largest value..?
				float longestSide = h2 + d2 + w2;
				physics->momentOfInertia = longestSide * longestSide * physics->mass * 0.165f;

             //   physics->inertiaTensorBody = Matrix3f();
                const float * elements = physics->inertiaTensorBody.getPointer();
                std::cout<<"\nInertia tensor for "<<dynamicEntity->name<<": ";
                for (int i = 0; i < 9; ++i)
                    std::cout<<" "<<elements[i];
                physics->inertiaTensorBodyInverted = physics->inertiaTensorBody.InvertedCopy();
                elements = physics->inertiaTensorBodyInverted.getPointer();
                std::cout<<"\nInverted inertia tensor for "<<dynamicEntity->name<<": ";
                for (int i = 0; i < 9; ++i)
                    std::cout<<" "<<elements[i];

                /// Woo!
                physics->inertiaTensorCalculated = true;
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

            physics->inertiaTensorInverted = rotationMatrix * physics->inertiaTensorBodyInverted * rotationMatrixTransposed;
            physics->angularVelocity = physics->inertiaTensorInverted * physics->angularMomentum;
			
			/// Process movement if not in rest.
			if (!inRest){
				/// Increment all state variables?
				/// Evaluate locks for position before incrementing it.
				if (dynamicEntity->physics->locks & POSITION_LOCKED){

				}
				else
					dynamicEntity->positionVector += physics->velocity * timeSinceLastUpdate;
			  //  entity->rotationVector += physics->angularVelocity * timeSinceLastUpdate;

				Quaternion & orientation = physics->orientation;
				Quaternion rotate(physics->angularVelocity.x, physics->angularVelocity.y, physics->angularVelocity.z);

				Quaternion r2(physics->angularVelocity * timeSinceLastUpdate, 1);

			//    std::cout<<"\nOrientation: "<<orientation;

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

				/// Recalculate matrix after all movement is done.
				dynamicEntity->RecalculateMatrix();
			}
        }
		//////////////////////////////////////////////////////////////////////////////////////////////////
        /// Physics as I designed it earlier.
		//////////////////////////////////////////////////////////////////////////////////////////////////
		else if (integrator == Integrator::SPACE_RACE_CUSTOM_INTEGRATOR){
//            std::cout<<"\nSpace race integrator for dynamicEntity: "<<dynamicEntity;

            // Move position a length relative to current velocity, but only if we've got some speed at all.
  //          std::cout<<"\nCurrent velotity: "<<dynamicEntity->physics->velocity;
            float currentVelocity = dynamicEntity->physics->velocity.Length();
    //        if (currentVelocity)
    //    	   std::cout<<"\nCVel: "<<currentVelocity;
            float velocitySquared = currentVelocity * currentVelocity;
            if (currentVelocity > 0.0001f){
               // assert(false);
      //          std::cout<<"vel: "<<currentVelocity;
        //        std::cout<<"\nMoving from : "<<dynamicEntity->positionVector;
                dynamicEntity->translate(dynamicEntity->physics->velocity * (float)timeSinceLastUpdate);
          //      std::cout<<" to: "<<dynamicEntity->positionVector;
                
                dynamicEntity->physics->state |= PhysicsState::COLLIDING;
                dynamicEntity->physics->state &= ~PhysicsState::IN_REST;
                /// Only apply this if we're exceeding stuffs

                /** http://en.wikipedia.org/wiki/Drag_%28physics%29
                    Drag depends on the properties of the fluid and on the size, shape,
                    and speed of the object. One way to express this is by means of the drag equation:
                        Fd = 0.5 * p * v^2 * Cd * A
                    where
                        FD is the drag force,
                        p is the density of the fluid,[12]
                        v is the speed of the object relative to the fluid,
                        A is the cross-sectional area, and
                        Cd is the drag coefficient – a dimensionless number.

                    The drag coefficient depends on the shape of the object and on the Reynolds number:
                        Re
                */
#define AIR_DENSITY	1.225 // kg/m^3
#define VELOCITY dynamicEntity->physics->velocity
#define MASS 500
                if (currentVelocity > 10.f){
            //        std::cout<<"Applying drag ";
                    float dragCoefficient = 0.001f;
                    float crossSectionalArea = dynamicEntity->physics->physicalRadius;
                    float dragForce = 0.5f * airDensity * velocitySquared * dragCoefficient * crossSectionalArea;
                    float dragEnergy = dragForce * timeSinceLastUpdate;
                    // kinetic energy = mv^2 / 2
                    float velocityDecrease = sqrt(dragEnergy / MASS * 2);

                    // And decrement the velocity.
					float preVel = dynamicEntity->physics->velocity.Length();
					Vector3f decrease = - dynamicEntity->physics->velocity.NormalizedCopy() * velocityDecrease;
                    dynamicEntity->physics->velocity += decrease;
					float postVel = dynamicEntity->physics->velocity.Length();
				//	assert(postVel < preVel);
                    // This looks like it should always be true,
                    // but it's false if x is a NaN.
                    if (!(dynamicEntity->physics->velocity.x == dynamicEntity->physics->velocity.x)){
              //          std::cout<<"\nwosh-";
                    }
                }
            }
    #ifdef _DEBUG
            // Make object bounce up again if at -100.0f made for testing physics only
    /*		if (dynamicEntity[i]->positionVector.y < -100.0f && dynamicEntity[i]->physics->velocity.y < 0){
                dynamicEntity[i]->positionVector.y = -100.0f;
                dynamicEntity[i]->physics->velocity.y = dynamicEntity[i]->physics->velocity.y * -0.4;
            }
            */
    #endif

            // Add gravitation and acceleration to velocities
            if (!(dynamicEntity->physics->state & PhysicsState::IN_REST)){
                dynamicEntity->physics->velocity += this->gravitation * timeSinceLastUpdate;
//                std::cout<<"Applying gravitation to velocity.";
            }
                

            // Always apply acceleration unless otherwise noted.
            if (dynamicEntity->physics->acceleration.MaxPart() > ZERO){
                Vector3f velocityIncrease = dynamicEntity->transformationMatrix.product(Vector4f(dynamicEntity->physics->acceleration, 0.0f)) * timeSinceLastUpdate;
                dynamicEntity->physics->velocity += velocityIncrease;
//                std::cout<<"Accelerating  velocityIncrease: "<<velocityIncrease<<" newVel: "<<dynamicEntity->physics->velocity;
                
                dynamicEntity->physics->state |= PhysicsState::COLLIDING;
                dynamicEntity->physics->state &= ~PhysicsState::IN_REST;
            }
            /// Apply angular velocities
			float angularVelocitySquared = dynamicEntity->physics->angularVelocity.LengthSquared();
			float angularVelocity = sqrt(angularVelocitySquared);
			if (angularVelocitySquared > 0.000001f){

                Vector3f lookAtPreRotate = dynamicEntity->rotationMatrix * Vector4d(0,0,-1,1);

#ifdef USE_QUATERNIONS
                ///Calculate with quaternions.
                PhysicsProperty * physics = dynamicEntity->physics;
                Quaternion & orientation = physics->orientation;
                if (physics->angularVelocity.MaxPart() > 0){

      //              std::cout<<"\nQuaternion, pre: "<<physics->orientation;

        //            std::cout<<"\nTime since last update: "<<timeSinceLastUpdate;
                    physics->orientation.ApplyAngularVelocity(dynamicEntity->physics->angularVelocity, timeSinceLastUpdate);
                    physics->orientation.Normalize();
          //          std::cout<<" post: "<<physics->orientation;
           //         std::cout<<"\nAngular Velocity: "<<physics->angularVelocity;
                }
                dynamicEntity->recalculateMatrix();
#else
                /// Default gimbal-locking rotations
                dynamicEntity->rotate(dynamicEntity->physics->angularVelocity * timeSinceLastUpdate);
#endif
				/// Adjust velocity in the movement direction depending on our new rotation!
				Vector3f lookAtPostRotate = dynamicEntity->rotationMatrix * Vector4d(0,0,-1,1);
				float velDotLookatPreRotate = lookAtPreRotate.DotProduct(dynamicEntity->physics->velocity);
				float velPreRotate = dynamicEntity->physics->velocity.Length();
				Vector3f oldVelocity = dynamicEntity->physics->velocity;
				Vector3f velocityMinusLocalZVelocity = oldVelocity - lookAtPreRotate * velDotLookatPreRotate;
				/// Change it.
				dynamicEntity->physics->velocity = velocityMinusLocalZVelocity +
					velDotLookatPreRotate * lookAtPostRotate * dynamicEntity->physics->velocityRetainedWhileRotating +
					velDotLookatPreRotate * lookAtPreRotate * (1.0f - dynamicEntity->physics->velocityRetainedWhileRotating);
				
				/// And apply some damping or it will increase.
				float dampingDueToRotation = pow(0.99854f, angularVelocity);
				dynamicEntity->physics->velocity *= dampingDueToRotation;
				if (dampingDueToRotation < 0.9985f)
					std::cout<<"\nDamping due to rotation: "<<dampingDueToRotation;

				float velPostRotate = dynamicEntity->physics->velocity.Length();
			//	assert(velPostRotate <= velPreRotate);

				//              std::cout<<"\nUpdating velocity using rotations..? "<<oldVelocity<<" newVel: "<<dynamicEntity->physics->velocity;
                
                // Decrease the velocity as if we've got some air in the way too?
                dynamicEntity->physics->angularVelocity *= pow(pow(0.05f, 1), timeSinceLastUpdate);
                dynamicEntity->physics->state |= PhysicsState::COLLIDING;
                dynamicEntity->physics->state &= ~PhysicsState::IN_REST;
            }
            // Apply angular acceleration
            if (dynamicEntity->physics->angularAcceleration.MaxPart() > ZERO){
                // Screw the quaternions and stuff for now... do it all in local space
                Vector3f angularVelocityIncrease = dynamicEntity->physics->angularAcceleration * timeSinceLastUpdate;
                dynamicEntity->physics->angularVelocity += angularVelocityIncrease;
                dynamicEntity->physics->state |= PhysicsState::COLLIDING;
                dynamicEntity->physics->state &= ~PhysicsState::IN_REST;
            }
            // Reposition them in the entityCollissionOctree as needed...!
            // This could maybe be done less times per loop, but that would apply to the whole loop below if so...!
        } /// End of custom physics step calculation

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