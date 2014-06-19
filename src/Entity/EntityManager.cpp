
#include "EntityManager.h"
#include "../Model.h"
#include "Texture.h"
#include "Graphics/GraphicsManager.h"
extern GraphicsManager graphics;

int EntityManager::idCounter = 1;

EntityManager * EntityManager::entityManager = NULL;

void EntityManager::Allocate(){
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

EntityManager::~EntityManager(){

};

/// Creates entity using specified model and base texture
Entity * EntityManager::CreateEntity(Model * model, Texture * texture){
	Entity * newEntity = NULL;
	/// Check for model
	if (model == NULL || (model && !model->Name())){
		std::cout<<"\nNo valid model was supplied. Aborting creation. ";
		return NULL;
	}
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
	for (int i = 0; i < MAX_ENTITIES; ++i){
		if (entity[i].id == 0){
			newEntity = &entity[i];
			break;
		}
	}
	if (newEntity == NULL){
		std::cout<<"\nWARNING: Could not create new entity! Array is full. Prompting deletion of unused entities.";
		int result = DeleteUnusedEntities();
		if (result){
			std::cout<<"\n"<<result<<" entities successfully freed/deleted! Attempting to re-add entity again.";
			return CreateEntity(model, texture);
		}
		return NULL;
	}
	// Copy over name since none was supplied...
	newEntity->SetName(model->Name());
	newEntity->SetModel(model);
	newEntity->radius = model->radius;
	newEntity->SetTexture(DIFFUSE_MAP | SPECULAR_MAP, texture);
	newEntity->id = this->idCounter++;

	return newEntity;
}

bool EntityManager::DeleteEntity(Entity * entity){
	if (entity == NULL){
		std::cout<<"\nERROR: Null entity";
		return false;
	}
	entity->SetModel(NULL);
	entity->SetTexture(0xFFFFFFFF, NULL);
	entity->id = 0;
	return true;
}


/** Deletes (resets IDs) of all entities that have been flagged for deletion and are not registered anywhere still. */
int EntityManager::DeleteUnusedEntities(){
	int deletedEntities = 0;
	for (int i = 0; i < MAX_ENTITIES; ++i){
		/// Only process those that have been flagged.
		if (!entity[i].flaggedForDeletion)
			continue;
		/// Check that it isn't still registered with any other manager!
		if (entity[i].registeredForRendering){
			std::cout<<"\nWARNING: Entity: "<<entity[i].name<<" registered for rendering while flagged for deletion!";
			continue;
		}
		if (entity[i].registeredForPhysics){
			std::cout<<"\nWARNING: Entity: "<<entity[i].name<<" registered for physics while flagged for deletion!";
			continue;
		}

		/// Reset ID.
		entity[i].id = 0;
		entity[i].name = "";

		/// De-flag after deletion is finished.
		entity[i].flaggedForDeletion = false;
		/// Increment amount that was successfully deleted.
		++deletedEntities;
	}
	return deletedEntities;
}
