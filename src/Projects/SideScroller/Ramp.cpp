/// Emil Hedemalm
/// 2015-05-06
/// Ramp prop

#include "Ramp.h"
#include "SideScroller.h"

RampProp::RampProp(Entity * owner)
: EntityProperty("Rampprop", 3, owner)
{
	speedBonus = 0.3f;
	sleeping = false;
	/// Times before sleeping.
	totalSpeedBonuses = 25;
	speedBonusesGiven = 0;

}
void RampProp::OnCollision(Collision & data)
{
	if (sleeping)
		return;
	Entity * other = data.one == owner? data.two : data.one;
	other->physics->velocity.x += speedBonus;
	++speedBonusesGiven;
	if (speedBonusesGiven >= totalSpeedBonuses)
		sleeping = true;
}	
void RampProp::Process(int timeInMs)
{

}
void RampProp::ProcessMessage(Message * message)
{

}
void RampProp::OnCollisionCallback(CollisionCallback * cc)
{

}
