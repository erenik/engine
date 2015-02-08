/// Emil Hedemalm
/// 2014-07-30
/// Property/Controller class for all turrets, providing aiming- and appropriate rotations for them, as well as handling repair/damage states.

#include "TIFSTurretProperty.h"

#include "TIFS/TIFS.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

TIFSTurretProperty::TIFSTurretProperty(Entity * base, Entity * swivel, Entity * underBarrel, Entity * barrel)
: EntityProperty("TIFSTurretProperty", TIFSProperty::TURRET, NULL), base(base), swivel(swivel), underBarrel(underBarrel), barrel(barrel)
{
	// Add owners manually
	owners.Add(base, swivel, underBarrel, barrel);

	maxHP = 40000;
	currentHP = 20000;
}

int TIFSTurretProperty::ID()
{
	return TIFSProperty::TURRET;
}


/// Time passed in seconds..!
void TIFSTurretProperty::Process(int timeInMs)
{
	// Do we have a target?
	target = NULL;
	// Is target eliminated?

	if (!target)
	{
		// Get new target
		if (tifs->drones.Size())
			target = tifs->drones[0];
	}
	// Aim at it.
	if (target)
	{
		Vector3f toTarget = target->position - barrel->worldPosition;
		float distance = toTarget.Length();
		Vector3f toTargetNormalized = toTarget.NormalizedCopy();
		
		Angle requiredYaw(toTargetNormalized.x, toTargetNormalized.z);
		requiredYaw -= Angle(PI/2);
		// Get current pitch and yaw?
		Vector3f lookAt = base->LookAt();
		Vector3f defaultLookAt(0,0,-1);
		Angle defaultAngle(defaultLookAt.x, defaultLookAt.z);
		Angle yaw(lookAt.x, lookAt.z);
		Angle currentYaw = yaw - defaultAngle;

		Angle yawDiff = requiredYaw - currentYaw;
		float yawVel = yawDiff.Radians();
		ClampFloat(yawVel, -yawPerSecond, yawPerSecond);

		/// Set rotation speeds accordingly?
		PhysicsMan.QueueMessage(new PMSetEntity(base, PT_ANGULAR_VELOCITY, Quaternion(Vector3f(0,1,0), yawVel)));

		Angle pitchNeeded(asin(toTargetNormalized.y));
		lookAt = underBarrel->LookAt();
		lookAt.y *= -1;
		float xz = Vector2f(lookAt.x, lookAt.z).Length();
		Angle currentPitch(asin(lookAt.y));
//		currentPitch = Angle(PI/2) - currentPitch;
		Angle pitchDiff = pitchNeeded - currentPitch;
		float pitchVel = pitchDiff.Radians();
		ClampFloat(pitchVel, -pitchPerSecond, pitchPerSecond);
//		std::cout<<"\nLookAt: "<<lookAt;
		PhysicsMan.QueueMessage(new PMSetEntity(underBarrel, PT_ANGULAR_VELOCITY, Quaternion(Vector3f(1,0,0), pitchVel)));


		// Just turn it. Straight away. (Sounds scary D:)
		// Yaw the base.
		/* // Working stuffs, but not smooth enough! o=o
//		Physics.QueueMessage(new PMSetEntity(base, PT_SET_ROTATION, Vector3f(0, yawNeeded, 0)));
		Physics.QueueMessage(new PMSetEntity(base, PT_SET_ROTATION, Quaternion(Vector3f(0, 1, 0), yawNeeded)));

		// Pitch the under-barrel (or barrel, if under-barrel does not hexist?)
//		Physics.QueueMessage(new PMSetEntity(underBarrel, PT_SET_ROTATION, Vector3f(pitchNeeded, 0, 0)));
		Physics.QueueMessage(new PMSetEntity(underBarrel, PT_SET_ROTATION, Quaternion(Vector3f(1, 0, 0), pitchNeeded)));	
		*/
	}
	// Shoot at it.

};











