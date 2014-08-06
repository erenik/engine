/// Emil Hedemalm
/// 2014-08-06
/// Entity property for player controllability and camera control.

#include "TIFSPlayerProperty.h"
#include "TIFSProperties.h"

TIFSPlayerProperty::TIFSPlayerProperty(Entity * owner)
: FirstPersonPlayerProperty("TIFSPlayerProperty", TIFSProperty::PLAYER, owner)
{

}
