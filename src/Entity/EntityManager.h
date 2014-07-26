

#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

/// Maximum amount of objects in the scene
const int MAX_ENTITIES = 5000;

#include "Entity.h"
#include "../Selection.h"

#define EntityMan	(*EntityManager::Instance())

/** A manager for all in-game elements, aptly named "Entities" in this engine as in many others.
	Not to be confused with the ModelManager that handles the loading 
*/
class EntityManager{
	friend class MapManager;
private:
	EntityManager();
	static EntityManager * entityManager;
public:
	static void Allocate();
	static EntityManager * Instance();
	static void Deallocate();
	~EntityManager();
	
	/** Creates an entity using specified model and base texture.
		Should only be callable by other managers.
	*/
	Entity * CreateEntity(String withName, Model * model, Texture * andTexture);
	/** Deletes target entity. 
		Should only be callable by other managers.
	*/
	bool DeleteEntity(Entity * entity);

private:
	/** Deletes (resets IDs) of all entities that have been flagged for deletion and are not registered anywhere still. */
	int DeleteUnusedEntities();

	/// Counter for generating IDs to entities
	static int idCounter;
	/// Array of loaded entities.
	Entity entity[MAX_ENTITIES];
};


#endif