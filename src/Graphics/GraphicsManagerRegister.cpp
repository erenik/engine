/// Emil Hedemalm
/// 2013-03-01

#include "Graphics/GraphicsManager.h"
#include "Graphics/GraphicsProperty.h"
#include "Texture.h"
#include "TextureManager.h"

/// Adds an Entity to be rendered to the vfcOctree.
bool GraphicsManager::RegisterEntity(Entity * entity){
	/// Check that the entity isn't already registered
	if (entity->registeredForRendering){
		std::cout<<"\nNote: Entity already registered, re-registering";
		UnregisterEntity(entity);
	}
	registeredEntities.Add(entity);
	vfcOctree->AddEntity(entity);
	entity->registeredForRendering = true;
	// Check for additional graphics-data
	GraphicsProperty * graphics = entity->graphics;
	if (graphics){
		// Check for attached dynamic lights
		if (graphics->dynamicLights){
			for (int i = 0; i < graphics->dynamicLights->Size(); ++i){
				Graphics.dynamicLights.Add((Light*)(*graphics->dynamicLights)[i]);
			}
		}
		if (graphics->particleSystems){
			for (int i = 0; i < graphics->particleSystems->Size(); ++i){
				ParticleSystem * ps = (*graphics->particleSystems)[i];
				if (!Graphics.particleSystems.Exists(ps))
					Graphics.particleSystems.Add(ps);
			}
		}
	}
	// Buffer textures and meshes if needed
	Texture * t = entity->GetTexture(DIFFUSE_MAP);
	if (t)
		TexMan.BufferizeTexture(t);
	return true;
};

/// Registers all entities in the selection for rendering. Returns the number of faield registrations.
int GraphicsManager::RegisterEntities(List<Entity*> & toRegister){
	int failed = 0;
	for (int i = 0; i < toRegister.Size(); ++i){
		if (!RegisterEntity(toRegister[i]))
			++failed;
	}
	return failed;
}

/// Removes an Entity from the rendering vfcOctree.
bool GraphicsManager::UnregisterEntity(Entity * entity){
	int entitiesBefore = registeredEntities.Size();
	int octreeEntitiesBeforeRemoval = vfcOctree->RegisteredEntities();
	if (!entity->registeredForRendering)
		return true;
	assert(entitiesBefore == octreeEntitiesBeforeRemoval);
	bool result = registeredEntities.Remove(entity);
	if (!result){
		std::cout<<"\nWARNING: Unable to remove entity, already unregistered?";
	}
	assert(vfcOctree->RemoveEntity(entity));
	entity->registeredForRendering = false;

	// Check for additional graphics-data
	if (entity->graphics){
		GraphicsProperty * gp = entity->graphics;
		// Check for attached dynamic lights
		if (entity->graphics->dynamicLights){
			for (int i = 0; i < entity->graphics->dynamicLights->Size(); ++i){
				Graphics.dynamicLights.Remove((Light*)((*entity->graphics->dynamicLights)[i]));
			}
		}
		if (gp->particleSystems){
			for (int i = 0; i < gp->particleSystems->Size(); ++i){
				particleSystems.Remove((*gp->particleSystems)[i]);
			}
		}
	}

	int entitesAfter = registeredEntities.Size();
	int octreeEntities = vfcOctree->RegisteredEntities();
	assert(octreeEntities < octreeEntitiesBeforeRemoval);
	assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	return true;
};

/// Unregisters all entities in the selection from rendering. Returns the number of failed unregistrations.
int GraphicsManager::UnregisterEntities(List<Entity*> & toUnregister){
	int failed = 0;
	for (int i = 0; i < toUnregister.Size(); ++i){
		if (!UnregisterEntity(toUnregister[i]))
			++failed;
	}
	assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	return failed;
}

/// Unregisters all entities possible from rendering.
int GraphicsManager::UnregisterAll(){
	while(registeredEntities.Size() > 0){
		UnregisterEntity(registeredEntities[0]);
	}
	registeredEntities.Clear();
	assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	return 0;
}
