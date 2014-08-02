/// Emil Hedemalm
/// 2014-07-31
/// An explosion o-o

#include "SpaceShooterExplosionProperty.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Maps/MapManager.h"

SpaceShooterExplosionProperty::SpaceShooterExplosionProperty(Entity * owner)
	: EntityProperty("SpaceShooterExplosionProperty", 5, owner)
{
	sleeping = false;
}
/// Time passed in seconds..!
void SpaceShooterExplosionProperty::Process(int timeInMs)
{

	Time now = Time::Now();
	int seconds = (now - startTime).Seconds();
	if (seconds > 3)
	{
		// Unregister self.
		Graphics.QueueMessage(new GMUnregisterEntity(owner));
		Physics.QueueMessage(new PMUnregisterEntity(owner));
//		MapMan.DeleteEntity(owner);
		sleeping = true;
	}
}
