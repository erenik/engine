/// Emil Hedemalm
/// 2014-07-27
/// Main class for generating a world. Sub-class and overload for custom worlds.

#include "WorldGenerator.h"

#include "Zone.h"

WorldGenerator::WorldGenerator()
{
	size = Vector2i(20, 20);
	water = 0.2f;
	waterLevel = 0;
}

/// Generates a new world.
bool WorldGenerator::GenerateWorld(World & world)
{
	// Various options.
	if (!world.empty)
		return false;
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
	world.zoneMatrix.SetDefaultValue(0);
	world.zoneMatrix.Allocate(size);
	
//	world.zoneMatrix.PrintContents();


	/// Generate zones.
	for (int x = 0; x < size.x; ++x)
	{
		for (int y = 0; y < size.y; ++y)
		{
			Zone * zone = new Zone();
			zone->position = Vector3i(x,y,0);
			world.zones.Add(zone);
			assert(world.zoneMatrix[x][y] == 0);
			world.zoneMatrix[x][y] = zone;
		}
	}
	world.ConnectZonesByDistance(1.2f);

	world.size = size;

//	world.zoneMatrix.PrintContents();

	/// Mark water
	assert(water < 1.f);
	int numZones = world.zones.Size();
	int numWaterTiles = water * numZones;
	int waterTilesCreated = 0;

	List<Zone*> waterOrigins;
	int failedAttempts = 0;
	while (waterTilesCreated < numWaterTiles)
	{
		int randomTileIndex = generatorRandom.Randi(numZones-1);
		Zone * zone = world.zones[randomTileIndex];
		if (zone->IsWater())
		{
			++failedAttempts;
			if (failedAttempts > 1000)
				break;
			continue;
		}
		zone->SetWater(true);
		++waterTilesCreated;
		waterOrigins.Add(zone);
	}

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
					neighbours[k]->SetWater(true);
				}
			}
		}
		waterOrigins = newWaterOrigins;
	}
	
	return true;
}
