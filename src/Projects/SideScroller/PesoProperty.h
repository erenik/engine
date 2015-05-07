/// Emil Hedemalm
/// 2015-05-07
/// Peso

#ifndef PESO_H
#define PESO_H

#include "Entity/EntityProperty.h"

class PesoProperty : public EntityProperty 
{
public:
	PesoProperty(Entity * owner);
	virtual void OnCollisionCallback(CollisionCallback * cc);
	bool sleeping;
	int value;
};

#endif