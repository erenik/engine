/// Emil Hedemalm
/// 2014-07-31
/// An explosion o-o

#include "SpaceShooterExplosionProperty.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "PhysicsLib/EstimatorFloat.h"

#include "Maps/MapManager.h"

SpaceShooterExplosionProperty::SpaceShooterExplosionProperty(Entity * owner)
	: EntityProperty("SpaceShooterExplosionProperty", ID(), owner)
{
}

/// ID of this class, also assigned to all created properties of this type.
int SpaceShooterExplosionProperty::ID()
{
	return 5;
}

void SpaceShooterExplosionProperty::SetType(int newType)
{
	this->type = newType;
}

/// Call to reset time alive to 0.
void SpaceShooterExplosionProperty::OnSpawn()
{
	timeAliveMs = 0;
	sleeping = false;
	Graphics.QueueMessage(new GMSetEntityf(owner, GT_ALPHA, 0.f));
	// Start an animation which adjusts alpha of this entity from 1.f to 0.f over a certain duration.
	EstimatorFloat * estimator = new EstimatorFloat();
	switch(type)
	{
		case ExplosionType::PROJECTILE:
			estimator->AddStatesMs(3, 
				0.f, 0,
				1.f, 100,
				0.f, 1000);
			// Set scale accordingly too.
			Physics.QueueMessage(new PMSetEntity(this->owner, PT_SET_SCALE, 25.f));
			break;
		case ExplosionType::SHIP:
			estimator->AddStatesMs(4, 
				0.f, 0,
				1.f, 100,
				1.1f, 150,
				0.f, 2000);
			// Set scale accordingly too.
			Physics.QueueMessage(new PMSetEntity(this->owner, PT_SET_SCALE, 65.f));
			break;
		default:
			assert(false);
	}
	Graphics.QueueMessage(new GMSlideEntityf(owner, GT_ALPHA, estimator));
}

/// Time passed in seconds..!
void SpaceShooterExplosionProperty::Process(int timeInMs)
{
	if (sleeping)
		return;

	timeAliveMs += timeInMs;
	if (timeAliveMs > 6000)
	{
		// Unregister self.
		Graphics.QueueMessage(new GMUnregisterEntity(owner));
		Physics.QueueMessage(new PMUnregisterEntity(owner));
//		MapMan.DeleteEntity(owner);
		sleeping = true;
	}
}
