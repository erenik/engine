/// Emil Hedemalm
/// 2013-03-01

#include "Graphics/GraphicsManager.h"
#include "Graphics/GraphicsProperty.h"
#include "Texture.h"
#include "TextureManager.h"
#include "Particles/ParticleSystem.h"
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"
#include "Entity/EntityProperty.h"

/// Adds an Entity to be rendered to the vfcOctree.
bool GraphicsManager::RegisterEntity(Entity * entity)
{
	/// Already registered, returning.
	if (entity->registeredForRendering)
		return true;
	/// Check that the entity isn't already registered
	if (entity->registeredForRendering){
		std::cout<<"\nNote: Entity already registered, re-registering";
		UnregisterEntity(entity);
	}
	registeredEntities.AddItem(entity);
#ifdef VFC_OCTREE
	if (optimizationStructure == VFC_OCTREE)
		vfcOctree->AddEntity(entity);
#endif
	entity->registeredForRendering = true;

	/// Bufferize models and textures?
	entity->Bufferize();

	// Check for additional graphics-data
	GraphicsProperty * gp = entity->graphics;
	if (gp)
	{
		// Check for attached graphical properties.
		for (int i = 0; i < gp->particleSystems.Size(); ++i)
		{
			ParticleSystem * ps = gp->particleSystems[i];
			RegisterParticleSystem(ps, false);
		}
	}
	/// If no graphics property exists. Add it.
	else 
	{
		gp = entity->graphics = new GraphicsProperty(entity);
	}

	/// Add it to its proper render groups.
	graphicsState->AddEntity(entity);

	// Buffer textures and meshes if needed
	List<Texture*> textures = entity->GetTextures(0xFFFFFFF);
	TexMan.BufferizeTextures(textures);
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
bool GraphicsManager::UnregisterEntity(Entity * entity)
{
	bool result = registeredEntities.RemoveItemUnsorted(entity);
	if (!result){
//		std::cout<<"\nWARNING: Unable to remove entity, already unregistered?";
		return false;
	}
	int entitiesBefore = registeredEntities.Size();
#ifdef VFC_OCTREE
	int octreeEntitiesBeforeRemoval = vfcOctree->RegisteredEntities();
#endif
	if (!entity->registeredForRendering)
		return true;
	
	// Unbind camera as needed.
	if (entity->cameraFocus && 	entity->cameraFocus->entityToTrack == entity)
	{
		entity->cameraFocus->entityToTrack = NULL;
	}

	for (int i = 0; i < entity->properties.Size(); ++i)
	{
		EntityProperty * prop = entity->properties[i];
		prop->OnUnregistrationFromGraphics();
	}

	// Check for additional graphics-data
	if (entity->graphics)
	{
		GraphicsProperty * gp = entity->graphics;
		
		// Check for attached dynamic lights
		for (int i = 0; i < gp->dynamicLights.Size(); ++i)
		{
			Light * light = gp->dynamicLights[i];
			List<Light*> & dynamicLights = graphicsState->dynamicLights;
			bool removed = dynamicLights.RemoveItemUnsorted(light);
			assert(removed);
		}
		List<ParticleSystem*> & entityParticleSystems = gp->particleSystems;
		for (int i = 0; i < entityParticleSystems.Size(); ++i)
		{
			ParticleSystem * ps = entityParticleSystems[i];
			bool succeeded = particleSystems.RemoveItemUnsorted(ps);
		}
	}

	/// Add it to its proper render groups.
	graphicsState->RemoveEntity(entity);

	int entitesAfter = registeredEntities.Size();
	// Remove from optimization structures, if any.
#ifdef VFC_OCTREE
	if (optimizationStructure == VFC_OCTREE)
	{
		assert(entitiesBefore == octreeEntitiesBeforeRemoval);
		assert(vfcOctree->RemoveEntity(entity));
		int octreeEntities = vfcOctree->RegisteredEntities();
		assert(octreeEntities < octreeEntitiesBeforeRemoval);
		assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	}
#endif
	entity->registeredForRendering = false;
	return true;
};

/// Unregisters all entities in the selection from rendering. Returns the number of failed unregistrations.
int GraphicsManager::UnregisterEntities(List<Entity*> & toUnregister)
{
	int failed = 0;
	for (int i = 0; i < toUnregister.Size(); ++i){
		if (!UnregisterEntity(toUnregister[i]))
			++failed;
	}
//	assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	return failed;
}

/// Unregisters all entities possible from rendering.
int GraphicsManager::UnregisterAll()
{
	while(registeredEntities.Size() > 0){
		UnregisterEntity(registeredEntities[0]);
	}
	registeredEntities.Clear();
//	assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	return 0;
}


bool GraphicsManager::RegisterParticleSystem(ParticleSystem * ps, bool global)
{
	if (!particleSystems.Exists(ps))
		particleSystems.Add(ps);
	if (global)
		if (!globalParticleSystems.Exists(ps))
			globalParticleSystems.Add(ps);
	ps->registeredForRendering = true;
	return true;
}

bool GraphicsManager::UnregisterParticleSystem(ParticleSystem * ps)
{
	assert(ps);
	if (!ps)
		return false;
	particleSystems.Remove(ps);
	globalParticleSystems.Remove(ps);
	ps->registeredForRendering = false;
	return true;
}
