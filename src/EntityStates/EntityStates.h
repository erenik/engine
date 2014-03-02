/// Emil Hedemalm
/// 2013-02-08

#ifndef ENTITY_STATES_H
#define ENTITY_STATES_H

// Defines IDs of all global entity states, which further define what behaviour type is in use of the particular entity.
namespace EntityStateType{
enum EntityStates{
	NULL_STATE,
	RACING_SHIP,
	CHARACTER,
	LOCATION,
	TILE_2D,

};
};

#endif