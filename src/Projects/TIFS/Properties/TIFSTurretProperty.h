/// Emil Hedemalm
/// 2014-07-30
/// Property/Controller class for all turrets, providing aiming- and appropriate rotations for them, as well as handling repair/damage states.

#include "TIFSProperties.h"
#include "MathLib/Angle.h"

class TIFSTurretProperty : public EntityProperty
{
public:
	// The base will become the owner... I think. Or all of them.. ? o.O
	TIFSTurretProperty(Entity * base, Entity * swivel, Entity * underBarrel, Entity * barrel);

	/// Time passed in seconds..!
	virtual void Process(int timeInMs);
	
	static int ID();

	// Might not use
	Entity * target;
	// Parts
	Entity * base, * swivel, * underBarrel, * barrel;

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
	bool shoot;
	void Aim();
	void Shoot();
};











