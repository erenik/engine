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
	bool GenerateWorld(World & world, bool newRandomSeed);

	// Various options.
	
	/// Size of the world overall, in tiles (width & height)
	Vector2i size;
	/// 0 to 1, how much surface should be covered with water.
	float water; 
	/// Default depth for water. Default -1.
	float waterDepth;
	/// Used to determine much the water spreads from its original sources.
	int waterLevel;
	/// 0 to 1, how much land surface should be mountainous (much easier to handle, since they will have fewer zones, etc.)
	float mountains;
	/// How much mountain height can be (maximum) beyond the standard 3.f.
	float mountainHeight;
	/// 0 to 1, how many of the tiles should be settlements. (Recommended value around 0.1?)
	float settlements;
	/// Smooths out elevation depending on adjacent tiles. Number indicates how many iterations the smoothing algorithm uses.
	int smoothing;
	/// Multiplier used for smoothing at each iteration. Default 0.1f
	float smoothingMultiplier;
private:
	void MarkWater();
	void RaiseWaterLevel();
	void MarkMountains();
	void Smooth();
	World * world;

	/// Seed to be used.
	Random randomSeed;
	
	List<Zone*> waterOrigins;
	
};

#endif
