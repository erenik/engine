/// Emil Hedemalm
/// 2015-01-17
/// Rendering the world as a whole.

#include "WorldMap.h"
#include "Zone.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "Model/Model.h"
#include "TextureManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Messages/GMCamera.h"

#include "Color.h"

WorldMap worldMap;

WorldMap::WorldMap()
{
	worldEntity = NULL;
	oceanEntity = NULL;
	worldMapCamera = NULL;
}

void WorldMap::UpdateOcean()
{
	if (!oceanEntity)
	{
		oceanEntity = MapMan.CreateEntity("OceanEntity", ModelMan.GetModel("plane.obj"), TexMan.GetTextureByHex32(0x0000FF77));
		// Set it to render last..?
		GraphicsMan.QueueMessage(new GMSetEntityb(oceanEntity, GT_REQUIRE_DEPTH_SORTING, true));
	}
	// Set height and position.
	Vector3f position = world.size;
	position /= 2.f;
	position.z = position.y;
	position.y = world.oceanElevation;
	// Update color
	GraphicsMan.QueueMessage(new GMSetEntityTexture(oceanEntity, DIFFUSE_MAP | SPECULAR_MAP, TexMan.GetTextureByColor(world.oceanColor)));
	PhysicsMan.QueueMessage(new PMSetEntity(oceanEntity, PT_SET_POSITION, position)); 
	// Set scale
	PhysicsMan.QueueMessage(new PMSetEntity(oceanEntity, PT_SET_SCALE, Vector3f(world.size.x, 1, world.size.y))); 
}

void WorldMap::Update()
{
	Texture * tex = world.GeneratePreviewTexture();
	tex->Save("World.png", true);
	Model * model = world.GenerateWorldModel();
	if (!worldEntity)
		worldEntity = MapMan.CreateEntity("World", ModelMan.GetModel("plane.obj"), tex);
	assert(worldEntity);
	// Re-bufferize the texture.
	GraphicsMan.QueueMessage(new GMBufferTexture(tex));
	// Same for the model..
	if (model)
	{
		GraphicsMan.QueueMessage(new GMBufferMesh(model->GetTriangulatedMesh()));
		GraphicsMan.QueueMessage(new GMSetEntity(worldEntity, GT_MODEL, model));
		// Try our model..
		GraphicsMan.QueueMessage(new GMSetEntityTexture(worldEntity, DIFFUSE_MAP | SPECULAR_MAP, tex));
	}
//	else 
		
//		Physics.QueueMessage(new PMSetEntity(worldMapEntity, PT_SET_SCALE, Vector3f(15.f, 1.f, 15.f)));
	// Place the ocean..!
//	UpdateOcean();
	UpdateCamera();
	CenterCamera();
}

// Updates the settlement representations, usually in the form of some building or a crest and text.
void WorldMap::UpdateSettlements()
{
	MapMan.DeleteEntities(settlementEntities);
	settlementEntities.Clear();
	for (int i = 0; i < world.settlements.Size(); ++i)
	{
		Zone * settlement = world.settlements[i];
		// Create appropriate representation entities.
		Entity * entity = settlement->CreateWorldMapRepresentation();
		if (!entity)
			continue;
		settlementEntities.Add(entity);
		Vector3f pos = settlement->position;
		// Invert Y, as it got.. not sure.
		pos.z = world.size.y - pos.y;
		pos.y = settlement->elevation;
		entity->worldPosition = pos;
		entity->scale = Vector3f(1,1,1) * 0.1f;
		entity->RecalculateMatrix();
	}
	MapMan.AddEntities(settlementEntities);
}


// o.o
void WorldMap::UpdateCamera()
{
	if (!worldMapCamera)
	{
		worldMapCamera = CameraMan.NewCamera("WorldMapCamera", true);
		worldMapCamera->movementType = CAMERA_MOVEMENT_ABSOLUTE;
		worldMapCamera->absForward = Vector3f(0,0,-1);
		worldMapCamera->absRight = Vector3f(1,0,0);
		worldMapCamera->absUp = Vector3f(0,1,0);
		CenterCamera();

		// Create the default camera-position too.
		Camera * worldMapResetCamera = CameraMan.NewCamera("WorldMapResetCamera");
		worldMapCamera->resetCamera = worldMapResetCamera;
	}
	// If the world changed much, adjust position, if not, don't really touch it?
	// Update the reset-camera's position and stuffs! o.o
	Vector3f position = FromWorldToWorldMap(world.size * 0.5f, 0.f);
	worldMapCamera->resetCamera->position = position + Vector3f(0, 20.f, 20.f);	 
	worldMapCamera->resetCamera->rotation = Vector3f(0.9f, 0, 0);
	worldMapCamera->resetCamera->flySpeed = world.size.Length() * 0.1f;
}

// Centers it so that the whole world is visible.
void WorldMap::CenterCamera()
{
	 if (!worldMapCamera)
		 UpdateCamera();	
	 // Just reset it to center it.
	 worldMapCamera->Reset();
}

// Registers all entities for display and makes the world-map camera active.
void WorldMap::MakeActive()
{
	// Register all entities for display?
	GraphicsMan.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
	// Make camera active. Create it if needed?
	CenterCamera();
	GraphicsMan.QueueMessage(new GMSetCamera(worldMapCamera));
	GraphicsMan.QueueMessage(new GMRegisterEntities(Entities()));
}

/// o.o
List<Entity*> WorldMap::Entities()
{
	List<Entity*> entities = settlementEntities;
	if (worldEntity) entities.Add(worldEntity);
	if (oceanEntity) entities.Add(oceanEntity);
	return entities;
}


Vector3f FromWorldToWorldMap(Vector2i vec, float elevation)
{
	return Vector3f(vec.x, elevation, world.size.y - vec.y);
}

Vector3f FromWorldToWorldMap(Vector3i pos)
{
	return Vector3f(pos.x, pos.z, pos.y);
}

