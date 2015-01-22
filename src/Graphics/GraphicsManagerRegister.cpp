/// Emil Hedemalm
/// 2013-03-01

#include "Graphics/GraphicsManager.h"
#include "Graphics/GraphicsProperty.h"
#include "Texture.h"
#include "TextureManager.h"
#include "Particles/ParticleSystem.h"
#include "GraphicsState.h"

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
	registeredEntities.Add(entity);
	if (optimizationStructure == VFC_OCTREE)
		vfcOctree->AddEntity(entity);
	entity->registeredForRendering = true;
	// Check for additional graphics-data
	GraphicsProperty * graphics = entity->graphics;
	if (graphics)
	{
		// Check for attached graphical properties.
		for (int i = 0; i < graphics->particleSystems.Size(); ++i)
		{
			ParticleSystem * ps = graphics->particleSystems[i];
			RegisterParticleSystem(ps, false);
		}
	}
	/// If no graphics property exists. Add it.
	else 
	{
		entity->graphics = new GraphicsProperty(entity);
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
bool GraphicsManager::UnregisterEntity(Entity * entity)
{
	int entitiesBefore = registeredEntities.Size();
	int octreeEntitiesBeforeRemoval = vfcOctree->RegisteredEntities();
	if (!entity->registeredForRendering)
		return true;
	entity->registeredForRendering = false;
	bool result = registeredEntities.Remove(entity);
	if (!result){
		std::cout<<"\nWARNING: Unable to remove entity, already unregistered?";
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
			bool removed = dynamicLights.Remove(light);
			assert(removed);
		}
		List<ParticleSystem*> & entityParticleSystems = gp->particleSystems;
		for (int i = 0; i < entityParticleSystems.Size(); ++i)
		{
			ParticleSystem * ps = entityParticleSystems[i];
			bool succeeded = particleSystems.Remove(ps);
		}
	}

	int entitesAfter = registeredEntities.Size();
	// Remove from optimization structures, if any.
	if (optimizationStructure == VFC_OCTREE)
	{
		assert(entitiesBefore == octreeEntitiesBeforeRemoval);
		assert(vfcOctree->RemoveEntity(entity));
		int octreeEntities = vfcOctree->RegisteredEntities();
		assert(octreeEntities < octreeEntitiesBeforeRemoval);
		assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
	}
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
int GraphicsManager::UnregisterAll()
{
	while(registeredEntities.Size() > 0){
		UnregisterEntity(registeredEntities[0]);
	}
	registeredEntities.Clear();
	assert(registeredEntities.Size() == vfcOctree->RegisteredEntities());
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
