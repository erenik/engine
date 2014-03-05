// Emil Hedemalm
// 2013-07-11

// A ship for racing, yo.
#ifndef SHIP_H
#define SHIP_H

#include "String/AEString.h"
#include "MathLib/Vector3f.h"

class Entity;

class Ship {
	friend class ShipManager;
	// Hide constructor so only the manager can use it!
	Ship();
public:
    /// Sends GMSetTexture messages to the entity, and maybe more parameters later.
    void AssignTexturesToEntity(Entity * entity);
    /// Makes sure it's stats are valid. Returns false if it does not.
	bool HasValidStats();

	String name;
	String source;
	String creator;
	// Resources
	String modelSource;
	String diffuseSource,
		specularSource,
		normalSource;

	// Gameplay stats
	// Local Z-wise acceleration.
	float thrust, reverse;
	// For turning, local angular acceleration.
	float angularThrust;

	/// For boosting the Z-wise acceleration, direct acceleration-multiplier.
	float boostMultiplier;
	/// Max duration of boost (in seconds), and boost regeneration in units per second.
	float maxBoost, boostRegeneration;

	// Survival
	float maxHP, hpRegeneration;

	/// Physics options.
	float velocityRetainedWhileTurning;

	Vector3f thrusterPosition;

private:

};

#endif

