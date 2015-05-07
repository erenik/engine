/// Emil Hedemalm
/// 2015-05-07
/// Attached to entities, will make the entity respond to collisions, initiating certain effects and procedures based on what has been flagged after creation.

#ifndef EVENT_PROPERTY_H
#define EVENT_PROPERTY_H

#include "Entity/EntityProperty.h"

class Entity;
struct Collision;
class Message;
class CollisionCallback;

class EventProperty : public EntityProperty 
{
public:
	EventProperty(Entity * owner);
	void OnCollision(Collision & data);	
	void Process(int timeInMs);
	void ProcessMessage(Message * message);
	void OnCollisionCallback(CollisionCallback * cc);

	/// Default 1.
	int triggerTimes;
	/// Default true.
	bool sleepOnTrigger; 
	/// Default false. When true, does nothing.
	bool asleep;

	/// IF contains string, play it. Default empty.
	String playSFX;

	/// Resets vel and acc. Default false.
	bool stopPlayer;
	String message;
//	bool stopAcceleration;

protected:
	

};

/// Creates an Entity featuring an EventProperty at target X position, scaling vertically. Collides by default with the player only.
void CreateEventEntity(Entity ** eventEntity, EventProperty ** eventProp);


#endif


