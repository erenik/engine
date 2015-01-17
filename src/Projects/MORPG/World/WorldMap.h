/// Emil Hedemalm
/// 2015-01-17
/// Rendering the world as a whole.

#include "World.h"

class Entity;

class WorldMap 
{
public:
	WorldMap();
	/// Updates the entire map, based on the World data.
	void Update();
	// Updates the ocean. Is automatically called from Update();
	void UpdateOcean();
	// Updates the settlement representations, usually in the form of some building or a crest and text.
	void UpdateSettlements();
	Entity * worldEntity;
	Entity * oceanEntity;
	
	List<Entity*> settlementEntities;
};

// There is but one world map..
extern WorldMap worldMap;

