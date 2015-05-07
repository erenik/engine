/// Emil Hedemalm
/// 2015-05-07
/// Peso

#include "PesoProperty.h"
#include "SideScroller.h"

PesoProperty::PesoProperty(Entity * owner)
: EntityProperty("PesoProp", EP_PESO, owner)
{
	value = 1;
	sleeping = false;
};
void PesoProperty::OnCollisionCallback(CollisionCallback * cc)
{
	if (sleeping)
		return;
	sleeping = true;
	munny += value;
	sideScroller->UpdateMunny();
	// Remove from physics and graphics?
	QueuePhysics(new PMUnregisterEntity(owner));
	QueueGraphics(new GMUnregisterEntity(owner));
}
