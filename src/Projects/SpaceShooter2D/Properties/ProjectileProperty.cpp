/// Emil Hedemalm
/// 2015-01-21
/// Projectile.

#include "SpaceShooter2D/SpaceShooter2D.h"
#include "SpaceShooter2D/Properties/ProjectileProperty.h"

ProjectileProperty::ProjectileProperty(const Weapon & weaponThatSpawnedIt, Entity * owner, bool enemy)
: EntityProperty("ProjProp", ID(), owner), weapon(weaponThatSpawnedIt), enemy(enemy)
{
	sleeping = false;
	timeAliveMs = 0;
	nextWobbleMs = 0;
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
//	Destroy();
}

void ProjectileProperty::Destroy()
{
	if (sleeping)
		return;
	// Remove self.
	sleeping = true;
	MapMan.DeleteEntity(owner);
	projectileEntities.Remove(owner);

	// Check distance to player.
//	Vector3f vectorDistance = (player1->position - atPosition);
//	vectorDistance /= 100.f;
//	float distSquared = vectorDistance.Length();
//	float distanceModifierToVolume = 1 / distSquared;
//	if (distanceModifierToVolume > 1.f)
//		distanceModifierToVolume = 1.f;


	// Check if an explosion should be spawned in its place.
	if (weapon.explosionRadius > 0.001)
	{
		activeLevel->Explode(weapon, owner, enemy);
		float lifeTime = weapon.explosionRadius / 10.f;
		ClampFloat(lifeTime, 2.5f, 10.f);
		// Explosion emitter o-o should prob. have its own system later on.
		SparksEmitter * tmpEmitter = new SparksEmitter(owner->worldPosition);
		tmpEmitter->SetEmissionVelocity(3.f);
		tmpEmitter->constantEmission = 40 + weapon.damage * weapon.explosionRadius;
		tmpEmitter->instantaneous = true;
		tmpEmitter->SetParticleLifeTime(2.5f);
		tmpEmitter->SetScale(0.15f);
		tmpEmitter->SetColor(color);
		tmpEmitter->SetRatioRandomVelocity(1.f);
		Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));
	}
	else /// Sparks for all physically based projectiles with friction against targets.
	{	
		// Add a temporary emitter to the particle system to add some sparks to the collision
		SparksEmitter * tmpEmitter = new SparksEmitter(owner->worldPosition);
		tmpEmitter->SetEmissionVelocity(3.f);
		tmpEmitter->constantEmission = 40;
		tmpEmitter->instantaneous = true;
		tmpEmitter->SetParticleLifeTime(2.5f);
		tmpEmitter->SetScale(0.1f);
		tmpEmitter->SetColor(color);
		tmpEmitter->SetRatioRandomVelocity(1.f);
		Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));
	}

//	float volume = distanceModifierToVolume * explosionSFXVolume;
//	AudioMan.QueueMessage(new AMPlaySFX("SpaceShooter/235968__tommccann__explosion-01.wav", volume));
}

Random wobbleRand;

/// Time passed in seconds..!
void ProjectileProperty::Process(int timeInMs)
{
	if (sleeping)
		return;
	if (paused)
		return;
	timeAliveMs += timeInMs;
	if (weapon.type == LASER_BURST)
	{
		if (nextWobbleMs == 0 || nextWobbleMs < timeAliveMs)
		{
			bool wasRight = nextWobbleMs & 0x1;
			up = Vector3f(0,1,0);
			// Apply some pulse to the projectile.
			Vector3f velocityImpulse = up * (wasRight? -1 : 1) * timeAliveMs * 0.05f + Vector3f(1,0,0);
			float prevVel = owner->physics->velocity.Length();
			Vector3f newVel = (owner->physics->velocity + velocityImpulse).NormalizedCopy() * prevVel;
			QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, newVel));
			nextWobbleMs += wobbleRand.Randi(30)+timeInMs;
			/// Set 1-bit accordinly so side-chaining changes each pulse.
			if (wasRight)
				nextWobbleMs = nextWobbleMs & ~(0x0001);
			else
				nextWobbleMs = nextWobbleMs | 0x00001;
		}
	}
	else if (weapon.type == HEAT_WAVE)
	{
		// Decay over distance / time.
		distanceTraveled = timeAliveMs * weapon.projectileSpeed * 0.001f;
		float alpha = weapon.relativeStrength = (weapon.maxRange - distanceTraveled) / weapon.maxRange;
		/// Update alpha?
		QueueGraphics(new GMSetEntityf(owner, GT_ALPHA, alpha));
		/// Adjust scale over time?
		float scale = owner->scale.x;
		scale = 1 + (1 - alpha) * weapon.linearScaling;
		QueuePhysics(new PMSetEntity(owner, PT_SET_SCALE, scale));
		if (distanceTraveled > weapon.maxRange)
		{
			// Clean-up.
			sleeping = true;
			return;
		}
	}
	else if (weapon.linearDamping < 1.f)
	{
		if (owner->Velocity().LengthSquared() < 1.f)
			sleeping = true;
	}
	if (weapon.homingFactor > 0)
	{
		// Seek closest enemy.
		// Adjust velocity towards it by the given factor, per second.
		// 1.0 will change velocity entirely to look at the enemy.
		// Values above 1.0 will try and compensate for target velocity and not just current position?
		Entity * closestTarget = spaceShooter->level.ClosestTarget(!enemy, owner->worldPosition);
		if (!closestTarget)
			return;
		Vector3f toTarget = closestTarget->worldPosition - owner->worldPosition;
		toTarget.Normalize();
		// Get current direction.
		float timeFactor = timeInMs * 0.001f;
		float factor = timeFactor * weapon.homingFactor;
		direction = direction * (1 - factor) + toTarget * factor;
		direction.Normalize();
		// Go t'ward it!
		UpdateVelocity();
	}
	// .. 
	if (weapon.projectilePath == Weapon::HOMING)
	{
		// Seek the closest enemy?
		assert(false);
	}
	else if (weapon.projectilePath == Weapon::SPINNING_OUTWARD)
	{
		// Right.
		// 
		// assert(false);
	}
}

void ProjectileProperty::UpdateVelocity()
{
	QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, direction * weapon.projectileSpeed));
}


void ProjectileProperty::ProcessMessage(Message * message)
{
	switch(message->type)
	{
		case MessageType::COLLISSION_CALLBACK:
		{
			if (onCollisionMessage.Length())
				MesMan.QueueMessages(onCollisionMessage);
			break;
		}
	}
}

bool ProjectileProperty::ShouldDamage(Ship * ship)
{
	if (penetratedTargets.Exists(ship))
		return false;
	return true;
}
