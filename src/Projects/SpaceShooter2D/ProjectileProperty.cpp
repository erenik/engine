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
	if (sleeping)
		return;
	// Remove self.
	sleeping = true;
	MapMan.DeleteEntity(owner);
	spaceShooter->projectileEntities.Remove(owner);

	// Check distance to player.
//	Vector3f vectorDistance = (player1->position - atPosition);
//	vectorDistance /= 100.f;
//	float distSquared = vectorDistance.Length();
//	float distanceModifierToVolume = 1 / distSquared;
//	if (distanceModifierToVolume > 1.f)
//		distanceModifierToVolume = 1.f;

	// Add a temporary emitter to the particle system to add some sparks to the collision
	SparksEmitter * tmpEmitter = new SparksEmitter(owner->position);
	tmpEmitter->SetEmissionVelocity(5.f);
	tmpEmitter->particlesPerSecond = 2000;
	tmpEmitter->deleteAfterMs = 20;	
	tmpEmitter->SetParticleLifeTime(3.5f);
	tmpEmitter->SetScale(0.1f);
	tmpEmitter->SetColor(Vector4f(0.1f, 0.5f, 1.f, 1.f));
	Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));

//	float volume = distanceModifierToVolume * explosionSFXVolume;
	// Play SFX!
//	AudioMan.QueueMessage(new AMPlaySFX("SpaceShooter/235968__tommccann__explosion-01.wav", volume));
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
