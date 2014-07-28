/// Emil Hedemalm
/// 2014-07-28
/** Class that weaves together all elements of a Space-shooter game.
	Encapsulation like this makes it easy to switch in/out various small/mini-games as pleased.
*/

#ifndef SPACE_SHOOTER_WEAPON_TYPE_H
#define SPACE_SHOOTER_WEAPON_TYPE_H

#include "String/AEString.h"

class SpaceShooterWeaponType 
{
public:
	SpaceShooterWeaponType(int type);
	enum {
		RAILGUN,
		MISSILE,
		LASER,
	};

	String name;
	// Determines model/texture, possibly more.
	int type;
	// amount
	int damage;
	/// Cooldown in milliseconds.
	int coolDown;
	/// In units per second.
	float initialVelocity;
};

#endif
