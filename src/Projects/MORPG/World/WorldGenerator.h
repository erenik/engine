/// Emil Hedemalm
/// 2014-07-27
/// Main class for generating a world. Sub-class and overload for custom worlds.

#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include "World.h"
#include "Random/Random.h"
#include "MathLib.h"

class WorldGenerator 
{
public:
	WorldGenerator();
	/// Generates a new world. Will return false if the world is not empty.
	bool GenerateWorld(World & world);


	/// Random number generator.
	Random generatorRandom;
	// Various options.
	
	/// Size of the world overall, in tiles (width & height)
	Vector2i size;
	/// 0 to 1, how much surface should be covered with water.
	float water; 
	/// 0 to 1, how much land surface should be mountainous (much easier to handle, since they will have fewer zones, etc.)
	float mountains;
	/// 0 to 1, how many of the tiles should be settlements. (Recommended value around 0.1?)
	float settlements;
};

#endif
