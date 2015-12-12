/// Emil Hedemalm
/// 2015-01-21
/// Projectile.

#include "SpaceShooter2D/SpaceShooter2D.h"
#include "SpaceShooter2D/Properties/ExplosionProperty.h"
#include "PhysicsLib/EstimatorFloat.h"

ExplosionProperty::ExplosionProperty(const Weapon & weaponThatSpawnedIt, Entity * owner)
: EntityProperty("ProjProp", ID(), owner), weapon(weaponThatSpawnedIt)
{
	sleeping = false;
	timeAliveMs = 0;
	totalDamageInflicted = 0;
}

// Static version.
int ExplosionProperty::ID()
{
	return EXPL_PROP;
}

/// If reacting to collisions...
void ExplosionProperty::OnCollision(Collision & data)
{
	// Do nothing?
//	Destroy();
}

void ExplosionProperty::Remove()
{
	if (sleeping)
		return;
	// Remove self.
	sleeping = true;
	MapMan.DeleteEntity(owner);
//	projectileEntities.Remove(owner);
}

/// Time passed in seconds..!
void ExplosionProperty::Process(int timeInMs)
{
	if (sleeping)
		return;
	if (paused)
		return;
	timeAliveMs += timeInMs;
	if (timeAliveMs > duration)
		Remove();
}


void ExplosionProperty::ProcessMessage(Message * message)
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

float ExplosionProperty::CurrentDamage()
{
	float currDmg = weapon.damage * pow((float)(1 - timeAliveMs/(float)duration), 2);
	totalDamageInflicted += currDmg;
	std::cout<<"\nCurrent Damage: "<<currDmg<<" total: "<<totalDamageInflicted;
	return currDmg;
}
bool ExplosionProperty::ShouldDamage(Ship * ship)
{
	if (damagedTargets.Exists(ship))
		return false;
	return true;
}
