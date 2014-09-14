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

/// Call to reset time alive to 0.
void SpaceShooterExplosionProperty::OnSpawn()
{
	timeAliveMs = 0;
	sleeping = false;
	Graphics.QueueMessage(new GMSetEntityf(owner, GT_ALPHA, 0.f));
	// Start an animation which adjusts alpha of this entity from 1.f to 0.f over a certain duration.
	EstimatorFloat * estimator = new EstimatorFloat();
	estimator->AddStates(4, 
		0.f, 0,
		1.f, 100,
		1.5f, 1000,
		0.f, 4000);
	/*
	estimator->AddState(0.f, 0);
	estimator->AddState(1.f, 100);
	estimator->AddState(1.f, 1000);
	estimator->AddState(0.f, 4000);
	*/
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
