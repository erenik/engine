/// Emil Hedemalm
/// 2013-02-08

#ifndef WORLD_H
#define WORLD_H

#include "../Entity/Entity.h"
#include "Locations/Location.h"
#include "Locations/Locations.h"
#include "AIMessage.h"

#define AIWorld (*World::Instance())

/// The "World" class is basically a manager for a set of AI-interactable entities.
class World {
	/// Default null-constructor
	World();
	static World world;
public:
	static World * Instance() { return &world; };
	virtual ~World();
	
	/// Posts a message to the world, re-directed depending on message-type.
	virtual void Message(AIMessage * message);

	/// Fills the world with a default setup.
	void CreateDefaultSetup();

	/// Re-places all locations to valid waypoints and binds all characters current positions to the closest valid points in the world.
	void MapToNavMesh();
private:
	List<Entity*> aiEntities;
};

#endif