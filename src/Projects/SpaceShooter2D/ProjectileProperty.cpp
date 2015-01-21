/// Emil Hedemalm
/// 2015-01-21
/// Projectile.

#include "SpaceShooter2D.h"
#include "ProjectileProperty.h"

ProjectileProperty::ProjectileProperty(Weapon weaponThatSpawnedIt, Entity * owner)
: EntityProperty("ProjProp", ID(), owner), weapon(weaponThatSpawnedIt)
{
	sleeping = false;
}

// Static version.
int ProjectileProperty::ID()
{
	return PROJ_PROP;
}

/// If reacting to collisions...
void ProjectileProperty::OnCollision(Collision & data)
{
	// Do nothing?
	Destroy();
}

void ProjectileProperty::Destroy()
{
	// Remove self.
	sleeping = true;
	MapMan.DeleteEntity(owner);
	spaceShooter->projectileEntities.Remove(owner);
}

/// Time passed in seconds..!
void ProjectileProperty::Process(int timeInMs)
{
	// .. 
	if (weapon.projectilePath == Weapon::HOMING)
	{
		// Seek the closest enemy?
		assert(false);
	}
	else if (weapon.projectilePath == Weapon::SPINNING_OUTWARD)
	{
		assert(false);
	}
}
