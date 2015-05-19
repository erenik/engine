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
	/// Play SFX
	List<String> coinSFX;
	coinSFX.Add("sfx/Coin 1.wav", "sfx/Coin 2.wav", "sfx/Coin 3.wav", "sfx/Coin 4.wav", "sfx/Coin 5.wav");
	QueueAudio(new AMPlaySFX(coinSFX[sfxRand.Randi(100) % coinSFX.Size()]));
	// Remove from physics and graphics?
	QueuePhysics(new PMUnregisterEntity(owner));
	QueueGraphics(new GMUnregisterEntity(owner));
}
