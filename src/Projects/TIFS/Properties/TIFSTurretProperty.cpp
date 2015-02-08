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

		float yawNeeded = -atan2(toTargetNormalized.x, toTargetNormalized.z);
		float pitchNeeded = asin(toTargetNormalized.y);
	
		// Just turn it. Straight away. (Sounds scary D:)
		// Yaw the base.
//		Physics.QueueMessage(new PMSetEntity(base, PT_SET_ROTATION, Vector3f(0, yawNeeded, 0)));
		Physics.QueueMessage(new PMSetEntity(base, PT_SET_ROTATION, Quaternion(Vector3f(0, 1, 0), yawNeeded)));

		// Pitch the under-barrel (or barrel, if under-barrel does not hexist?)
//		Physics.QueueMessage(new PMSetEntity(underBarrel, PT_SET_ROTATION, Vector3f(pitchNeeded, 0, 0)));
		Physics.QueueMessage(new PMSetEntity(underBarrel, PT_SET_ROTATION, Quaternion(Vector3f(1, 0, 0), pitchNeeded)));

	
	}
	// Shoot at it.

};











