/// Emil Hedemalm
/// 2014-07-25
/// Properties for the projectiles in a space-shooter game.

#ifndef SPACE_SHOOTER_PROJECTILE_PROPERTY_H
#define SPACE_SHOOTER_PROJECTILE_PROPERTY_H

#include "Entity/EntityProperty.h"

struct SpaceShooterWeaponType 
{
	String name;
	// Determines model/texture, possibly more.
	int type;
	// amount
	int damage;
};

class SpaceShooterProjectileProperty : public EntityProperty 
{
public:
	SpaceShooterProjectileProperty(Entity * owner);

	SpaceShooterWeaponType type;

	// Whose side the projectile belongs to. Determines who it will react to/damage.
	bool player;
	bool enemy;
};

#endif
