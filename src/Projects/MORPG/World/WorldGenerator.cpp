/// Emil Hedemalm
/// 2014-07-27
/// Main class for generating a world. Sub-class and overload for custom worlds.

#include "WorldGenerator.h"

#include "Zone.h"

WorldGenerator::WorldGenerator()
{
	size = Vector2i(20, 20);
	water = 0.2f;
	waterDepth = -1.f;
	waterLevel = 0;
	smoothing = 0;
	smoothingMultiplier = 0.1f;
	mountainHeight = 3.f;
}

/// Generates a new world.
bool WorldGenerator::GenerateWorld(World & worldToBeGenerated, bool newRandomSeed)
{
	// Various options.
	if (!worldToBeGenerated.empty)
		return false;

	waterOrigins.Clear();

	if (newRandomSeed)
	{
		randomSeed = Random();
	}

	this->world = &worldToBeGenerated;
	/*
	/// Size of the world overall, in tiles (width & height)
	Vector2i size;
	/// 0 to 1, how much surface should be covered with water.
	float water; 
	/// 0 to 1, how much land surface should be mountainous (much easier to handle, since they will have fewer zones, etc.)
	float mountains;
	/// 0 to 1, how many of the tiles should be settlements. (Recommended value around 0.1?)
	float settlements;
*/
	/// Allocate zone matrix
	world->zoneMatrix.SetDefaultValue(0);

	if (world->zoneMatrix.Size() != size)
	{
		world->Delete();
		world->zoneMatrix.Allocate(size);
		/// Generate zones.
		for (int x = 0; x < size.x; ++x)
		{
			for (int y = 0; y < size.y; ++y)
			{
				Zone * zone = new Zone();
				zone->position = Vector3i(x,y,0);
				world->zones.Add(zone);
				assert(world->zoneMatrix[x][y] == 0);
				world->zoneMatrix[x][y] = zone;
			}
		}
		world->ConnectZonesByDistance(1.2f);
	}
	else 
	{
		// Reset all zones to default.
		for (int i = 0; i < world->zones.Size(); ++i)
		{
			Zone * zone = world->zones[i];
			zone->Nullify();
		}
	}
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
	