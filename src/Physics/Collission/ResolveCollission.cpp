// Emil Hedemalm
// 2013-03-19


#include "../PhysicsProperty.h"
#include "Collissions.h"
#include "Collission.h"
#include "../Entity/Entity.h"
;
#include "Physics/Messages/CollissionCallback.h"
#include "Message/MessageManager.h"
#include "Physics/Contact/Contact.h"

#ifndef min
#define min(a, b) ((a < b)? (a) : (b))
#endif

#include "Physics/PhysicsManager.h"

/// Ref: Morgan Kaufmann Game Physics Engine Development.
void CreateOrthonormalBasis(const Vector3f & x, Vector3f * y, Vector3f * z){
    assert(y && z);
    *z = x.CrossProduct(*y).NormalizedCopy();
    if (z->LengthSquared() == 0.0)
        return;
    (*y) = (*z).CrossProduct(x).NormalizedCopy();
};


/// Resolves the active collission between the two entities using the given data. Returns false if they are no longer colliding after resolution.
bool ResolveCollission(Collission &data){
	;
	Entity * one = data.one;
	Entity * two = data.two;
	assert(one);
	assert(two);

//	assert(data.collissionPoint.MaxPart() > 0.0f);
	if (data.collissionNormal.MaxPart() == 0)
		return false;
    assert(data.collissionNormal.MaxPart() > 0.0f); 

	data.cr.onePreResolution = one->transformationMatrix;
	data.cr.twoPreResolution = two->transformationMatrix;

	// If toggled, notify of the collission
	int collissionCallback = one->physics->collissionCallback || two->physics->collissionCallback;
	if (collissionCallback){
		// Assume (hope) one holds necessary data :3
		assert(one->physics->collissionCallback != DISABLED);
		bool shouldSendMessage = true;
		float impactVelocity = 0.0f;
		// Wush
		switch(one->physics->collissionCallback){
			case NO_REQUIREMENT:
				break;
			case IMPACT_VELOCITY: {
				impactVelocity = one->physics->velocity.DotProduct(data.collissionNormal) +
					two->physics->velocity.DotProduct(data.collissionNormal);
				// Each physics simulation is roughly 10ms long
				impactVelocity = AbsoluteValue(impactVelocity);
				// Check exact length
				if (impactVelocity < one->physics->collissionCallbackRequirementValue ||
					impactVelocity < two->physics->collissionCallbackRequirementValue)
					shouldSendMessage = false;
				break;
								  }
			default:
				assert(false && "Bad type when checking collission callback requirements");
				shouldSendMessage = false;
				break;
		}
		if (shouldSendMessage){
			CollissionCallback * msg = new CollissionCallback();
			// Save the entities if only they are needed first.
			msg->one = one;
			msg->two = two;
			msg->impactNormal = data.collissionNormal;
			msg->impactVelocity = impactVelocity;
			msg->collissionPoint = data.collissionPoint;
			/// Queue le message!
			MesMan.QueueMessage((Message*)msg);
		}
	}

	// Return straight away if no collission should be resolved, yo?
	if (one->physics->noCollissionResolutions || two->physics->noCollissionResolutions){
		return false;
	}

	/// Check what kind of physical objects we're comparing too (dynamic, static, kinematic, etc.)
	int dynamicEntities = 0;
	if (one->physics->type == PhysicsType::DYNAMIC)
		++dynamicEntities;
	if (two->physics->type == PhysicsType::DYNAMIC)
		++dynamicEntities;

	/// Pointers for dynamic/static respectively if the collissions are between these types.
	Entity * dynamicEntity = NULL;
	Entity * staticEntity = NULL;
	if (dynamicEntities == 1){
		if (one->physics->type == PhysicsType::DYNAMIC){
			dynamicEntity = one;
			staticEntity = two;
		}
		else {
			dynamicEntity = two;
			staticEntity = one;
		}
	}

	Vector3f collissionNormal = data.collissionNormal;

	/// Differentiate between implementation.
    if (Physics.collissionResolver == LAB_PHYSICS_IMPULSES){

		float kineticEnergyPreResolution = one->physics->KineticEnergy() + two->physics->KineticEnergy();

        /// Create matrix to contact-space
        /// ccN = Contact coordinate N
        Vector3f ccX = data.collissionNormal.NormalizedCopy();
        Vector3f ccY(0,1.0f,0), ccZ;
        if (AbsoluteValue(ccX.x) < AbsoluteValue(ccX.y))
            ccY = Vector3f(1.0,0,0);
        CreateOrthonormalBasis(ccX, &ccY, &ccZ);

        Vector3f & contactNormal = data.collissionNormal;

        /// Matrix * Point(local) = Point(world)
        Matrix3f basisMatrix(ccX.x, ccX.y, ccX.z, ccY.x, ccY.y, ccY.z, ccZ.x, ccZ.y, ccZ.z);
        Matrix3f inverseBasisMatrix = basisMatrix.TransposedCopy();


		/// New fresh start... lol.
		Vector3f relativeContactPosition = data.collissionPoint - one->position;
		Vector3f relativeContactPosition2 = data.collissionPoint - two->position;
		
		/// Add Calculate relative velocities for the contact point.
		Vector3f relVel1 = -one->physics->angularVelocity.CrossProduct(relativeContactPosition) + one->physics->velocity;
		Vector3f relVel2 = -two->physics->angularVelocity.CrossProduct(relativeContactPosition2) + two->physics->velocity;
		Vector3f relativeVelocity = relVel1 - relVel2;
        
		data.collissionPointVelocity[0] = relVel1;
		data.collissionPointVelocity[1] = relVel2;

		/// Move them away from each other.
		assert(data.results & DISTANCE_INTO);
		data.distanceIntoEachOther = AbsoluteValue(data.distanceIntoEachOther);
		if (dynamicEntity){
			float sign = dynamicEntity == one? -1.0f : 1.0f;
			dynamicEntity->position += data.collissionNormal * data.distanceIntoEachOther * 1.1f * sign;
		}
		else {
			one->position -= data.collissionNormal * data.distanceIntoEachOther * 0.55f;
			two->position += data.collissionNormal * data.distanceIntoEachOther * 0.55f;
		}
		
		/// Calculate requested change in contact velocity.
		float restitution = min(one->physics->restitution, two->physics->restitution);
		float requestedVelocityChange = - (1.0f + restitution) * relativeVelocity.Length();
		
		/// Calcualte required data to apply the requested velocity change.

		/// Velocity change per impulse unit for both entities.
		float contactAngularComponentChangePerImpulse = 0;
		Vector3f impulsiveTorquePerUnitImpulse = -relativeContactPosition.CrossProduct(contactNormal);
		Vector3f impulsiveTorquePerUnitImpulse2 = -relativeContactPosition2.CrossProduct(contactNormal);
		Vector3f rotationPerUnitImpulse = one->physics->inertiaTensorInverted.product(impulsiveTorquePerUnitImpulse);
		Vector3f rotationPerUnitImpulse2 = two->physics->inertiaTensorInverted.product(impulsiveTorquePerUnitImpulse2);
		Vector3f velocityPerUnitImpulse = rotationPerUnitImpulse.CrossProduct(relativeContactPosition);
		Vector3f velocityPerUnitImpulse2 = rotationPerUnitImpulse2.CrossProduct(relativeContactPosition2);
		
		contactAngularComponentChangePerImpulse += velocityPerUnitImpulse.Length() + velocityPerUnitImpulse2.Length();
		
		float linearVelocityChangePerUnitImpulse = one->physics->inverseMass + two->physics->inverseMass;
		float totalVelocityChangePerUnitImpulse = linearVelocityChangePerUnitImpulse + contactAngularComponentChangePerImpulse;

		// Calculate J (impulse)
		float impulseMagnitude = requestedVelocityChange / totalVelocityChangePerUnitImpulse;
		Vector3f j;
		j = data.collissionNormal * impulseMagnitude;

		/// Store previous moment to render delta.
		Vector3f previousLinearMomentum = one->physics->linearMomentum,
			previousLinearMomentum2 = two->physics->linearMomentum,
			previousAngularMomentum = one->physics->angularMomentum,
			previousAngularMomentum2 = two->physics->angularMomentum;

		// Apply impulse to both using the standardized function, yo.
		one->physics->ApplyImpulse(j, data.collissionPoint);
		two->physics->ApplyImpulse(-j, data.collissionPoint);

		// Re-calcualte velocities and stuff.
		/// Store previous values for debug-rendering.
		one->physics->StorePreviousVelocities();
		two->physics->StorePreviousVelocities();
		/// Recalculate the entities velocities for proper debug-rendering/visualization.
		one->physics->CalculateVelocity();
		two->physics->CalculateVelocity();

		/// Save away resolution data.
		data.cr.deltaLinearMomentum[0] = one->physics->linearMomentum - previousLinearMomentum; 
		data.cr.deltaLinearMomentum[1] = two->physics->linearMomentum - previousLinearMomentum2;
		data.cr.deltaAngularMomentum[0] = one->physics->angularMomentum - previousAngularMomentum; 
		data.cr.deltaAngularMomentum[1] = two->physics->angularMomentum - previousAngularMomentum2;

		/// Extract collissionpoint
		Vector3f collissionPoint = data.collissionPoint;
		
		/// Recalculate matrices if not already done.
		one->RecalculateMatrix();
		one->physics->obb.Recalculate(one);
		two->RecalculateMatrix();
		two->physics->obb.Recalculate(two);
		
		bool restrictKineticEnergy = false;
		// Ensure kinematic energy hasn't flown through the roof..
		float kineticEnergyPostResolution = one->physics->KineticEnergy() + two->physics->KineticEnergy();
		if (kineticEnergyPostResolution > kineticEnergyPreResolution * 10.0f && restrictKineticEnergy){
			float ratio = kineticEnergyPostResolution / kineticEnergyPreResolution;
			std::cout<<"\nWarning: Kinetic energy increased by a ratio of "<<ratio<<".";
		//	assert(false && "Kinetic energy fucked up.");
			one->physics->linearMomentum /= ratio;
			one->physics->angularMomentum /= ratio;
			two->physics->linearMomentum /= ratio;
			two->physics->angularMomentum /= ratio;
			std::cout<<"\nINFO: Energy hopefully somewhat stabilized now.";
		}

		/// Apply damping to both linear and angular momentums since there are so many points in the collission!
		float oneRatio = 1.0f - (data.pointsOne.Size() * 0.02f);
		float twoRatio = 1.0f - (data.pointsTwo.Size() * 0.02f);
		one->physics->angularMomentum *= oneRatio;
		two->physics->angularMomentum *= twoRatio;
		one->physics->linearMomentum *= oneRatio;
		two->physics->linearMomentum *= twoRatio;


		/// Set them both to be in rest if one already was in rest, the momentums are low enough and the normal is pointing up, etc.
		bool inRest = false;
		if (data.pointsOne.Size() >= 3 && data.pointsTwo.Size() >= 3) {
			/// Apply damping to both linear and angular momentums since there are so many points in the collission!
			one->physics->angularMomentum *= 0.95f;
			two->physics->angularMomentum *= 0.95f;

		//	std::cout<<"\nSeveral collission points. Maybe time to rest depending on momentums.";
			bool linearMomentumLowEnough = false;
			int requirements = 0;
			if (one->physics->linearMomentum.MaxPart() < one->physics->mass * 0.1f &&
				one->physics->angularMomentum.MaxPart() < one->physics->mass * 10.0f){
				linearMomentumLowEnough = true;
				++requirements;
		//		std::cout<<"\nLinear meomentum below 10% of mass.";
			}
			if (two->physics->linearMomentum.MaxPart() < two->physics->mass * 0.1f &&
				two->physics->angularMomentum.MaxPart() < two->physics->mass * 10.0f){
				++requirements;
			}
			if ((one->physics->state & PhysicsState::IN_REST || one->physics->type == PhysicsType::STATIC)|| 
				(two->physics->state & PhysicsState::IN_REST || two->physics->type == PhysicsType::STATIC))
				++requirements;
			/// Set the entity to be in rest if the momentum and sum of all external forces is below a certain value.
			if (requirements >= 3 &&
				AbsoluteValue(collissionNormal.y) > 0.9f
				)
			{
				RestingContact * ct = new RestingContact(one, two);
				one->physics->contacts.Add(ct);
				two->physics->contacts.Add(ct);
				Physics.contacts.Add(ct);
				
				one->physics->state |= PhysicsState::IN_REST;
				two->physics->state |= PhysicsState::IN_REST;
				// Remove the colliding flag..?
				inRest = true;
			}
		}
		/// Flag both as not in rest after collission if not deemed otherwise (see above)
		if (!inRest){
			one->physics->SetPhysicsState(PhysicsState::COLLIDING);
			two->physics->SetPhysicsState(PhysicsState::COLLIDING);
		}

        /// Linear velocity change = impulse * (inverseMass 1 + inverseMass 2)

        /// Relative position of contact to origin of object: Q(relative) = q - p

	//	assert(contactVelocityF > 0);
        
/*
		// For two dynamic entities' resolution
		if (dynamicEntities == 2){

			/// Impulsive torque generated from a unit of impulse u = Q(relative) x d
			/// where d is the direction of the impulse (in our case the contact normal).
			Vector3f relativeContactPosition = data.collissionPoint - one->position;

			Vector3f impulsiveTorquePerUnitImpulse = relativeContactPosition.CrossProduct(contactNormal);
			Vector3f rotationPerUnitImpulse = one->physics->inertiaTensorInverted.product(impulsiveTorquePerUnitImpulse);
			Vector3f velocityPerUnitImpulse = rotationPerUnitImpulse.CrossProduct(relativeContactPosition);
			/// And convert to contact coordinates.
			Vector3f velocityPerUnitImpulseContact = inverseBasisMatrix.product(velocityPerUnitImpulse);
			/// X being the direction of the contact normal.
			float angularComponent = velocityPerUnitImpulseContact.x;
			float angularComponent2 = velocityPerUnitImpulse.DotProduct(contactNormal);

			/// Add angular velocity
			Vector3f relativeContactPosition2 = data.collissionPoint - two->position;
			Vector3f relVel1 = -one->physics->angularVelocity.CrossProduct(relativeContactPosition) + one->physics->velocity;
			Vector3f relVel2 = -two->physics->angularVelocity.CrossProduct(relativeContactPosition2) + two->physics->velocity;
			Vector3f relativeVelocity = relVel1 - relVel2;
	        
			data.collissionPointVelocity[0] = relVel1;
			data.collissionPointVelocity[1] = relVel2;

			float vRel = data.collissionNormal.DotProduct(relativeVelocity);
			Vector3f contactVelocity = inverseBasisMatrix.product(relativeVelocity);
			float contactVelocityF = contactVelocity.x;
			contactVelocityF = contactVelocityF;

			/// Move them away from each other before evaluating if further actions should be taken?
			/// Normal should go from one to two.
			assert(data.results & DISTANCE_INTO);
			data.distanceIntoEachOther = AbsoluteValue(data.distanceIntoEachOther);
			one->position -= data.collissionNormal * data.distanceIntoEachOther * 0.55f;
			two->position += data.collissionNormal * data.distanceIntoEachOther * 0.55f;

			/// If zis contact velocity is below 0 (or above?) then they're already separating, so don't annoy them anymore, kay?
			if (contactVelocityF < 0){
				std::cout<<"\nNot colliding?";//return false;//return false;
				return false;
			}


			 std::cout<<"\nContact velocity: "<<contactVelocity<<" of which in normal: "<<contactVelocityF;
			float deltaVelocity = -contactVelocityF * (1.0f + one->physics->restitution * two->physics->restitution);
			float deltaVelocityToDesired = contactVelocityF;
			/// Change in velocity, g = impulse required, v = desired change, d = velocity change per unit impulse
			/// g = v / d
			Vector3f impulseContact(deltaVelocity,0,0);

			Vector3f impulse = basisMatrix * impulseContact * (one->physics->mass + two->physics->mass)*0.5f;

			/// Applying the impulse...:
			Vector3f velocityChange = impulse * one->physics->inverseMass;
			Vector3f impulsiveTorque = impulse.CrossProduct(relativeContactPosition);
			Vector3f rotationChange = one->physics->inertiaTensorInverted * impulsiveTorque;

			impulse *= -1.f;

			/// Applying the impulse...:
			Vector3f velocityChange2 = impulse * two->physics->inverseMass;
			Vector3f impulsiveTorque2 = impulse.CrossProduct(relativeContactPosition2);
			Vector3f rotationChange2 = two->physics->inertiaTensorInverted * impulsiveTorque2;

	//		std::cout<<"\nRotationChange 1 (yellow?): "<<rotationChange;
	//		std::cout<<"\nRotationChange 2 (teal?): "<<rotationChange2;

			// Ensure that the change in linear momentum are aligned relative to the collission normal, and in the rightj direction..
			float deltaVelDotNormal = velocityChange.DotProduct(data.collissionNormal);
			if (deltaVelDotNormal > 0)
				velocityChange *= -1.f;
			deltaVelDotNormal = velocityChange2.DotProduct(data.collissionNormal);
			if (deltaVelDotNormal < 0)
				velocityChange2 *= -1.f;

			Vector3f deltaLinearMomentum1 = velocityChange * one->physics->mass;
			Vector3f deltaLinearMomentum2 = velocityChange2 * two->physics->mass;

			Vector3f deltaAngularMomentum1 = rotationChange;
			Vector3f deltaAngularMomentum2 = rotationChange2;

			/// Separate them using old velocities before updating them.
			float time = 0.01f;
		//	one->position -= time * one->physics->velocity;
		//	two->position -= time * two->physics->velocity;
			
			/// Turn back rotations too, yes yes. No.
		//	one->physics->orientation = one->physics->orientation * Quaternion(-time * one->physics->angularVelocity, 1);
		//	two->physics->orientation = two->physics->orientation * Quaternion(-time * two->physics->angularVelocity, 1);

			/// Save away resolution data.
			data.cr.deltaLinearMomentum[0] = deltaLinearMomentum1; 
			data.cr.deltaLinearMomentum[1] = deltaLinearMomentum2;
			data.cr.deltaAngularMomentum[0] = deltaAngularMomentum1; 
			data.cr.deltaAngularMomentum[1] = deltaAngularMomentum2;

			one->physics->linearMomentum += deltaLinearMomentum1;
			two->physics->linearMomentum += deltaLinearMomentum2;
			one->physics->angularMomentum += deltaAngularMomentum1;
			two->physics->angularMomentum += deltaAngularMomentum2;

			/// Change in angular velocity for a unit of impulsive torque for a unit of impulse = inverse inertia tensor * u (impulsive torque)

			/// Linear velocity of a point due only to rotation q(dot) = Omega x Q(relative)


			bool inRest = false;
			/// Set them both to be in rest if one already was in rest, the momentums are low enough and the normal is pointing up, etc.
			if (data.pointsOne.Size() >= 3 && data.pointsTwo.Size() >= 3) {
				std::cout<<"\nSeveral collission points. Maybe time to rest depending on momentums.";
				bool linearMomentumLowEnough = false;
				int requirements = 0;
				if (one->physics->linearMomentum.MaxPart() < one->physics->mass * 0.1f &&
					one->physics->angularMomentum.MaxPart() < one->physics->mass * 10.0f){
					linearMomentumLowEnough = true;
					++requirements;
					std::cout<<"\nLinear meomentum below 10% of mass.";
				}
				if (two->physics->linearMomentum.MaxPart() < two->physics->mass * 0.1f &&
					two->physics->angularMomentum.MaxPart() < two->physics->mass * 10.0f){
					++requirements;
				}
				if (one->physics->state & PhysicsState::IN_REST || two->physics->state & PhysicsState::IN_REST)
					++requirements;
				/// Set the entity to be in rest if the momentum and sum of all external forces is below a certain value.
				if (requirements >= 3 &&
					AbsoluteValue(collissionNormal.y) > 0.9f
					)
				{
					RestingContact * ct = new RestingContact(one, two);
					one->physics->contacts.Add(ct);
					two->physics->contacts.Add(ct);
					Physics.contacts.Add(ct);
					
					one->physics->state |= PhysicsState::IN_REST;
					two->physics->state |= PhysicsState::IN_REST;
					inRest = true;
				}
			}
			/// Flag both as not in rest after collission if not deemed otherwise (see above)
			if (!inRest){
				one->physics->SetPhysicsState(PhysicsState::COLLIDING);
				two->physics->SetPhysicsState(PhysicsState::COLLIDING);
			}
			/// Store previous values for debug-rendering.
			one->physics->StorePreviousVelocities();
			two->physics->StorePreviousVelocities();
			/// Recalculate the entities velocities for proper debug-rendering/visualization.
			one->physics->CalculateVelocity();
			two->physics->CalculateVelocity();

			/// Extract collissionpoint
			Vector3f collissionPoint = data.collissionPoint;

			one->RecalculateMatrix();
			one->physics->obb.Recalculate(one);
			two->RecalculateMatrix();
			two->physics->obb.Recalculate(two);
		}		
		/// For Dynamic-Static entity collission resolution
		else {


			/// Impulsive torque generated from a unit of impulse u = Q(relative) x d
			/// where d is the direction of the impulse (in our case the contact normal).
			Vector3f relativeContactPosition = data.collissionPoint - dynamicEntity->position;
		//	std::cout<<"\nRelative contact position: "<<relativeContactPosition;

			Vector3f impulsiveTorquePerUnitImpulse = relativeContactPosition.CrossProduct(contactNormal);
			Vector3f rotationPerUnitImpulse = dynamicEntity->physics->inertiaTensorInverted.product(impulsiveTorquePerUnitImpulse);
			Vector3f velocityPerUnitImpulse = rotationPerUnitImpulse.CrossProduct(relativeContactPosition);
			/// And convert to contact coordinates.
			Vector3f velocityPerUnitImpulseContact = inverseBasisMatrix.product(velocityPerUnitImpulse);
			/// X being the direction of the contact normal.
			float angularComponent = velocityPerUnitImpulseContact.x;
			
			/// Add angular velocity
			Vector3f relVel = -dynamicEntity->physics->angularVelocity.CrossProduct(relativeContactPosition) + dynamicEntity->physics->velocity;
			Vector3f relativeVelocity = relVel;
	        
			data.collissionPointVelocity[0] = relVel;
			data.collissionPointVelocity[1] = Vector3f();

			float vRel = data.collissionNormal.DotProduct(relativeVelocity);
			Vector3f contactVelocity = inverseBasisMatrix.product(relativeVelocity);
			float contactVelocityF = contactVelocity.x;
			contactVelocityF = contactVelocityF;

			// Woo?
			if (contactVelocityF > 0){
				std::cout<<"\nNot colliding?";//return false;//return false;
				return false;
			}
			/// Handle collissions of low velocities as if they are descending into rest or something?
			else if (contactVelocityF > -0.01f){
				//
				return false;
			}

			/// Where: 0 < restitution < 1, relativeVelocity is before collission
			float restitution = 0.5f;
			/// See Wiki: http://en.wikipedia.org/wiki/Collision_response
			/// jr 

			float impulseMagnitude = relativeVelocity.DotProduct(collissionNormal);	
			impulseMagnitude *= -(1 + restitution);
			
			/// Set the static-entity's matrix to be a 0-matrix.
			staticEntity->physics->inertiaTensorInverted = Matrix3f(0,0,0,0,0,0,0,0,0);

			float inverseMass = dynamicEntity->physics->inverseMass, inverseMass2 = 0;
			Vector3f stuff = (staticEntity->physics->inertiaTensorInverted * (relativeContactPosition.CrossProduct(collissionNormal))).CrossProduct(relativeContactPosition);
			float dividend = inverseMass + inverseMass2 + 
				((dynamicEntity->physics->inertiaTensorInverted * (relativeContactPosition.CrossProduct(collissionNormal))).CrossProduct(relativeContactPosition)
				+ stuff
				).DotProduct(collissionNormal);
			impulseMagnitude /= dividend;
			/// Make it correct, yo.
		//	impulseMagnitude *= -1.0f;
			/// Impulse vector Jr
			Vector3f impulseVector = impulseMagnitude * collissionNormal;
			
			/// Compute new velocities
	//		std::cout<<"\nImpulse magnitude: "<<impulseMagnitude;
	//		std::cout<<"\nImpulseVector: "<<impulseVector;
			
			Vector3f velocityChange = impulseVector * dynamicEntity->physics->inverseMass; // Double up the impulse to be applied to the dynamic entity, ye?
			Vector3f impulsiveTorque = impulseMagnitude * (dynamicEntity->physics->inertiaTensorInverted * (relativeContactPosition.CrossProduct(collissionNormal)));


			float deltaVelocity = -contactVelocityF * (1.0f + restitution);
			float deltaVelocityToDesired = contactVelocityF;
			/// Change in velocity, g = impulse required, v = desired change, d = velocity change per unit impulse
			/// g = v / d
			Vector3f impulseContact(deltaVelocity,0,0);
			Vector3f impulse = basisMatrix * impulseContact * dynamicEntity->physics->mass;
			Vector3f impulsiveTorqueTest = impulse.CrossProduct(relativeContactPosition);
			Vector3f rotationChangeTest = dynamicEntity->physics->inertiaTensorInverted * impulsiveTorque;
			Vector3f deltaAngularMomentum1Test = rotationChangeTest;


			/// Multiply torque by -1 to make it right because of coordinate-system..ish
			impulsiveTorque *= -1.0f;
			Vector3f deltaLinearMomentum = velocityChange * dynamicEntity->physics->mass;
			
			Vector3f deltaAngularMomentum = impulsiveTorque;
			
			/// Separate them using old velocities before updating them.
			float time = 0.011f;
		//	dynamicEntity->position -= time * dynamicEntity->physics->velocity;
			// Separate them along the contact normal too?
			/// Normal should go from one to two.
			assert(data.results & DISTANCE_INTO);
			data.distanceIntoEachOther = AbsoluteValue(data.distanceIntoEachOther);
			float sign = dynamicEntity == one? -1.0f : 1.0f;
			dynamicEntity->position += sign * data.collissionNormal * data.distanceIntoEachOther * 1.5f;
			
			/// Turn back rotations too, yes yes. No.
		//	dynamicEntity->physics->orientation = dynamicEntity->physics->orientation * Quaternion(-time * dynamicEntity->physics->angularVelocity, 1);
			
			int index = dynamicEntity == one? 0 : 1;
			int emptyIndex = index? 0 : 1;

			/// Save away resolution data.
			data.cr.deltaLinearMomentum[index] = deltaLinearMomentum; 
			data.cr.deltaLinearMomentum[emptyIndex] = Vector3f();
			data.cr.deltaAngularMomentum[index] = deltaAngularMomentum; 
			data.cr.deltaAngularMomentum[emptyIndex] = Vector3f();

			dynamicEntity->physics->linearMomentum += deltaLinearMomentum;
			dynamicEntity->physics->angularMomentum += deltaAngularMomentum;

			if (data.pointsOne.Size() >= 3 && data.pointsTwo.Size() >= 3) {
				std::cout<<"\nSeveral collission points. Maybe time to rest depending on momentums.";
				bool linearMomentumLowEnough = false;
				int requirements = 0;
				if (dynamicEntity->physics->linearMomentum.MaxPart() < dynamicEntity->physics->mass * 0.1f){
					linearMomentumLowEnough = true;
					++requirements;
					std::cout<<"\nLinear meomentum below 10% of mass.";
				}
				if (dynamicEntity->physics->angularMomentum.MaxPart() < dynamicEntity->physics->mass * 10.0f){
					++requirements;
				}
				/// Set the entity to be in rest if the momentum and sum of all external forces is below a certain value.
				if (requirements >= 2 &&
					collissionNormal.y > 0.9f
					)
				{
					dynamicEntity->physics->state |= PhysicsState::IN_REST;
				}
			}

			/// Apply some simulated "friction" or damping at every collission, just in-case?
			dynamicEntity->physics->linearMomentum *= 0.9f;
		//	dynamicEntity->physics->angularMomentum *= 0.95f;
			
			/// Extract collissionpoint
			Vector3f collissionPoint = data.collissionPoint;
			
			/// Recalculate the entities velocities for proper rendering/visualization.
			dynamicEntity->physics->StorePreviousVelocities();
			dynamicEntity->physics->CalculateVelocity();
			dynamicEntity->RecalculateMatrix();
			dynamicEntity->physics->obb.Recalculate(dynamicEntity);
		} // End Static-Dynamic collission resolution.
		*/
		return true;
    }
    /// Differentiate between implementation.
    else if (Physics.collissionResolver == CUSTOM_SPACE_RACE_PUSHBACK){

        assert(data.results & DISTANCE_INTO);
        /// The one and only.
        Vector3f collissionNormal = data.collissionNormal;
        float distanceIntoEachOther = abs(data.distanceIntoEachOther);
        if (distanceIntoEachOther < 0.0001f)
            distanceIntoEachOther = 0.0001f;

        /// Calculate attributes common to all collissions
        float friction = min(one->physics->friction, two->physics->friction);
        float restitution = one->physics->restitution * two->physics->restitution;

        /// For one-sided calculations
        if (dynamicEntity){

            /// Skip collissions where the velocity and collission normal point the same way ^^
            if (collissionNormal.DotProduct(dynamicEntity->physics->velocity) > 0.0f){
                return false;
            }

			Vector3f velPreCol = dynamicEntity->physics->velocity;

            /// Velocity along the normal
            Vector3f nVelocity = dynamicEntity->physics->velocity.DotProduct(collissionNormal) * collissionNormal;
            /// Velocity along tangent to the collission normal
            Vector3f tVelocity = dynamicEntity->physics->velocity - nVelocity;
            // Mirror the dynamic entity's velocity along the collission normal
			float frictionModifier = 1 - friction;
			assert(frictionModifier < 1.f);
			assert(restitution < 1.f);
            dynamicEntity->physics->velocity = tVelocity * frictionModifier - nVelocity * restitution;
            dynamicEntity->physics->angularVelocity *= 1 - friction;

			/// Apply overall collission damping.
			Vector3f velDir = dynamicEntity->physics->velocity.NormalizedCopy();
			if (dynamicEntity->physics->velocity.Length() > 1500.0f){
				// std::cout<<"Lall";
			}
	
			/*
			float velDotNormal = velDir.DotProduct(collissionNormal);
			// If going into a wall, straight or so, apply damping?
			float collissionDamping = velDotNormal;
			Clamp(collissionDamping, 0, 1);
			float damping = pow(0.1f, collissionDamping);
			dynamicEntity->physics->velocity *= damping;
			if (damping < 0.99f)
				std::cout<<"\nDamping: "<<damping;
			*/

			//float yPart = AbsoluteValue(collissionNormal.y);
			///// Using yPart, apply proper collission damping to negate the strange bug.
			//// If close to 1 in Y, apply 0 damping since the regular friction is working decently. However, as Y part approaches 0, apply damping to full degree.
			//float damping = pow(0.95f, 1.0f - yPart);
			//std::cout<<"\nDamping: "<<damping;
			//dynamicEntity->physics->velocity *= damping;

			Vector3f velPostCol = dynamicEntity->physics->velocity;
			float absVelPreCol = velPreCol.Length();
			float absVelPostCol = velPostCol.Length();
			assert(absVelPostCol < absVelPreCol);
			float velDecrease = absVelPostCol - absVelPreCol;
//			if (AbsoluteValue(velDecrease) > 5.0f)
//				std::cout<<"\nVelDecrease: "<<velDecrease;

            /// Move back the dynamic entity along the collission normal until it's outside.
            dynamicEntity->position += distanceIntoEachOther * collissionNormal * 1.05f; // * +1% to avoid glitchies
			
            /// Update entity collission state
            UpdateCollissionState(dynamicEntity, collissionNormal);
			// Recalculate matrix for the entity.
			dynamicEntity->RecalculateMatrix();
        }
        /// Two-sided calculations
        else {
            /// If velocity of one is too small, then that means that two is the collidding fool!
            if (!one->physics->velocity.Length()){
                /// If both colliding parts have 0 length, just make them stay still and go home, yo...
                if (!two->physics->velocity.Length()){
                    std::cout<<"\nColliders "<<one->id<<" & "<<two->id<<" too slow o-o";
                    /// Update collission states for both entities!
                    Vector3f oneVec(1,1,1);
                    UpdateCollissionState(two, oneVec);
                    UpdateCollissionState(one, oneVec);
                    return false;
                }
                /// Swap parts
                Entity * three = one;
                one = two;
                two = three;
            }

            /// Velocity along the normal
            float oneDotNormal = one->physics->velocity.DotProduct(collissionNormal);
            float twoDotNormal = two->physics->velocity.DotProduct(collissionNormal);
            Vector3f nVelocity = one->physics->velocity.DotProduct(collissionNormal) * collissionNormal;
            Vector3f n2Velocity = two->physics->velocity.DotProduct(collissionNormal) * collissionNormal;
            float totalNormalVelocity = nVelocity.Length() + n2Velocity.Length();
            /// Separate the impact velocities into the two's normal velocities
            nVelocity = nVelocity.Normalize() * totalNormalVelocity * 1.0f;
            n2Velocity = n2Velocity.Normalize() * totalNormalVelocity * 1.0f;
            /// Velocity along tangent to the collission normal
            Vector3f tVelocity = one->physics->velocity - nVelocity;
            Vector3f t2Velocity = two->physics->velocity - n2Velocity;

            // Mirror the dynamic entity's velocity along the collission normal
            float friction = min(one->physics->friction, two->physics->friction);
            float restitution = one->physics->restitution * two->physics->restitution;
            one->physics->velocity = ((tVelocity - t2Velocity) * (1 - friction) - (nVelocity - n2Velocity) * restitution) * 0.5f;
            two->physics->velocity = ((t2Velocity - tVelocity) * (1 - friction) - (n2Velocity - nVelocity) * restitution) * 0.5f;

            one->physics->angularVelocity *= 1 - friction;
            two->physics->angularVelocity *= 1 - friction;

            /// Move back both entities along their normals half the distance
            Vector3f awayFromTwo = (one->position - two->position).Normalize();
            Vector3f awayFromOne = (two->position - one->position).Normalize();
            one->position += distanceIntoEachOther * awayFromTwo * 0.55f; // * +2% to avoid glitchies
            two->position += distanceIntoEachOther * awayFromOne * 0.55f;	// * +2% to avoid glitchies

            /// Update collission states for both entities!
            UpdateCollissionState(two, collissionNormal);
            UpdateCollissionState(one, collissionNormal);
        }
    }
	return false;
}
