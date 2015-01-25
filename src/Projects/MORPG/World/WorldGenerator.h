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
	virtual ~WorldGenerator();

	/// Generates a new world. Will return false if the world is not empty.
	bool GenerateWorld(World & world, bool newRandomSeed);
	bool GenerateSettlements(World & world, bool newRandomSeed);
	/// Generates the zone and local population within target zone.
	bool GenerateSettlement(Zone * inZone);

	// Various options.
	
	/// Size of the world overall, in tiles (width & height)
	Vector3i size;
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
	/// Exact number.
	int numSettlements;
	/// Smooths out elevation depending on adjacent tiles. Number indicates how many iterations the smoothing algorithm uses.
	int smoothing;
	/// Multiplier used for smoothing at each iteration. Default 0.1f
	float smoothingMultiplier;
private:
	void MarkWater();
	void RaiseWaterLevel();
	void MarkMountains();
	void Smooth();
	
	// Finds out what type of a zone each zone is, depending on elevation, neighbouring zones, continent(?), etc.
	void EvaluateZoneTypes();
	/// Places settlements on decent zones. Will vary on zone type how probable it is.
	void PlaceSettlements();
	/// Creates characters to live in the zone.
	void CreateCharacters(Zone * forZone);
	// Population 1 to 10.
	void CreateFamilyZone(Zone * forZone);
	// Population 11 to 100.
	void CreateVillageZone(Zone * forZone);
	// Population 100 to 1000.
	void CreateTownZone(Zone * forZone);
	// Population 1000 and upward.
	void CreateCityZone(Zone * forZone);

	/// Zone-setup of building slots. Places where one would want to build anything.
	void GenerateBuildingSlots(Zone * forZone);


	World * world;

	/// Seed to be used.
	Random randomSeed;
	
	List<Zone*> waterOrigins;
	
};

#endif
