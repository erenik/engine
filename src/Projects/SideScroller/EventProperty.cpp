/// Emil Hedemalm
/// 2015-05-07
/// Attached to entities, will make the entity respond to collisions, initiating certain effects and procedures based on what has been flagged after creation.

#include "EventProperty.h"
#include "SideScroller.h"

EventProperty::EventProperty(Entity * owner)
: EntityProperty("Event", 5, owner)
{
	triggerTimes = 1;
	sleepOnTrigger = true;
	asleep = false;
	stopPlayer = false;
}

void EventProperty::OnCollision(Collision & data)
{
	if (asleep)
		return;
	if (triggerTimes == 1)
		asleep = true;
	/// o.o
	if (playSFX.Length())
		AudioMan.QueueMessage(new AMPlaySFX(playSFX));

	Entity * other = data.one == owner? data.two : data.one;
//	if (stopAcceleration)
//		other->physics->acceleration = Vector3f();
	if (stopPlayer)
	{
		other->physics->velocity = other->physics->acceleration = Vector3f();
		Message stopMsg("Stop");
		other->ProcessMessage(&stopMsg);
	}
	if (message.Length())
		MesMan.QueueMessages(message);
}

void EventProperty::Process(int timeInMs){}
void EventProperty::ProcessMessage(Message * message){}
void EventProperty::OnCollisionCallback(CollisionCallback * cc){}

/// Creates an Entity featuring an EventProperty at target X position, scaling vertically. Collides by default with the player only.
void CreateEventEntity(Entity ** eventEntity, EventProperty ** eventProp)
{
	/// Add a reactionary entity to play the SFX?
	Model * cube = ModelMan.GetModel("obj/cube.obj");
	Texture * white = TexMan.GetTexture("0xFF");


	Entity * entity	= EntityMan.CreateEntity("sfx event", cube, white);
	entity->scale.y = 1000;
	if (!entity->physics)
		entity->physics = new PhysicsProperty();
	entity->physics->shapeType = ShapeType::AABB;
	entity->physics->collisionFilter |= CC_PLAYER;
	entity->physics->onCollision = true;
	entity->physics->noCollisionResolutions = true;

	*eventProp = new EventProperty(entity);
	entity->properties.AddItem(*eventProp);

	*eventEntity = entity;
}

