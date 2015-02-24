/// Emil Hedemalm
/// 2014-07-30
/// Enum of the IDs of the entity properties used in the game

#ifndef TIFS_PROPERTIES_H
#define TIFS_PROPERTIES_H

#include "Entity/EntityProperty.h"

namespace TIFSProperty {
	enum {
		TURRET = EntityPropertyID::CUSTOM_GAME_1,
		DRONE,
		PLAYER,
		MOTHERSHIP,
	};
};

#endif
