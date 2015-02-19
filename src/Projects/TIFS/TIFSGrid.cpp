/// Emil Hedemalm
/// 2015-02-15
/// Grid used for creating maps for the TIFS game.

#include "TIFSGrid.h"
#include "Maps/Grids/TileGrid3D.h"
#include "Random/Random.h"

TIFSTile::TIFSTile()
: Tile()
{
	Initialize();
}

// Called at construction, sets default values to the booleans. Does not change positions.
void TIFSTile::Initialize()
{
	isOccupied = isGround = isRoad = false;
}



TIFSGrid::TIFSGrid()
{
	maxTilesPerBuilding = 10;
	roadWidth = 1;
	triesPerBuilding = 200;
}

TIFSGrid::~TIFSGrid()
{
}

/** Resizes to the chosen amount of tiles, centered on Vector3f(0,0,0)
	MapSize will dictate the distance between each tile, rounded to the nearest integer.
*/
void TIFSGrid::Resize(Vector3i gridSize, ConstVec3fr mapSize)
{
	// Only re-allocate the grid if necessary.
	bool clean = true;
	if (grid.Size() != gridSize)
	{
		grid.Resize(gridSize);
		clean = false;
	}

	List<TIFSTile*> tiles = grid.GetTiles();
	// Clean it manually here otherwise.
	if (clean)
	{
		for (int i = 0; i < tiles.Size(); ++i)
		{
			TIFSTile * tile = tiles[i];
			tile->Initialize();
		}	
	}
	// Set their positions.
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		// XZ
		tile->position = (tile->matrixPosition / gridSize) * mapSize - mapSize * 0.5f;
		// Y
		tile->position.y = tile->matrixPosition.y * mapSize.y;
		/// Mark default ground-plane.
		if (tile->matrixPosition.y == 0)
			tile->isGround = true;
	}	
}

Random playerRandom;

/// This may be either on ground or at some strategical point (key buildings?)
bool TIFSGrid::GetNewPlayerPosition(Vector3f & playerPos)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	int tries = 0;
	while (tries < 1000)
	{
		++tries;
		int index = playerRandom.Randi(tiles.Size() - 1);
		TIFSTile * tile = tiles[index];
		if (tile->isOccupied == true)
			continue;
		if (!tile->isGround)
			continue;
		playerPos = tile->position;
		tile->isOccupied = true;
		return true;
	}
	return false;
}


Random turretRandom;

/// This may be either on ground or at some strategical point (key buildings?)
bool TIFSGrid::GetNewTurretPosition(Vector3f & turretPos)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	int tries = 0;
	while (tries < 100)
	{
		++tries;
		int index = turretRandom.Randi(tiles.Size() - 1);
		TIFSTile * tile = tiles[index];
		if (tile->isOccupied == true)
			continue;
		if (!tile->isGround)
			continue;
		turretPos = tile->position;
		tile->isOccupied = true;
		return true;
	}
	return false;
}

/** Requested slot-size in meters. List of available slot-sizes can be queried.
	Size is requested size in meters. Size will be updated to the size the new building can maximally take up.
	Position is stored in position.
*/
Random buildingRandom;
bool TIFSGrid::GetNewBuildingPosition(Vector3f & maxSize, Vector3f & position)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	int tries = 0;
	while (tries < triesPerBuilding)
	{
		++tries;
		int index = buildingRandom.Randi(tiles.Size()) % tiles.Size();
		TIFSTile * tile = tiles[index];
		if (tile->isOccupied == true)
			continue;
		if (!tile->isGround)
			continue;

		// Check if surrounding tiles comply to the demand of the size?
		List<TIFSTile*> affectedTiles, queryTiles;
		List<TIFSTile*> testTiles = tile;
		// Expand size until we can no more?
		bool expansionOK = true;
		while(expansionOK)
		{
			/// Break if we have consumed enough tiles already. Don't want to exhaust the map?
			if (affectedTiles.Size() > maxTilesPerBuilding)
				break;
			affectedTiles = queryTiles;
			queryTiles = testTiles;
			expansionOK = ExpandXZ(testTiles);
		}
		/// If 0 affected tiles, the expansion failed (e.g. the requirement of roads somehow failed).
		if (affectedTiles.Size() == 0)
			continue;
		position = tile->position;
		Vector3f minPos(9999,9999,9999), maxPos(-9999,-9999,-9999);
		for (int i = 0; i < affectedTiles.Size(); ++i)
		{
			TIFSTile * at = affectedTiles[i];
			minPos = Vector3f::Minimum(minPos, at->position);
			maxPos = Vector3f::Maximum(maxPos, at->position);
		}
		/// Mark all query-tiles as occupied, so we get some spacing.
		for (int i = 0; i < queryTiles.Size(); ++i)
		{
			TIFSTile * at = queryTiles[i];
			at->isOccupied = true;
		}
		// Get size?
		maxSize = maxPos - minPos;
		// Adjust position, if we change the expansion algorithm earlier?
		position = (maxPos + minPos) * 0.5;
		/// Set floor and ground-level, though.
		position.y = minPos.y;
		return true;
	}
	return false;	
}

/// o.o
Random roadRandom;
void TIFSGrid::PlaceRoads(int roads)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	int attempts = 0;
	int roadsCreated = 0;
	bool ok = true;
	while (roadsCreated < roads && attempts < 100)
	{
		++attempts;
		/// Choose a random X or Y-path, not along the edges.
		TIFSTile * startTile = tiles[roadRandom.Randi(tiles.Size()) - 1];
		if (!startTile->isGround)
			continue;
		if (startTile->isOccupied)
			continue;

		List<TIFSTile*> tiles = startTile;
		// Make sure it's
		int dir = roadRandom.Randi(1000) % 2; // 0 to 1 did not seem to work..
		for (int i = 0; i < roadWidth; ++i)
		{
			if (dir == 0) // Z width for X-roads.
				ok = Expand(tiles, false, false, true, IGNORE_NULL | IGNORE_ROADS);
			else  // X-width for Z-roads.
				ok = Expand(tiles, true, false, false, IGNORE_NULL | IGNORE_ROADS);
		}
		// Not wide enough?
		if (!ok)
			continue;

		if (dir == 0)
			while(Expand(tiles, true, false, false, IGNORE_NULL | IGNORE_ROADS));
		else 
			while(Expand(tiles, false, false, true, IGNORE_NULL | IGNORE_ROADS));
		
		for (int i = 0; i < tiles.Size(); ++i)
		{
			TIFSTile * t = tiles[i];
			t->isOccupied = true;
			t->isRoad = true;
		}
		++roadsCreated;
	}
}

/// Expands the list with all neighbours.
bool TIFSGrid::ExpandZ(List<TIFSTile*> & tiles)
{
	return Expand(tiles, false, false, true);
}

/// Expands the list with all neighbours.
bool TIFSGrid::ExpandX(List<TIFSTile*> & tiles)
{
	return Expand(tiles, true, false, false);
}

/// Expands the list with all neighbours.
bool TIFSGrid::ExpandXZ(List<TIFSTile*> & tiles)
{
	return Expand(tiles, true, false, true);
}

/// Expands the list with all neighbours. Max size is increased based on the positions of the tiles and the grid.
bool TIFSGrid::Expand(List<TIFSTile*> & tiles, bool x, bool y, bool z, int option)
{
	List<TIFSTile*> toAdd;
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		List<TIFSTile*> toAddNow;
		if (x)
		{
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(1,0,0)));
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(-1,0,0)));
		}
		if (z)
		{
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,0,1)));
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,0,-1)));
		}
		if (y)
		{
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,1,0)));
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,-1,0)));
		}
		// Check if possible now.
		for (int i = 0; i < toAddNow.Size(); ++i)
		{
			bool bad = false;
			TIFSTile * tile = toAddNow[i];
			if (tile == NULL)
			{
				bad = true;
				if (option & IGNORE_NULL)
				{
					// Just ignore it and check next one.
					bad = false;
					toAddNow.RemoveIndex(i);
					--i;
					continue;
				}
			}
			else if (tile->isOccupied)
			{
				bad = true;
				if (tile->isRoad && option & IGNORE_ROADS)
					bad = false;
			}
			/// o.o Abort!
			if (bad)
			{
				return false;
			}
		}
		toAdd.Add(toAddNow);
	}
	toAdd.RemoveUnsorted(tiles);
	/// Nothing to add? Then return false, we are fully maximized now.
	if (toAdd.Size() == 0)
		return false;
	tiles.Add(toAdd);
	return true;
}


/** Creates a road along the given lines. Fails if any position there is not vacant or already a road. 
	Should be called before placing buildings and turrets?
*/
bool SetRoad(List<Vector3i> positions);

