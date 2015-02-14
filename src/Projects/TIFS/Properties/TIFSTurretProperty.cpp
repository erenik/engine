/// Emil Hedemalm
/// 2014-07-30
/// Property/Controller class for all turrets, providing aiming- and appropriate rotations for them, as well as handling repair/damage states.

#include "TIFSTurretProperty.h"
#include "TIFSProjectile.h"

#include "TIFS/TIFS.h"

#include "Physics/Messages/PhysicsMessage.h"
#include "Entity/EntityManager.h"

int TIFSTurretProperty::defaultTurretCooldown = 2000;
float TIFSTurretProperty::defaultPitchYawPerSecond = 0.2f;

TIFSTurretProperty::TIFSTurretProperty(Entity * base, Entity * swivel, Entity * underBarrel, Entity * barrel)
: EntityProperty("TIFSTurretProperty", TIFSProperty::TURRET, NULL), base(base), swivel(swivel), underBarrel(underBarrel), barrel(barrel)
{
	// Add owners manually
	owners.Add(base, swivel, underBarrel, barrel);

	projectileSpeed = 1000.f;
	shootCooldown = weaponCooldownMs = defaultTurretCooldown;
	maxHP = 40000;
	currentHP = 20000;

	pitchPerSecond = yawPerSecond = defaultPitchYawPerSecond;
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
	shoot = false;
	if (target)
	{
		Aim();
	}
	shootCooldown -= timeInMs;
	if (shootCooldown > 0)
		shoot = false;

	// Shoot at it.
	if (shoot)
	{
		Shoot();
	}
};


void TIFSTurretProperty::Aim()
{
	toTarget = target->position - barrel->worldPosition;
	distanceToTarget = toTarget.Length();
	toTargetNormalized = toTarget.NormalizedCopy();
	Vector2f toTargetXZ(toTarget.x, toTarget.z);
	toTargetXZ.Normalize();
	Angle requiredYaw(toTargetXZ);
	requiredYaw -= Angle(PI/2);
	// Get current pitch and yaw?
	Vector3f lookAt = base->LookAt();
	Vector3f defaultLookAt(0,0,-1);
	Angle defaultAngle(defaultLookAt.x, defaultLookAt.z);
	Vector2f xz(lookAt.x, lookAt.z);
	xz.Normalize();
	Angle yaw(xz);
	Angle currentYaw = yaw - defaultAngle;

	Angle yawDiff = requiredYaw - currentYaw;
	float yawVel = yawDiff.Radians();
	ClampFloat(yawVel, -yawPerSecond, yawPerSecond);

	/// Set rotation speeds accordingly?
	Quaternion yawQuat(Vector3f(0,1,0), yawVel);
	assert(yawQuat.x == yawQuat.x);
	PhysicsMan.QueueMessage(new PMSetEntity(base, PT_ANGULAR_VELOCITY, yawQuat));

	Angle pitchNeeded(asin(toTargetNormalized.y));
	lookAt = underBarrel->LookAt();
	lookAt.y *= -1;
	Angle currentPitch(asin(lookAt.y));
//		currentPitch = Angle(PI/2) - currentPitch;
	Angle pitchDiff = pitchNeeded - currentPitch;
	float pitchVel = pitchDiff.Radians();
	ClampFloat(pitchVel, -pitchPerSecond, pitchPerSecond);
//		std::cout<<"\nLookAt: "<<lookAt;
	PhysicsMan.QueueMessage(new PMSetEntity(underBarrel, PT_ANGULAR_VELOCITY, Quaternion(Vector3f(1,0,0), pitchVel)));

	// Barrel look-at?
	Vector3f barrelLookAt = -barrel->LookAt();
	float dot = barrelLookAt.DotProduct(toTargetNormalized);
	if (dot > 0.9f)
		shoot = true;


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


void TIFSTurretProperty::Shoot()
{
	shootCooldown = weaponCooldownMs;
	// Emit some particles at the cannon
	
	// Create laser entity, or bullet entity, depending on what is wanted?
	Entity * projEntity = EntityMan.CreateEntity("Laser", ModelMan.GetModel("sphere.obj"), TexMan.GetTexture("0x00AAFFFF"));
	TIFSProjectile * proj = new TIFSProjectile(projEntity);
	projEntity->properties.Add(proj);
	
	GraphicsProperty * gp = new GraphicsProperty(projEntity);
	gp->castsShadow = false;
	projEntity->SetTexture(EMISSIVE_MAP, TexMan.GetTexture("0x00FF00FF"));
	projEntity->graphics = gp;

	PhysicsProperty * pp = new PhysicsProperty();
	projEntity->physics = pp;
	pp->owner = projEntity;
	// Set dynamic and collision with drones.
	pp->type = PhysicsType::KINEMATIC;
	pp->collisionCategory = CC_LASER;
	pp->collisionFilter = CC_DRONE | CC_ENVIRON;

	Vector3f midPosition = (projEntity->position + target->position) * 0.5f;
	// Set scale?
	projEntity->scale = Vector3f(0.25,0.25,15.f);
	// Rotate?
	projEntity->position = this->barrel->transformationMatrix * Vector4f(0,0,0,1) + toTargetNormalized * 0.1f;
	// Calculate rotations? 
	// Grab look-at from barrel?
	Vector3f barrelLookAt = -barrel->LookAt();
	Vector3f barrelUp = barrel->UpVec();
	Vector3f barrelRight = barrel->RightVec();

	projEntity->localRotation.SetVectors(barrelRight, barrelUp, -barrelLookAt);
	projEntity->RecalculateMatrix(2);

	Vector3f upVec = projEntity->UpVec(),
		rightVec = projEntity->RightVec();
	Vector3f lookAt = projEntity->LookAt();
	
	float upDotRight = upVec.DotProduct(rightVec);
	float upDotLookAt = upVec.DotProduct(lookAt);

	pp->relativeVelocity = Vector3f(0,0,1) * projectileSpeed;

	Vector3f relVelWorldSpaced;
	if (pp->relativeVelocity.MaxPart())
	{
		// Add it.
		Vector3f relVel = pp->relativeVelocity;
		relVel.z *= -1;
		relVelWorldSpaced = projEntity->rotationMatrix * relVel;
	}

	std::cout<<"\nRelVelWorldspaced: "<<relVelWorldSpaced;

	float dot = lookAt.DotProduct(toTargetNormalized);
	if (dot < 0.9f)
	{
		std::cout<<"\nFiring into nether..!";
	}

	MapMan.AddEntity(projEntity);

	// Try give it a velocity too?

	// Emit particles at the target?

}





