/// Emil Hedemalm
/// 2015-02-13
/// Projectile property.

#ifndef TIFS_PROJECTILE_H
#define TIFS_PROJECTILE_H

#include "Entity/EntityProperty.h"

class TIFSProjectile : public EntityProperty 
{
public:
	TIFSProjectile(Entity * owner);
	virtual void Process(int timeInMs);
	static int ID();
	/// o.o
	int lifeTimeMs;
private:
	int livedMs;
	bool sleeping;
};

#endif


