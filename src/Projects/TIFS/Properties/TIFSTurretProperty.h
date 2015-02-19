/// Emil Hedemalm
/// 2014-07-30
/// Property/Controller class for all turrets, providing aiming- and appropriate rotations for them, as well as handling repair/damage states.

#ifndef TIFS_TURRET_PROPERTY_H
#define TIFS_TURRET_PROPERTY_H

#include "TIFSProperties.h"
#include "MathLib/Angle.h"
#include "MathLib.h"

class TIFSTurretProperty : public EntityProperty
{
public:
	// The base will become the owner... I think. Or all of them.. ? o.O
	TIFSTurretProperty(Entity * base, Entity * swivel, Entity * underBarrel, Entity * barrel);

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	
	static int ID();

	// Might not use
	Entity * target, * targetLastFrame;
	// Parts
	Entity * base, * swivel, * underBarrel, * barrel;

	/// Defaults. Setable via script while testing around.
	static int defaultTurretCooldown;
	static float defaultPitchYawPerSecond;
	static float defaultRecoilSpringConstant;
	static float defaultRecoilLinearDamping;
	static float defaultProjectileSpeed;
	static float defaultRecoilSpeed;
	static float defaultCooldown;

	/// Determines how fast it may turn?
	int turretSize;
	float yawPerSecond;
	float pitchPerSecond;
	Angle lastYaw;
	Angle lastPitch;

	int currentHP, maxHP;
	bool active;
	// current cooldown.
	int shootCooldown;
	// weapon cooldown max
	int weaponCooldownMs;
	float projectileSpeed;
private:
	bool springsCreated;

	bool shoot;
	void Aim();
	bool Reload(int timeInMs);
	void Shoot();

	Vector3f toTarget;
	float distanceToTarget;
	Vector3f toTargetNormalized;

};

#endif
