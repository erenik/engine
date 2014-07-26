/// Emil Hedemalm
/// 2014-07-25
/// Player properties for a space-shooter game.
/// Applicable to both human and "enemy"/AI-players.

#include "SpaceShooterPlayerProperty.h"

#include "Entity/Entity.h"

SpaceShooterPlayerProperty::SpaceShooterPlayerProperty(Entity * owner)
	: EntityProperty("SpaceShooterPlayerProperty", owner)
{


}


/// Time passed in seconds..!
void SpaceShooterPlayerProperty::Process(int timeInMs)
{
	// Get look-at direction.
	Vector4f minusZ(0,0,-1,0);
	Vector4f lookAt = owner->rotationMatrix.Product(minusZ);
	std::cout<<"\nLook at: "<<lookAt;
}
