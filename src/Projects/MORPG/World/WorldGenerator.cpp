/// Emil Hedemalm
/// 2014-07-27
/// Main class for generating a world. Sub-class and overload for custom worlds.

#include "WorldGenerator.h"
#include "ZoneGenerator.h"

#include "Zone.h"

#include "Model/ModelManager.h"
#include "TextureManager.h"

ZoneGenerator * zoneGenerator = NULL;

WorldGenerator::WorldGenerator()
{
	size = Vector3i(20, 5, 20);
	water = 0.2f;
	waterDepth = -1.f;
	waterLevel = 0;
	smoothing = 0;
	smoothingMultiplier = 0.1f;
	mountainHeight = 3.f;
	numSettlements = 3;
}

WorldGenerator::~WorldGenerator()
{
	SAFE_DELETE(zoneGenerator);
}

/// Generates a new world.
bool WorldGenerator::GenerateWorld(World & worldToBeGenerated, bool newRandomSeed)
{
	// Delete it first... geez.
	worldToBeGenerated.Delete();

	// Various options.
	if (!worldToBeGenerated.empty)
	{
		assert(false && "World not empty, clear it first before generating it again, yo.");
		return false;
	}
	waterOrigins.Clear();
	if (newRandomSeed)
	{
		randomSeed = Random();
	}
	this->world = &worldToBeGenerated;
	/// Allocate zone matrix
	world->zoneMatrix.SetDefaultValue(0);

	world->zoneMatrix.Allocate(size);
	/// Generate zones.
	for (int x = 0; x < size.x; ++x)
	{
		for (int z = 0; z < size.z; ++z)
		{
			Zone * zone = new Zone();
			Vector3i position = Vector3i(x,0,z);
			zone->position = position;
			zone->name = "X"+String(x)+" Z"+String(z);
			world->zones.Add(zone);
			assert(world->zoneMatrix.At(position) == 0);
			world->zoneMatrix.Set(position, zone);
		}
	}
	world->ReconnectZones();

	//	world.zoneMatrix.PrintContents();

	world->size = size;

//	world.zoneMatrix.PrintContents();
#define Print(s) std::cout<<s;
	Print("\nMarking water");
	MarkWater();
	Print("\nRaising water level");
	RaiseWaterLevel();
	Print("\nMarking mountains");
	MarkMountains();
	Print("\nSmoothing");
	Smooth();
	Print("\nRaw generation complete");
	EvaluateZoneTypes();
	PlaceSettlements();
	return true;
}

bool WorldGenerator::GenerateSettlements(World & worldToBeGenerated, bool newRandomSeed)
{
	if (newRandomSeed)
		randomSeed = Random();
	this->world = &worldToBeGenerated;
	world->ClearSettlementsAndCharacters();
	EvaluateZoneTypes();
	PlaceSettlements();
	return true;
}

/// Generates the zone and local population within target zone.
bool WorldGenerator::GenerateSettlement(Zone * inZone)
{
	// Generate zone contents first?
	if (inZone->rooms.Size() == 0)
	{
		if (!zoneGenerator)
			zoneGenerator = new ZoneGenerator();
		zoneGenerator->GenerateZone(inZone);
	}
	this->CreateCharacters(inZone);
	return true;
}

void WorldGenerator::MarkWater()
{
	/// Mark water
//	assert(water < 1.f);
	int numZones = world->zones.Size();
	int numWaterTiles = water * numZones;
	int waterTilesCreated = 0;

	Random waterRandom = randomSeed;

	int failedAttempts = 0;
	while (waterTilesCreated < numWaterTiles)
	{
		int randomTileIndex = waterRandom.Randi(numZones-1);
		Zone * zone = world->zones[randomTileIndex];
		if (zone->IsWater())
		{
			++failedAttempts;
			if (failedAttempts > 1000)
				break;
			continue;
		}
		zone->SetWater(true);
		zone->elevation = waterDepth;
		++waterTilesCreated;
		waterOrigins.Add(zone);
	}
}

void WorldGenerator::RaiseWaterLevel()
{
	// Raise the water-level..!
	for (int i = 0; i < waterLevel; ++i)
	{	
		List<Zone*> newWaterOrigins;
		for (int j = 0; j < waterOrigins.Size(); ++j)
		{
			Zone * zone = waterOrigins[j];
			List<Zone*> neighbours = zone->neighbours;
			for (int k = 0; k < neighbours.Size(); ++k)
			{
				Zone * neighbour = neighbours[k];
				if (!neighbour->IsWater())
				{
					newWaterOrigins.Add(neighbour);
					neighbour->SetWater(true);
					neighbour->elevation = waterDepth;
				}
			}
		}
		waterOrigins = newWaterOrigins;
	}
}

void WorldGenerator::MarkMountains()
{
	Random mountainRandom = randomSeed;
	Random mountainHeightRandom = randomSeed;

	int numZones = world->zones.Size();
	int numMountainTiles = this->mountains * numZones;
	int mountainTilesCreated = 0;

	int failedAttempts = 0;
	while (mountainTilesCreated < numMountainTiles)
	{
		int randomTileIndex = mountainRandom.Randi(numZones-1);
		Zone * zone = world->zones[randomTileIndex];
		if (zone->IsWater())
		{
			++failedAttempts;
			if (failedAttempts > 1000)
				break;
			continue;
		}
		zone->SetMountain(true);
		zone->elevation += mountainHeightRandom.Randf(mountainHeight);
		++mountainTilesCreated;
	}
}

void WorldGenerator::Smooth()
{
	List<Zone*> & zones = world->zones;
	// Apply smoothing iteratively.
	for (int i = 0; i < smoothing; ++i)
	{	
		List<float> elevations;
		for (int j = 0; j < zones.Size(); ++j)
		{
			Zone * zone = zones[j];
			List<Zone*> neighbours = zone->neighbours;
			// Cacl average elevation of neighbours.
			float aveEle = 0;
			for (int k = 0; k < neighbours.Size(); ++k)
			{
				Zone * neighbour = neighbours[k];
				aveEle += neighbour->elevation;
			}
			aveEle /= neighbours.Size();
			// Apply 50% smoothing factor each iteration?
			float newElevation = zone->elevation * (1 - smoothingMultiplier) + aveEle * smoothingMultiplier;
			elevations.Add(newElevation);
		}
		/// Apply new elevations.
		for (int i = 0; i < zones.Size(); ++i)
		{
			Zone * zone = zones[i];
			zone->elevation = elevations[i];
		}
	}	
}
	
// Finds out what type of a zone each zone is, depending on elevation, neighbouring zones, continent(?), etc.
void WorldGenerator::EvaluateZoneTypes()
{
	List<Zone*> & zones = world->zones;
	for (int i = 0; i < zones.Size(); ++i)
	{
		Zone * zone = zones[i];
		float elevation = zone->elevation;
		List<Zone*> neighbours = zone->neighbours;
		int belowWater = 0, aboveWater = 0, muchLower = 0, muchHigher = 0;
		for (int j = 0; j < neighbours.Size(); ++j)
		{
			Zone * neighbour = neighbours[j];
			float elevation2 = neighbour->elevation;
			if (elevation2 < world->oceanElevation)
				++belowWater;
			else if (elevation2 > world->oceanElevation)
				++aboveWater;
			if (elevation2 < elevation - 1.f)
				++muchLower;
			else if (elevation2 > elevation + 1.f)
				++muchHigher;
		}
		zone->isWater = elevation < world->oceanElevation;
		zone->isHighAltitude = elevation > world->oceanElevation + 3.f;
		/// Land-based zones.
		if (!zone->isWater)
		{
			zone->isCostal = belowWater > 0;
			zone->isIsland = belowWater >= neighbours.Size(); // Island if all neighbours are water.
		}
		/// Water-based zones.
		else 
		{
			zone->isLagoon = aboveWater >= neighbours.Size() - 1;
		}
		// Mountains can be both under-water and above water.. right?
		zone->isMountain = muchLower >= neighbours.Size();
		zone->isPit = muchHigher >= neighbours.Size();
	}
}

/// Places settlements on decent zones. Will vary on zone type how probable it is.
void WorldGenerator::PlaceSettlements()
{
	Random settlementRand = randomSeed;
	Random settlementSizeRand = randomSeed;
	int settlementsToCreate = numSettlements;
	int iterations = 0;
	while(settlementsToCreate > 0)
	{
		// Fail-safe thingy.
		++iterations;
		if (iterations > 10000)
			break;
		int rand = settlementRand.Randi(world->zones.Size());
		Zone * zone = world->zones[rand];
		if (zone->characters.Size())
			continue;
		// Don't live in the water, yo... unless it is a lagoon?
		if (zone->isWater)
			continue;
		// Check chance of adding a settlement, based on what kind of tile it is.
		float chance = 0.1f;
		// Double chance of settlements on the cost-lines.
		if (zone->isCostal)
		{
			chance *= 2.f;
		}
		// Reduce chance based on elevation if it is above 1?
		if (zone->elevation > 1.f)
			chance *= 1 / zone->elevation;
		float rChance = settlementRand.Randf(1.f);
		if (rChance > chance)
			continue;

		zone->hasSettlement = true;
		zone->numInhabitants = settlementSizeRand.Randi(10000);
		world->settlements.Add(zone);
		--settlementsToCreate;
	}
}

void WorldGenerator::CreateCharacters(Zone * forZone)
{
	if (forZone->buildingSlots.Size() == 0)
		GenerateBuildingSlots(forZone);

	// Probably want to have the zone laid out first... or? No! o.o Make characters first! Then create houses for them! o.o Yes, yes.
	int inhabitants = forZone->numInhabitants;
	int charactersToCreate; 
	// Some defaults.
	if (inhabitants < 10)
	{
		// A family? Make the parents, some grandparent and some kids? 
		// Add one house. o.o
		CreateFamilyZone(forZone);
	}
	// Village.
	else if (inhabitants < 100)
	{
		CreateVillageZone(forZone);
	}
	// Town
	else if (inhabitants < 1000)
	{
		CreateTownZone(forZone);		
	}
	// City
	else 
	{
		CreateCityZone(forZone);
	}
	/// Do post-creation settings.. whatever they might be.
}

// Population 1 to 10.
void WorldGenerator::CreateFamilyZone(Zone * forZone)
{
	BuildingSlot * slot = forZone->GetFreeBuildingSlot();
	// If not building, no use to add any people to live there.
	if (!slot)
		return;
	Building * building = new Building(ModelMan.GetModel("Buildings/House_3x4_2"), TexMan.GetTextureByColor(Color(125,125,0,255)));
	forZone->buildings.Add(building);
	slot->building = building;
	building->slot = slot;
}

// Population 11 to 100.
void WorldGenerator::CreateVillageZone(Zone * forZone)
{
	CreateFamilyZone(forZone);
}

// Population 100 to 1000.
void WorldGenerator::CreateTownZone(Zone * forZone)
{
	CreateVillageZone(forZone);
}

// Population 1000 and upward.
void WorldGenerator::CreateCityZone(Zone * forZone)
{
	CreateTownZone(forZone);
}

/// Zone-setup of building slots. Places where one would want to build anything.
void WorldGenerator::GenerateBuildingSlots(Zone * forZone)
{
	BuildingSlot * slot = new BuildingSlot();
	slot->position = Vector3f(0,0,0);
	slot->price = -1; // Not for sale?
	// 3x2 meters?
	slot->size = Vector2f(3,4);
	forZone->buildingSlots.Add(slot);
}

