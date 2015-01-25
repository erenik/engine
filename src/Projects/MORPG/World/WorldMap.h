/// Emil Hedemalm
/// 2015-01-17
/// Rendering the world as a whole.

#include "World.h"
#include "Graphics/Camera/Camera.h"

class Entity;

/// Converts co-ordinates from the XY + elevation space used in the world-map to 3D renderable co-ordiantes using XZ and Y for elevation.
Vector3f FromWorldToWorldMap(Vector2i position, float elevation);
Vector3f FromWorldToWorldMap(Vector3i position);

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

	// Creates the camera, and moves it within bounds if it has glided too far out.
	void UpdateCamera();
	// Centers it so that the whole world is visible.
	void CenterCamera();
	// Registers all entities for display and makes the world-map camera active.
	void MakeActive();
	
	/// o.o
	List<Entity*> Entities();

	/// Camera dedicated to the world-map o.o
	Camera * worldMapCamera;
	Entity * worldEntity;
	Entity * oceanEntity;
	
	List<Entity*> settlementEntities;
};

// There is but one world map..
extern WorldMap worldMap;

