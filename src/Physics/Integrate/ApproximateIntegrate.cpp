/// Emil Hedemalm
/// 2014-05-18
/// Separating the integrate process to a separate file for clarity.

#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"

void PhysicsManager::ApproximateIntegrate(Entity * entity, float timeSinceLastUpdate)
{
	PhysicsProperty * physics = entity->physics;
   	// Calculate velocity using the moment?
	physics->CalculateVelocity();
	float currentVelocity = physics->velocity.Length();
    float velocitySquared = currentVelocity * currentVelocity;
    if (currentVelocity > 0.0001f)
	{
		entity->position += physics->velocity * timeSinceLastUpdate;
        physics->state |= PhysicsState::COLLIDING;
        physics->state &= ~PhysicsState::IN_REST;

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
        if (currentVelocity > 10.f)
		{
            float dragCoefficient = 0.001f;
            float crossSectionalArea = physics->physicalRadius;
            float dragForce = 0.5f * airDensity * velocitySquared * dragCoefficient * crossSectionalArea;
            float dragEnergy = dragForce * timeSinceLastUpdate;
            float velocityDecrease = sqrt(dragEnergy / MASS * 2);

            // And decrement the velocity.
			float preVel = physics->velocity.Length();
			Vector3f decrease = - physics->velocity.NormalizedCopy() * velocityDecrease;
            physics->velocity += decrease;
			float postVel = physics->velocity.Length();
		    // This looks like it should always be true,
            // but it's false if x is a NaN.
            if (!(physics->velocity.x == physics->velocity.x)){
      //          std::cout<<"\nwosh-";
            }
        }
    }
#ifdef _DEBUG
    // Make object bounce up again if at -100.0f made for testing physics only
/*		if (dynamicEntity[i]->position.y < -100.0f && dynamicEntity[i]->physics->velocity.y < 0){
        dynamicEntity[i]->position.y = -100.0f;
        dynamicEntity[i]->physics->velocity.y = dynamicEntity[i]->physics->velocity.y * -0.4;
    }
    */
#endif

    // Add gravitation and acceleration to velocities
    if (!(physics->state & PhysicsState::IN_REST))
	{
//                dynamicEntity->physics->velocity += this->gravitation * timeSinceLastUpdate;
		physics->linearMomentum += this->gravitation * physics->mass * timeSinceLastUpdate;
//                std::cout<<"Applying gravitation to velocity.";
    }
        

    // Always apply acceleration unless otherwise noted.
    if (physics->acceleration.MaxPart() > ZERO)
	{
		Vector3f velIncrease = entity->rotationMatrix.product(physics->acceleration);
		velIncrease *= timeSinceLastUpdate;
        Vector3f momentumIncrease = velIncrease * physics->mass;
		physics->linearMomentum += momentumIncrease;
//                std::cout<<"Accelerating  velocityIncrease: "<<velocityIncrease<<" newVel: "<<dynamicEntity->physics->velocity;
        
        physics->state |= PhysicsState::COLLIDING;
        physics->state &= ~PhysicsState::IN_REST;
    }
	
	/// Apply angular velocities
	float angularVelocitySquared = physics->angularVelocity.LengthSquared();
	float angularVelocity = sqrt(angularVelocitySquared);
	if (angularVelocitySquared > 0.000001f)
	{
        Vector3f lookAtPreRotate = entity->rotationMatrix * Vector4d(0,0,-1,1);

		if (physics->useQuaternions)
		{
			///Calculate with quaternions.
			PhysicsProperty * physics = entity->physics;
			Quaternion & orientation = physics->orientation;
			if (physics->angularVelocity.MaxPart() > 0){

	//              std::cout<<"\nQuaternion, pre: "<<physics->orientation;

	//            std::cout<<"\nTime since last update: "<<timeSinceLastUpdate;
				physics->orientation.ApplyAngularVelocity(physics->angularVelocity, timeSinceLastUpdate);
				physics->orientation.Normalize();
	  //          std::cout<<" post: "<<physics->orientation;
	   //         std::cout<<"\nAngular Velocity: "<<physics->angularVelocity;
			}
		}
		// Euclidean.
		else {
			/// Default gimbal-locking rotations
			entity->Rotate(physics->angularVelocity * timeSinceLastUpdate);
		}
		// Recalculate the matrix?
		entity->RecalculateMatrix();

		/// Adjust velocity in the movement direction depending on our new rotation!
		Vector3f lookAtPostRotate = entity->rotationMatrix * Vector4d(0,0,-1,1);
		float velDotLookatPreRotate = lookAtPreRotate.DotProduct(physics->velocity);
		float velPreRotate = physics->velocity.Length();
		Vector3f oldVelocity = physics->velocity;
		Vector3f velocityMinusLocalZVelocity = oldVelocity - lookAtPreRotate * velDotLookatPreRotate;
		/// Change it.
		physics->velocity = velocityMinusLocalZVelocity +
			velDotLookatPreRotate * lookAtPostRotate * physics->velocityRetainedWhileRotating +
			velDotLookatPreRotate * lookAtPreRotate * (1.0f - physics->velocityRetainedWhileRotating);
		
		/// And apply some damping or it will increase.
		float dampingDueToRotation = pow(0.99754f, angularVelocity);
		// Close, 0.99854f
		physics->velocity *= dampingDueToRotation;
		physics->velocity *= pow(linearDamping, timeSinceLastUpdate);
	//	if (dampingDueToRotation < 0.9985f)
		//	std::cout<<"\nDamping due to rotation: "<<dampingDueToRotation;

		float velPostRotate = physics->velocity.Length();
	//	assert(velPostRotate <= velPreRotate);

		//              std::cout<<"\nUpdating velocity using rotations..? "<<oldVelocity<<" newVel: "<<dynamicEntity->physics->velocity;
        
        // Decrease the velocity as if we've got some air in the way too?
        physics->angularVelocity *= pow(angularDamping, timeSinceLastUpdate);
        physics->state |= PhysicsState::COLLIDING;
        physics->state &= ~PhysicsState::IN_REST;
    }
	// Apply angular acceleration
    if (physics->angularAcceleration.MaxPart() > ZERO){
        // Screw the quaternions and stuff for now... do it all in local space
        Vector3f angularVelocityIncrease = physics->angularAcceleration * timeSinceLastUpdate;
        physics->angularVelocity += angularVelocityIncrease;
        physics->state |= PhysicsState::COLLIDING;
        physics->state &= ~PhysicsState::IN_REST;
    }

	// Update velocity.
	if (physics->inverseMass == 0)
		physics->inverseMass = 1 / physics->mass;
	physics->velocity = physics->linearMomentum * physics->inverseMass;
	entity->position += physics->velocity * timeSinceLastUpdate;

	// Move position


    // Reposition them in the entityCollissionOctree as needed...!
    // This could maybe be done less times per loop, but that would apply to the whole loop below if so...!

}