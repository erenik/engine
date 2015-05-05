/// Emil Hedemalm
/// 2015-04-20
/// Player property.

#include "SideScroller.h"
#include "Luchador.h"

LuchadorProperty::LuchadorProperty(Entity * owner)
	: EntityProperty("LuchaProp", EP_LUCHA, owner)
{
	sleeping = false;
	lastJump = Time::Now();
}
void LuchadorProperty::OnCollision(Collision & data)
{
	Entity * other = NULL;
	if (data.one == owner)
		other = data.two;
	else
		other = data.one;
	if (other->physics->collisionCategory == CC_ENVIRONMENT)
	{
		// o.o
		if (AbsoluteValue(data.collisionNormal.y) > 0.8f)
		{
			GMPlayAnimation anim("Run", owner);
			anim.Process(); // Process straight away, no use queueing it up.
		}
	}
}
void LuchadorProperty::Process(int timeInMs)
{
	if (sleeping) 
		return;
	distance = owner->position.x;
	sideScroller->UpdateDistance();
	if (owner->position.y < -2.f)
	{
		// Deaded.
		QueuePhysics(new PMSetEntity(owner, PT_PHYSICS_TYPE, PhysicsType::STATIC));
		attempts->iValue += 1;
		sideScroller->UpdateAttempts();
		// Add up total munny.
		totalMunny->iValue += munny;
		// Auto-save?
		sideScroller->AutoSave();
		sleeping = true;
		QueueGraphics(new GMPlayAnimation("Idle", owner));
		sideScroller->GameOver();
//		ScriptMan.PlayScript("scripts/OnDeath.txt");
	}
}

void LuchadorProperty::ProcessMessage(Message * message)
{
	String & msg = message->msg;
	if (msg == "Jump")
	{
		if (sleeping)
			return;
		int jumpCooldownMs = 500;
		if ((now - lastJump).Milliseconds() < jumpCooldownMs)
			return;
		lastJump = now;
		QueuePhysics(new PMSetEntity(owner, PT_VELOCITY, owner->Velocity() + Vector3f(0,5.f,0)));
		// Set jump animation! o.o
		QueueGraphics(new GMPlayAnimation("Jump", owner));
	}
}

void LuchadorProperty::OnCollisionCallback(CollisionCallback * cc)
{
	Entity * other = NULL;
	if (cc->one == owner)
		other = cc->two;
	else
		other = cc->one;
	if (other->physics->collisionCategory == CC_ENVIRONMENT)
	{
		// o.o
		if (cc->impactNormal.y > 0.8f)
		{
			GMPlayAnimation anim("Run", owner);
			anim.Process(); // Process straight away, no use queueing it up.
		}
	}
}

