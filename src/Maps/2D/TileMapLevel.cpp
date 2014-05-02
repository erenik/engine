/// Emil Hedemalm
/// 2013-12-22
/// Level that contains a grid of tiles, an elevation and then some 

#include "Maps/Grids/Tile.h"
#include "TileMapLevel.h"
#include "Maps/Grids/TileTypeManager.h"
#include <fstream>

/// Macros for going through all tiles.
#define FOR_TILE_START for (int y = 0; y < size.y; ++y){\
		List<Tile*> * yTileList = tiles[y];\
		for (int x = 0; x < size.x; ++x){\
		Tile * tile = (*yTileList)[x];
#define FOR_TILE_END }}


TileMapLevel::TileMapLevel(TileMapLevel * base)
: TileGrid2D(){
	std::cout<<"\nTileMapLevel constructor.";
	// Set default values
	elevation = 0;
	// Copy values as possible.
	if (base)
		elevation = base->elevation;
}

TileMapLevel::~TileMapLevel()
{
	std::cout<<"\nTileMapLevel destructor";
	/// Delete all objects, yo.
	objects.ClearAndDelete();
}

/// Returns all tiles within a given boundary.
List<Tile*> TileMapLevel::GetTilesWithinBoundary(float left, float right, float top, float bottom)
{
	List<Tile*> tilesWithin;
	for (float x = left; x < right; x++)
	{
		for (float y = top; y > bottom; --y)
		{
			Tile * t = GetTile((int)x,(int)y);
			if (t)
				tilesWithin.Add(t);
		}
	} 
	return tilesWithin;
}
/// Which tiles will be occupied when trying to paint target object. The list may contain NULL-pointers, meaning that the object cannot be placed at this location.
List<Tile*> TileMapLevel::TilesRequired(GridObjectType * byObjectType, Vector2f ifPaintingOnLocation, Vector2f * resultingPivotLocation)
{

	List<Tile*> list;
	if (byObjectType == NULL)
		return list;

	if (byObjectType->size.GeometricSum() < 1)
		return list;

	Vector3f position = ifPaintingOnLocation;
	Vector2f pivotPosition;
	Vector2i objectSize;
	GridObjectType * objectType = byObjectType;
	objectSize = objectType->size;
	pivotPosition = objectType->pivotPosition;
	
	if (objectSize.x < 1)
		objectSize.x = 1;
	if (objectSize.y < 1)
		objectSize.y = 1;
	if (pivotPosition.MaxPart() == 0){
		pivotPosition = objectSize;
		pivotPosition /= 2.0f;
	}

	float pivotToLeft = -pivotPosition.x;
	float pivotToRight = objectSize.x - pivotPosition.x;
	float pivotToTop = objectSize.y - pivotPosition.y;
	float pivotToBottom = -pivotPosition.y;

	Vector2i doublePivot = pivotPosition * 2;

	/// Depending on the pivot position, adjust the anchor for where to center painting of it.
	if (doublePivot.x % 2 == 0){
		/// Even amount of cells in width, center it between the closest two cells X-wise.
		float x = position.x;
		float roundedX = RoundFloat(x);
		float otherX = 0;
		if (roundedX <= x)
			otherX = RoundFloat(x + 0.5f);
		else 
			otherX = RoundFloat(x - 0.5f);
		position.x = (RoundFloat(roundedX) + RoundFloat(otherX)) / 2;
	}
	else {
		position.x = RoundFloat(position.x);
	}
	if (doublePivot.y % 2 == 0){
		float y = position.y;
		float roundedY = RoundFloat(y);
		float otherY = 0;
		if (roundedY <= y){
			otherY = RoundFloat(y + 0.5f);
		}
		else {
			otherY = RoundFloat(y - 0.5f);
		}
		position.y = (RoundFloat(roundedY) + RoundFloat(otherY)) / 2;
	}
	else {
		position.y = RoundFloat(position.y);
	}

	//	glVertex3f(position.x + pivotToLeft, position.y + pivotToBottom,z);
	//	glVertex3f(position.x + pivotToLeft, position.y + pivotToTop,z);
	//	glVertex3f(position.x + pivotToRight, position.y + pivotToTop,z);
	//	glVertex3f(position.x + pivotToRight, position.y + pivotToBottom,z);
		
	float left = position.x + pivotToLeft;
	float right = position.x + pivotToRight;
	float top = position.y + pivotToTop;
	float bottom = position.y + pivotToBottom;

	/// Now that the true pivot location has been found, including pivots if needed, generate a list of positions that we require.
	int error = 0;

	/// Save it if queried.
	if (resultingPivotLocation)
		*resultingPivotLocation = position;
	
	/// Check which tiles are needed.
	List<Vector2i> positionsNeeded;
	for (int y = 0; y < objectSize.y; ++y){
		for (int x = 0; x < objectSize.x; ++x){
			/// Check passability first. We don't need tiles not marked as passable.
			int index = y * objectSize.x + x;
			/// If passable, skip it.
			bool passability = *byObjectType->passability[index];
			if (passability)
				continue;
			/// Fetch tile for the position, accounting for adjustments due to request location and pivot point.
			/// Tile position = pivot current position, + difference between pivot position and tile position as defined in the object type.
			float tileX = x + 0.5f, 
				tileY = (objectSize.y - y) - 0.5f;

			Vector2f tilePosition(tileX, tileY);
			Vector2f objectCenter = byObjectType->size;
			objectCenter *= 0.5f;
			Vector2f pivotToCenter = objectCenter - byObjectType->pivotPosition;
			Vector2f centerToTile = tilePosition - objectCenter;
			Vector2f pos = position + pivotToCenter + centerToTile;
			Vector2i pos2i = pos;
			positionsNeeded.Add(pos2i);
		}
	}

	/// Get tiles for desired needs. If not available, mark as not werking!
	for (int i = 0; i < positionsNeeded.Size(); ++i)
	{
		Vector2i pos = positionsNeeded[i];
		Tile * t = GetTile(pos.x, pos.y);
		list.Add(t);
	}
	return list;
}
/// Queries if it is possible.
bool TileMapLevel::CanCreateObject(GridObjectType * ofType, Vector2f atLocation)
{
	List<Tile*> requiredTileLocations = TilesRequired(ofType, atLocation, NULL);
	if (requiredTileLocations.Size() == 0){
		std::cout<<"Error, required tiles 0. Should not be paintable, yo.";
		return false;
	}
	for (int i = 0; i < requiredTileLocations.Size(); ++i)
	{
		Tile * tile = requiredTileLocations[i];
		if (!tile)
			return false;
		/// hoccupied! no workie!
		if (!tile->IsVacant())
			return false;
	}
	return true;
}
/// Attempts to create an object at target location. Returns the object upon success.
bool TileMapLevel::AddObject(GridObject * go)
{
	if (go == NULL || go->type == NULL)
		return false;		
	Vector2f location = go->position;
	/// Use magic to find out if we are allowed to create the object at all.
	if (!CanCreateObject(go->type, location))
		return false;
	
	/// Fetch required tiles and link 'em up.
	Vector2f resultingLocation;
	List<Tile*> required = TilesRequired(go->type, location, &resultingLocation);
	go->position = resultingLocation;
	for (int i = 0; i < required.Size(); ++i)
	{
		Tile * tile = required[i];
		/// Link it.
		tile->objects.Add(go);
		go->attachedTiles.Add(tile);
	}
	/// Sort it into the list.
	bool inserted = false;
	/// Sort so that those with highest Y are at the front, since we want to render from back to front. This sorting should probably be moved to a render-thread. o.o'
	for (int i = 0; i < objects.Size(); ++i){
		GridObject * g = objects[i];
		if (go->position.y > g->position.y){
			objects.Insert(go, i);
			inserted = true;
			break;
		}
	}
	if (!inserted)
		objects.Add(go);
	return true;
}

/// Deletes target object, removing any bindings it had with current tiles.
void TileMapLevel::DeleteObject(GridObject * go)
{
	for (int i = 0; i < go->attachedTiles.Size(); ++i)
	{
		Tile * tile = go->attachedTiles[i];
		tile->objects.Remove(go);
	}
	// Delete it.
	objects.Remove(go, ListOption::RETAIN_ORDER);
	delete go;
}

/// Deletes all objects that exist on the given tiles. Take care to pause rendering before calling this.
void TileMapLevel::DeleteObjects(List<Tile*> fromTiles)
{
	for (int i = 0; i < fromTiles.Size(); ++i){
		Tile * tile = fromTiles[i];
		if (!tile)
			continue;
		while(tile->objects.Size()){
			GridObject * go = tile->objects[0];
			DeleteObject(go);
		}
	}
}

// Versions
#define TILE_MAP_LEVEL_VERSION_0 0// Initial version.
/** Tile Map level Versions
	0 - Initial version
	1 - GridObjects added, 2014-02-27
*/
int tileMapLevelVersion = 1;

/// Writes to file stream.
void TileMapLevel::WriteTo(std::fstream & file)
{
	// Write version
	file.write((char*)&tileMapLevelVersion, sizeof(int));
	// Write elevation
	file.write((char*)&elevation, sizeof(int));
	// Write size
	size.WriteTo(file);
	// Write all tiles
	FOR_TILE_START;
		tile->WriteTo(file);
	FOR_TILE_END;

	/// Write objects
	int numObjects = objects.Size();
	file.write((char*)&numObjects, sizeof(int));
	for (int i = 0; i < objects.Size(); ++i){
		GridObject * go = objects[i];
		go->WriteTo(file);
	}
}

/** Tile Map level Versions
	0 - Initial version
	1 - GridObjects added, 2014-02-27
*/
/// Reads from file stream.
void TileMapLevel::ReadFrom(std::fstream & file){
	// Read version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(tileMapLevelVersion == version);
	// Read elevation
	file.read((char*)&elevation, sizeof(int));
	// Read size
	size.ReadFrom(file);
	// Allocate
	Resize(size);
	// Read tile data
	FOR_TILE_START
		/// Raw data.
		tile->ReadFrom(file);
		/// Assign type straight away, since objects need it to be added.
		tile->type = TileTypeMan.GetTileTypeByIndex(tile->typeIndex);
	FOR_TILE_END;
	/// First version stops here.
	if (version == 0)
		return;
	/// Read objects.
	int numObjects;
	file.read((char*)&numObjects, sizeof(int));
	for (int i = 0; i < numObjects; ++i)
	{
		GridObject * go = new GridObject();
		bool success = go->ReadFrom(file);
		if (!success){
			delete go;
			continue;
		}
		/// Fetch type before trying to add it...!
		go->type = GridObjectTypeMan.GetType(go->typeName);
		this->AddObject(go);
	}
}


int TileMapLevel::Elevation() const { 
	return elevation;
};

void TileMapLevel::Resize(Vector2i newSize){
	std::cout<<"\nTileMapLevel::Resize";
	TileGrid2D::Resize(newSize);
	std::cout<<"\nTileMapLevel::Resize - setting appropriate Z-values";
	std::cout<<"\nTileMapLevel::Resize - setting appropriate Z-values";

	std::cout<<"\nRows: "<<tiles.Size();
	for (int y = 0; y < size.y; ++y){
		List<Tile*> * list = tiles[y];
		for (int x = 0; x < size.x; ++x){
			Tile * tile = (*list)[x];
		}
	}

	/// Set appropriate Z-value for all tiles.
	FOR_TILE_START
		tile->position.z = elevation;
	FOR_TILE_END
}

void TileMapLevel::Render(GraphicsState & graphicsState){
//	std::cout<<"\nTileMapLevel:: Render";
	
	TileGrid2D::Render(graphicsState);
	/*
	FOR_TILE_START
		// Render da tile!
		glBegin(GL_QUADS);

	FOR_TILE_END
*/
}

/// Returns the closest vacant & walkable tile to given coordinates.
Tile * TileMapLevel::GetClosestVacantTile(Vector3i position){
	float closestDistance = 100000.0f;
	Tile * closestVacantTile = NULL;
	FOR_TILE_START
		if (!tile->IsVacant())
			continue;
		float distance = (tile->position - position).Length();
		if (distance < closestDistance){
			closestVacantTile = tile;
			closestDistance = distance;
		}
	FOR_TILE_END
	return closestVacantTile;
}

/// Returns a list of all tiles in the level.
List<Tile*> TileMapLevel::GetTiles(){
	List<Tile*> list;
	FOR_TILE_START
		list.Add(tile);
	FOR_TILE_END
	return list;
}