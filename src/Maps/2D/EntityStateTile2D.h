/// Emil Hedemalm
/// 2014-01-20
/// EntityPropertyState used for TileMap2D interaction.

#ifndef ENTITY_STATE_TILE_2D_H
#define ENTITY_STATE_TILE_2D_H

//#include "Maps/Grids/Tile.h"
#include "MathLib.h"
#include "Entity/EntityProperty.h"

/// Base Entity-state for enabling 2D map interaction. Is this class even needed..?
class EntityStateTile2D : public EntityProperty
{
public:
	EntityStateTile2D(Entity * entity);
	~EntityStateTile2D();
	/// Render, why does the entity state have a render function..?
	void Render();

	/// Returns the ID of this specific property-type (used when identifying it within an entity later on).
	virtual int ID();

	

	virtual void OnEnter();
	virtual void Process(int timeInMs);
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

	/// 2D position.
	Vector3i position;
	/// Tile it's currently.. standing on?
//	Tile * tile;
};

#endif