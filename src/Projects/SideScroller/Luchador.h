/// Emil Hedemalm
/// 2015-04-20
/// Player property.

#ifndef LUCHADOR_H
#define LUCHADOR_H

#include "Entity/EntityProperty.h"

class Entity;
struct Collision;
class Message;
class CollisionCallback;

class LuchadorProperty : public EntityProperty 
{
public:
	LuchadorProperty(Entity * owner);
	virtual void OnCollision(Collision & data);	
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);
	virtual void OnCollisionCallback(CollisionCallback * cc);
protected:
	bool sleeping;
	Time lastJump;
};

#endif
