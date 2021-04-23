/// Emil Hedemalm
/// 2015-05-21

#include "EntityManager.h"
#include "Model/Model.h"
#include "Texture.h"
#include "Graphics/GraphicsManager.h"
#include "File/LogFile.h"
#include "Physics/PhysicsManager.h"
#include "StateManager.h"

#include "EntityProperty.h"

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
	entities.Clear();
};

bool EntityManager::IsGood(Entity* entity)
{
	return entities.Exists(entity);
}

List<std::pair<String, int>> nameOccurances;

/// Creates entity using specified model and base texture
Entity* EntityManager::CreateEntity(String name, Model * model, Texture * texture)
{
	bool found = false;
	int occurances = 0;
	for (int i = 0; i < nameOccurances.Size(); ++i)
	{
		if (nameOccurances[i].first == name)
		{
			occurances = ++nameOccurances[i].second;
			found = true;
			break;
		}
	}
	if (!found)
		nameOccurances.AddItem(std::pair<String, int>(name, 1));
	if (occurances > 1)
		name = name +"_"+ String(occurances);

	Entity* newEntity = NULL;

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
		newEntity->selfPtr = newEntity; // Assign own weak ptr based on the new shared ptr.
		entities.AddItem(newEntity);
	}
	if (newEntity == NULL)
	{
		std::cout<<"\nWARNING: Could not create new entity! Array is full. Prompting deletion of unused entities.";
		int result = DeleteUnusedEntities(50000); // should delete all
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

bool EntityManager::DeleteEntity(Entity* entity)
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

/// All active ones not already flagged for deletion.
List< Entity* > EntityManager::AllEntities()
{
	return entities;
}

void EntityManager::MarkEntitiesForDeletion(List<Entity*> entitiesToMark)
{
	LogMain("EntityManager::MarkEntitiesForDelection: "+String(entitiesToMark.Size()), DEBUG);
	Time now = Time::Now();
	int64 ms = now.Milliseconds();
	for (int i = 0; i < entitiesToMark.Size(); ++i)
	{
		Entity* entity = entitiesToMark[i];
		if (!entity->flaggedForDeletion)
		{
			entity->flaggedForDeletion = true;
			entity->deletionTimeMs = 3000;
			entitiesToDelete.AddItem(entity);
//			assert(entitiesToDelete.Duplicates() == 0);
		}
	}
}



/** Deletes (resets IDs) of all entities that have been flagged for deletion and are not registered anywhere still. */
int EntityManager::DeleteUnusedEntities(int timeInMs)
{
	if (!entitiesToDelete.Size())
		return 0;
	int64 nowMs = Time::Now().Milliseconds();
	if (nowMs < 0)
	{
		LogMain("NowMs: "+String(nowMs), DEBUG);
		assert(nowMs >= 0);
	}
	int deletedEntities = 0;
	assert(entitiesToDelete.Duplicates() == 0);

	for (int i = 0; i < entitiesToDelete.Size(); ++i)
	{
		Entity* entity = entitiesToDelete[i];
		entity->deletionTimeMs -= timeInMs;
		/// Only process those that have been flagged.
		if (!entity->flaggedForDeletion)
			continue;
		if (entity->deletionTimeMs > 0)
			continue;
		/// Check that it isn't still registered with any other manager!
		if (entity->deletionTimeMs < -3000) {
			if (entity->registeredForRendering)
			{
				std::cout << "\nWARNING: Entity: " << entity->name << " registered for rendering while flagged for deletion!";
				continue;
			}
			if (entity->registeredForPhysics)
			{
				std::cout << "\nWARNING: Entity: " << entity->name << " registered for physics while flagged for deletion!";
				QueuePhysics(new PMUnregisterEntity(entity));
				continue;
			}
		}
		// Delete all extra properties attached to it now.
		entity->properties.ClearAndDelete();

		/// Increment amount that was successfully deleted.
		++deletedEntities;
		entities.RemoveItemUnsorted(entity);
		entitiesToDelete.RemoveItemUnsorted(entity);
		delete entity; // Assume it deletes auto-magically from de-referencing?
		--i;
	}
	return deletedEntities;
}
