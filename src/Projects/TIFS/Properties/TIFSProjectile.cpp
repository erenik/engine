/// Emil Hedemalm
/// 2015-02-13
/// Projectile property.

#include "TIFSProjectile.h"
#include "Maps/MapManager.h"

TIFSProjectile::TIFSProjectile(Entity * owner)
: EntityProperty("TIFSProjectile property", ID(),owner)
{
	livedMs = 0;
	lifeTimeMs = 2000;
	sleeping = false;
}

int TIFSProjectile::ID()
{
	return 16;
}

void TIFSProjectile::Process(int timeInMs)
{
	livedMs += timeInMs;
	if (livedMs > lifeTimeMs && !sleeping)
	{
		MapMan.DeleteEntity(owner);
		sleeping = true;
	}
}
