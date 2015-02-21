/// Emil Hedemalm
/// 2014-07-30
/// Property/Controller class for all turrets, providing aiming- and appropriate rotations for them, as well as handling repair/damage states.

#include "TIFSTurretProperty.h"
#include "TIFSProjectile.h"
#include "TIFSDroneProperty.h"

#include "TIFS/TIFS.h"

#include "Physics/Messages/PhysicsMessage.h"
#include "Entity/EntityManager.h"
#include "StateManager.h"
#include "MathLib/Angle3.h"

int TIFSTurretProperty::defaultTurretCooldown = 2000;
float TIFSTurretProperty::defaultPitchYawPerSecond = 0.2f;
float TIFSTurretProperty::defaultRecoilSpringConstant = 225.f;
float TIFSTurretProperty::defaultRecoilLinearDamping = 0.125f;
float TIFSTurretProperty::defaultProjectileSpeed = 1000.f;
float TIFSTurretProperty::defaultRecoilSpeed = 1.0f;

TIFSTurretProperty::TIFSTurretProperty(Entity * base, Entity * swivel, Entity * underBarrel, Entity * barrel)
: EntityProperty("TIFSTurretProperty", TIFSProperty::TURRET, NULL), base(base), swivel(swivel), underBarrel(underBarrel), barrel(barrel)
{
	// Add owners manually
	owners.Add(base, swivel, underBarrel, barrel);

	springsCreated = false;
	projectileSpeed = defaultProjectileSpeed;
	shootCooldown = weaponCooldownMs = defaultTurretCooldown;
	maxHP = 40000;
	currentHP = 20000;

	pitchPerSecond = yawPerSecond = defaultPitchYawPerSecond;
	targetLastFrame = NULL;
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

	/// Do nothing if not active?
	if (!active)
		return;

	// Is target eliminated?
	if (!target)
	{
		// Get new target
		if (tifs->drones.Size())
		{
			// Get closest target?
			float minSquaredLen = 100000;
			Entity * closest = NULL;
			for (int i = 0; i < tifs->drones.Size(); ++i)
			{
				Entity * drone = tifs->drones[i];
				TIFSDroneProperty * droneProp = (TIFSDroneProperty * )drone->properties[0];
				if (!droneProp->isActive)
					continue;
				float squaredLen = (drone->position - base->position).LengthSquared();
				if (squaredLen < minSquaredLen)
				{
					closest = drone;
					minSquaredLen = squaredLen;
				}
			}
			target = closest;
		}
	}
	// Aim at it.
	shoot = false;
	if (target)
	{
		Aim();
	}
	// If not target, and not issued stop command..
	else if (target != targetLastFrame)
	{
		std::cout<<"\nLost sight of target, stopping.";
		// Stop rotations if no target is there.
		PhysicsQueue.Add(new PMSetEntity(base, PT_ANGULAR_VELOCITY, Quaternion()));
		PhysicsQueue.Add(new PMSetEntity(underBarrel, PT_ANGULAR_VELOCITY, Quaternion()));
	}
	if (Reload(timeInMs))
		shoot = false;
	
	// Shoot at it.
	if (shoot)
	{
		Shoot();
	}

	targetLastFrame = target;
};


void TIFSTurretProperty::Activate()
{
	active = true;
}


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
	PhysicsQueue.Add(new PMSetEntity(base, PT_ANGULAR_VELOCITY, yawQuat));

	Angle pitchNeeded(asin(toTargetNormalized.y));
	lookAt = underBarrel->LookAt();
	lookAt.y *= -1;
	float arcSin = asin(lookAt.y);
	assert(arcSin == arcSin);
	Angle currentPitch(arcSin);
//		currentPitch = Angle(PI/2) - currentPitch;
	Angle pitchDiff = pitchNeeded - currentPitch;
	float pitchVel = pitchDiff.Radians();
	ClampFloat(pitchVel, -pitchPerSecond, pitchPerSecond);
//		std::cout<<"\nLookAt: "<<lookAt;
	Quaternion pitchQuat(Vector3f(1,0,0), pitchVel);
	assert(pitchQuat.x == pitchQuat.x);
	PhysicsQueue.Add(new PMSetEntity(underBarrel, PT_ANGULAR_VELOCITY, pitchQuat));

	// Barrel look-at?
	Vector3f barrelLookAt = -barrel->LookAt();
	float dot = barrelLookAt.DotProduct(toTargetNormalized);
	if (dot > 0.9f)
		shoot = true;

	/// Test if the new Angle3 class is of any use here.
	Angle3 toRotate = Angle3::GetRequiredRotation(lookAt, toTarget);
//	std::cout<<"\nToRotate: "<<toRotate;


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


bool TIFSTurretProperty::Reload(int timeInMs)
{
	shootCooldown -= timeInMs;
	if (shootCooldown > 0)
		return true;
	// Stuff.
	return false;
}

void TIFSTurretProperty::Shoot()
{
	shootCooldown = weaponCooldownMs;
	if (!springsCreated)
	{
		springsCreated = true;
		PhysicsQueue.Add(new PMCreateSpring(barrel, Vector3f(), defaultRecoilSpringConstant, 0));
	}
	// Apply impulse.
	PhysicsQueue.Add(new PMApplyImpulse(barrel, Vector3f(0,0,-defaultRecoilSpeed) * barrel->physics->mass, Vector3f()));

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
	projEntity->Scale(Vector3f(0.25,0.25,15.f));
	float length = projEntity->scale.z;
	// Rotate?
	// Calculate rotations? 
	// Grab look-at from barrel?
	Vector3f barrelLookAt = -barrel->LookAt();
	Vector3f barrelUp = barrel->UpVec();
	Vector3f barrelRight = barrel->RightVec();

	projEntity->localRotation.SetVectors(barrelRight, barrelUp, -barrelLookAt);
	projEntity->rotationMatrix = projEntity->localRotation;
	projEntity->hasRotated = false;
	projEntity->RecalculateMatrix(Entity::ALL_BUT_ROTATION);
	
	// Using the updated transform of the actual projectile, derive position for it...
	Vector3f vectorDir = projEntity->rotationMatrix * Vector4f(0,0,-1,0);
	toTargetNormalized = vectorDir.NormalizedCopy();
	projEntity->position = this->barrel->transformationMatrix * Vector4f(0,0,0,1) + toTargetNormalized * length;
	projEntity->position += Vector3f(0,2.f,0);
	projEntity->RecalculateMatrix(Entity::TRANSLATION_ONLY); // Update the position we just set.

	pp->relativeVelocity = Vector3f(0,0,-1) * projectileSpeed;
	// Derive velocity...
	pp->currentVelocity = projEntity->rotationMatrix * pp->relativeVelocity;


	Vector3f relVelWorldSpaced;
	if (pp->relativeVelocity.MaxPart())
	{
		// Add it.
		Vector3f relVel = pp->relativeVelocity;
		relVel.z *= -1;
		relVelWorldSpaced = projEntity->rotationMatrix * relVel;
	}

	MapMan.AddEntity(projEntity);

	// Try give it a velocity too?

	// Emit particles at the target?

}





