
#include "EntityManager.h"
#include "Model/Model.h"
#include "Texture.h"
#include "Graphics/GraphicsManager.h"
extern GraphicsManager graphics;

int EntityManager::idCounter = 1;

EntityManager * EntityManager::entityManager = NULL;

void EntityManager::Allocate()
{
	assert(entityManager == NULL);
	entityManager = new EntityManager();
}
EntityManager * EntityManager::Instance(){
	assert(entityManager);
	return entityManager;
}
void EntityManager::Deallocate()
{
	assert(entityManager);
	delete entityManager;
	entityManager = NULL;
}

EntityManager::EntityManager(){

};

EntityManager::~EntityManager()
{
	// Delete 'em! o.o
	entities.ClearAndDelete();
};

/// Creates entity using specified model and base texture
Entity * EntityManager::CreateEntity(String name, Model * model, Texture * texture)
{
	Entity * newEntity = NULL;
	/// Check for model
/*	if (model == NULL || (model && !model->Name())){
		std::cout<<"\nNo valid model was supplied. Aborting creation. ";
		return NULL;
	}
	*/

	// Check for texture
/*	if (texture->name == NULL){
		texture = Model::GetDefaultTexture();
		if (!texture || texture->name == NULL){
			std::cout<<"\nNo valid texture supplied and default is NULL. Aborting creation. ";
			return NULL;
		}
	}
	*/
	// Get a spot om the entity list
	for (int i = 0; i < entities.Size(); ++i)
	{
		if (entities[i]->id == 0)
		{
			newEntity = entities[i];
			break;
		}
	}
	// Create it if needed.
	if (!newEntity)
	{
		newEntity = new Entity(idCounter++);
		entities.Add(newEntity);
	}
	if (newEntity == NULL)
	{
		std::cout<<"\nWARNING: Could not create new entity! Array is full. Prompting deletion of unused entities.";
		int result = DeleteUnusedEntities();
		if (result){
			std::cout<<"\n"<<result<<" entities successfully freed/deleted! Attempting to re-add entity again.";
			return CreateEntity(name, model, texture);
		}
		return NULL;
	}
	// Copy over name since none was supplied...
	newEntity->SetName(name);
	newEntity->SetModel(model);
	newEntity->SetTexture(DIFFUSE_MAP | SPECULAR_MAP, texture);

	return newEntity;
}

bool EntityManager::DeleteEntity(Entity * entity)
{
	if (entity == NULL){
		std::cout<<"\nERROR: Null entity";
		return false;
	}
	entity->SetModel(NULL);
	entity->SetTexture(0xFFFFFFFF, NULL);
	entity->id = 0;
	return true;
}

void EntityManager::MarkEntitiesForDeletion(List<Entity*> entitiesToMark)
{
	Time now = Time::Now();
	int64 ms = now.Milliseconds();
	for (int i = 0; i < entitiesToMark.Size(); ++i)
	{
		Entity * entity = entitiesToMark[i];
		if (!entity->flaggedForDeletion)
		{
			entity->flaggedForDeletion = true;
			entity->deletionTimeMs = ms + 3000;
		}
	}
	entitiesToDelete.Add(entitiesToMark);
}



/** Deletes (resets IDs) of all entities that have been flagged for deletion and are not registered anywhere still. */
int EntityManager::DeleteUnusedEntities()
{
	if (!entitiesToDelete.Size())
		return 0;
	int64 nowMs = Time::Now().Milliseconds();
	assert(nowMs >= 0);
	int deletedEntities = 0;
	int duplicates = entitiesToDelete.Duplicates();

	for (int i = 0; i < entitiesToDelete.Size(); ++i)
	{
		Entity * entity = entitiesToDelete[i];
		/// Only process those that have been flagged.
		if (!entity->flaggedForDeletion)
			continue;
		if (entity->deletionTimeMs > nowMs)
			continue;
		/// Check that it isn't still registered with any other manager!
		if (entity->registeredForRendering)
		{
			std::cout<<"\nWARNING: Entity: "<<entity->name<<" registered for rendering while flagged for deletion!";
			continue;
		}
		if (entity->registeredForPhysics)
		{
			std::cout<<"\nWARNING: Entity: "<<entity->name<<" registered for physics while flagged for deletion!";
			continue;
		}
		/// Increment amount that was successfully deleted.
		++deletedEntities;
		while(entities.Remove(entity));
		while(entitiesToDelete.Remove(entity));
		delete entity;
		--i;
	}
	return deletedEntities;
}
