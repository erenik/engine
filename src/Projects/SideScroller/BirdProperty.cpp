/// Emil Hedemalm
/// 2015-05-08
/// Yarp

#include "BirdProperty.h"	
#include "SideScroller.h"

BirdProperty::BirdProperty(Entity * owner)
 : EntityProperty("BirdProp", EP_BIRD, owner)
{
	collided = false;
	constantVelocityReduction = 3.f;
	velocity = 3.f;
}

void BirdProperty::OnCollision(Collision & data)
{
	if (collided)
		return;
	// Collide with ze player!!!
	// o.o
	Entity * other = data.one == owner? data.two : data.one;
	float & velX = owner->physics->velocity.x;
	other->physics->velocity.x -= constantVelocityReduction;
	velX *= 0.5f;
	/// In addition to that, reduce player speed by, say, 25%?
	other->physics->velocity.x *= 0.55f;

	collided = true;
}
void BirdProperty::Process(int timeInMs)
{

}

void BirdProperty::ProcessMessage(Message * message)
{
	String msg = message->msg;
	if (msg.StartsWith("Boost"))
	{
		float factor = msg.Tokenize(":")[1].ParseFloat();
		constantVelocityReduction *= factor;
		velocity *= factor;
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f(-velocity, 0, 0)));
	}
}
void BirdProperty::OnCollisionCallback(CollisionCallback * cc)
{

}


void BirdProperty::SetupForFlight()
{
	PhysicsProperty * pp = NULL;
	if (!owner->physics)
		owner->physics = new PhysicsProperty();
	pp = owner->physics;
	
	pp->type = PhysicsType::DYNAMIC;
	pp->gravityMultiplier = 0.f;
	pp->shapeType = PhysicsShape::SPHERE;
	pp->collisionCategory = CC_BIRD;
	pp->collisionFilter = CC_PLAYER;
	pp->onCollision = true;

	/// Give some default X vel
	QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, Vector3f(-velocity, 0,0)));
}