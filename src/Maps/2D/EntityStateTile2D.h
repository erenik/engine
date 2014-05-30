/// Emil Hedemalm
/// 2014-01-20
/// EntityState used for TileMap2D interaction.

#ifndef ENTITY_STATE_TILE_2D_H
#define ENTITY_STATE_TILE_2D_H

#include "Maps/Grids/Tile.h"
#include "EntityStates/EntityState.h"

/// Base Entity-state for enabling 2D map interaction.
class EntityStateTile2D : public EntityState {
public:
	EntityStateTile2D(Entity * entity);
	/// Render
	void Render();

	virtual void OnEnter();
	virtual void Process(float time);
	virtual void OnExit();
	virtual void ProcessMessage(Message * message);

	/// Used for specifying subtype of this state, defined in your own game preferably.
	int subType;
	enum types {
		NULL_TYPE,
		DEFAULT,
		USER_DEFINED,
		MAX_TYPES = USER_DEFINED + 100,
	};

	/// Entity owner of this state.
	Entity * entity;
	/// 2D position.
	Vector3i position;
	/// Tile it's currently.. standing on?
	Tile * tile;
};

#endif