/// Emil Hedemalm
/// 2014-07-27
/// Main class for generating a world. Sub-class and overload for custom worlds.

#include "WorldGenerator.h"

#include "Zone.h"

WorldGenerator::WorldGenerator()
{
	size = Vector2i(20, 20);
	water = 0.2f;
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
	/// Generate zones.
	for (int x = 0; x < size.x; ++x)
	{
		for (int y = 0; y < size.y; ++y)
		{
			Zone * zone = new Zone();
			zone->position = Vector3i(x,y,0);
			world.zones.Add(zone);
		}
	}
	world.size = size;

	/// Mark water
	assert(water < 1.f);
	int numZones = world.zones.Size();
	int numWaterTiles = water * numZones;
	int waterTilesCreated = 0;
	while (waterTilesCreated < numWaterTiles)
	{
		int randomTileIndex = generatorRandom.Randi(numZones-1);
		Zone * zone = world.zones[randomTileIndex];
		if (zone->IsWater())
			continue;
		zone->SetWater(true);
		++waterTilesCreated;
	}

	return true;
}
