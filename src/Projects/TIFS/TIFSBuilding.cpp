/// Emil Hedemalm
/// 2015-02-19
/// Class for handling the buildings in the game.

#include "TIFS.h"
#include "TIFSBuilding.h"

#include "Entity/EntityManager.h"

List<TIFSBuilding*> TIFSBuilding::types;
Random TIFSBuilding::buildingRandom;

TIFSBuilding::TIFSBuilding()
{
	Nullify();
}
/// Creates a building given a base name, and sets base, top, and texture sources based on it, and appending _*.ext 
TIFSBuilding::TIFSBuilding(String baseName)
{
	Nullify();
	name = baseName;
#define PREFIX "img/Buildings/"
	diffuseSource = PREFIX+baseName+"_diffuse.png";
	specularSource = PREFIX+baseName+"_specular.png";
	normalSource = PREFIX+baseName+"_normal.png";
	diffuseSource = PREFIX+baseName+"_diffuse";
#define PREFIX_M "obj/Buildings/"
	bottomSource = PREFIX_M+baseName+"_bottom.obj";
	topSource = PREFIX_M+baseName+"_top.obj";
}

void TIFSBuilding::Nullify()
{
	floorHeight = 2.5f;
	scale = Vector3f(1,1,1) * 2.5f;
}

/// Creates it.
List<Entity*> TIFSBuilding::CreateNew(ConstVec3fr atLocation, ConstVec3fr withGivenMaxSize)
{
	List<Entity*> entities;
	Vector3f position = atLocation;
	/// All created and done?
	Vector3f maxSize = withGivenMaxSize;
	float sizeSquared = maxSize.LengthSquared();
	// Randomize some bonus height!
	if (maxSize.y == 0)
		maxSize.y = buildingRandom.Randf(sizeSquared * 0.05f) + sizeSquared * 0.01f + 5.f;
	
	// Fetch fitting type.
	Vector3f rotationNeeded;
	TIFSBuilding * type = GetType(maxSize, rotationNeeded);
	if (!type)
		return entities;
	float floorHeight = type->floorHeight;

	// Fetch floors, minimum 1 + roof 
	int floors = (int) (maxSize.y / type->floorHeight);
	if (floors < 2)
		floors = 2;

	// Try out the iterative approach of 1 level at a time. o.o
	List<Entity*> buildingFloors;
	for (int j = 0; j < floors; ++j)
	{
		Model * model;
		if (j == floors - 1)
			model = type->topModel;
		else
			model = type->bottomModel;
//			assert(model);
		Entity * entity = EntityMan.CreateEntity(type->name+" Floor "+String(j), model, type->diffuseMap);
		entity->normalMap = type->normalMap;
		entity->specularMap = type->specularMap;
		// Create da building blocksuuu.
		entity->position = position;
		// Increase Y per floor.
		entity->position.y += j * type->floorHeight;
		entity->rotation = rotationNeeded;
		// Scale it?
		entity->Scale(type->scale);
		buildingFloors.Add(entity);
	}
	entities.Add(buildingFloors);
	// Register floors for rendering.
	GraphicsQueue.Add(new GMRegisterEntities(buildingFloors));
	
	// Create 2 physics entities, 1 for the body and 1 for the roof.
	Entity * physicsBase = EntityMan.CreateEntity(type->name+" base physics", type->bottomModel, type->diffuseMap);
	// Position it accordingly.
	physicsBase->position = position;
	// No addition to Y position if using mesh-based physics, as the mesh was starting at Y0 and going uprward Y+
//	physicsBase->position.y += floorHeight * (floors - 1) / 2.f;
	// Scale it.
	Vector3f floorVector = Vector3f(1, (float) floors - 1, 1);
	Vector3f basePhysicsScale = type->scale * floorVector;
	physicsBase->Scale(basePhysicsScale);
	// Add top 
	Entity * physicsRoof = EntityMan.CreateEntity(type->name+" roof physics", type->topModel, type->diffuseMap);
	physicsRoof->position = position;
	physicsRoof->position.y += floorHeight * (floors - 1);
	physicsRoof->Scale(type->scale);
	// Set up physics properties.
	List<Entity*> physicsEntities(physicsBase, physicsRoof);
	for (int i = 0; i < physicsEntities.Size(); ++i)
	{
		Entity * entity = physicsEntities[i];
		PhysicsProperty * prop = entity->physics = new PhysicsProperty();
		prop->type = PhysicsType::STATIC;
		prop->shapeType = ShapeType::MESH;
	}
	// Register for physics
	PhysicsQueue.Add(new PMRegisterEntities(physicsEntities));
	if (debug == 10)
	{
		std::cout<<"\nRegistering physics entity for rendering";
		std::cout<<"\nScale: "<<type->scale<<" physicsBase scale: "<<physicsBase->scale;
		std::cout<<"\nPosition: "<<position<<" physicsBase position: "<<physicsBase->position;
		GraphicsQueue.Add(new GMRegisterEntities(List<Entity*>(physicsBase, physicsRoof)));
	}
	// Add it to be returned.
	entities.Add(physicsBase, physicsRoof);

	/*
	Model * bottomModel = NULL, * topModel = NULL;
	String bottomName, topName, diffuseName;
	int floors = 10;
	float floorHeight = 2.5f;
	Texture * diffuse = NULL;

	int type = i % 3;
	switch(type)
	{
	case 0:
		topName = "medium_building_top";
		bottomName = "medium_building_bottom";
		diffuseName = "medium_building_diffuse";
		break;
	case 1:
		topName = "small_building_top";
		bottomName = "small_building_bottom";
		diffuseName = "small_building_diffuse";
		break;
	case 2:
		topName = "big_building_top";
		bottomName = "big_building_bottom";
		diffuseName = "big_building_diffuse";
		floorHeight = 2.5f;
		break;
	default: assert(false);
	}

	/// Fetch amount of floors based on the height?
	floors = maxSize.y / floorHeight;
	if (floors == 0)
	{
		LogMain("0 floors, skipping.", INFO);
		continue;
	}
	maxSize.y = floors * floorHeight; // Recalculate maxSize based on the floors we used.

	/// Create "building" of random size based on the given maxSize :)
	Entity * buildingEntity = EntityMan.CreateEntity("Building "+String(i), ModelMan.GetModel("cube.obj"), TexMan.GetTexture("0x82"));
	PhysicsProperty * pp = new PhysicsProperty();
	buildingEntity->physics = pp;
	pp->shapeType = PhysicsShape::AABB;

	// Register for rendering.
	GraphicsQueue.Add(new GMRegisterEntities(buildingFloors));
	// Adjust Y based on update Y-scale.
	position.y = position.y + maxSize.y * 0.5; 
	buildingEntity->position = position;
	buildingEntity->SetScale(maxSize);
*/
	return entities;
}

bool TIFSBuilding::LoadTypes(String fromSource /*= "data/BuildingTypes.txt" */)
{
	List<String> lines = File::GetLines(fromSource);
	if (lines.Size() == 0)
		return false;
	// Clear old
	types.ClearAndDelete();
	// Parse new types
	TIFSBuilding * type = NULL;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		List<String> spaceTokens = line.Tokenize(" ");
		if (spaceTokens.Size() < 2)
			continue;
		if (line.StartsWith("Type"))
		{
			String name = spaceTokens[1];
			type = new TIFSBuilding(name);
			types.Add(type);
		}
		if (!type) 
			continue;
//		if (line.StartsWith("")
		//// o.o
//		string basesource, topsource, diffusesource, specularsource, normalsource;
////		vector3f scale;
//		float floorheight;
		/// aabb?
//		model * base, * top;
//		texture * diffuse, * specular, * normal;
	}
	/// Fetch resources.
	for (int i = 0; i < types.Size(); ++i)
	{
		TIFSBuilding * type = types[i];
		if (!type->FetchResources())
		{
			LogMain("Unable to fetch resoruces for building type: "+type->name, ERROR);
			types.RemoveItem(type);
			delete type;
			--i;
		};
	}
	return true;
}

TIFSBuilding * TIFSBuilding::GetType(ConstVec3fr forMaxSize, Vector3f & rotationNeeded)
{
	List<TIFSBuilding*> eligibleTypes;
	/// Fetch eligible types.
	for (int i = 0; i < types.Size(); ++i)
	{
		TIFSBuilding * type = types[i];
		/// Check size it would be in-game by multiplying it's AABB.
		const AABB & baseAABB = type->bottomModel->GetAABB();
		Vector3f inGameScale = baseAABB.scale * type->scale;
		/// Compare inGameScale scale. Is good?
		bool isGood = false;
		if (inGameScale < forMaxSize)
			isGood = true;
		// o.o
		if (isGood)
			eligibleTypes.Add(type);
	}
	if (eligibleTypes.Size() == 0)
		return NULL;
	/// Randomzie among eligible types.
	return eligibleTypes[buildingRandom.Randi(1000) % eligibleTypes.Size()];
}


bool TIFSBuilding::FetchResources()
{
	// Fetch actual resources
	topModel = ModelMan.GetModel(topSource);
	bottomModel = ModelMan.GetModel(bottomSource);
	diffuseMap = TexMan.GetTexture(diffuseSource);
	specularMap = TexMan.GetTexture(specularSource);
	normalMap = TexMan.GetTexture(normalSource);
	if (!topModel || !bottomModel || !diffuseMap)
	{
		LogMain("Unable to extract resources for building "+String(name), ERROR | CAUSE_ASSERTION_ERROR);
		return false;
	}
	return true;
}


