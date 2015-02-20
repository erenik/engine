/// Emil Hedemalm
/// 2015-02-15
/// Grid used for creating maps for the TIFS game.

#include "TIFSGrid.h"
#include "Maps/Grids/TileGrid3D.h"
#include "Random/Random.h"
#include "File/LogFile.h"

TIFSTile::TIFSTile()
: Tile()
{
	Initialize();
}

// Called at construction, sets default values to the booleans. Does not change positions.
void TIFSTile::Initialize()
{
	isOccupied = isGround = isRoad = false;
	isBuilding = isTurret = false;
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
	LogMain("TIFSGrid::Resize - start", DEBUG);
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
	this->mapSize = mapSize;
	sizePerTile = mapSize / gridSize;
	LogMain("TIFSGrid::Resize - finish", DEBUG);
}

/// Used for the various algorithms inside.
void TIFSGrid::SetExpansionFlags(bool x, bool y, bool z)
{
	expandX = x;
	expandY = y;
	expandZ = z;
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
bool TIFSGrid::GetNewBuildingPosition(Vector3f & maxSize, Vector3f & position, List<TIFSTile*> & relevantTiles)
{
	SetExpansionFlags(true, false, true);
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	int tries = 0;
	while (tries < triesPerBuilding)
	{
		++tries;
		int index = buildingRandom.Randi(tiles.Size()) % tiles.Size();
		TIFSTile * tile = tiles[index];
		/// Test the initial tile.
		if (!AllGood(List<TIFSTile*>(tile), NO_OPTION))
			continue;

		// Check if surrounding tiles comply to the demand of the size?
		List<TIFSTile*> affectedTiles, queryTiles;
		List<TIFSTile*> testTiles = tile;
		// Expand size until we can no more?
		bool expansionOK = true;
		Vector3i min, max, scale, goodMin, goodMax;
		maxSize = min = max = tile->matrixPosition;
		while(expansionOK)
		{
			// Save from last iteration
			goodMin = min;
			goodMax = max;
			expansionOK = Expand(min, max, NO_OPTION);
			if (!expansionOK)
				break;
			scale = max - min + Vector3i(1,1,1);
			int numTiles = scale.x * scale.y * scale.z;
			/// Break if we have consumed enough tiles already. Don't want to exhaust the map?
			if (numTiles > maxTilesPerBuilding)
				break;
		}
		/// Paste over to min and max from the confirmed good values.
		min = goodMin;
		max = goodMax;
		/// Mark all query-tiles as occupied, so we get some spacing. -> Do this when actually placing the building.
		relevantTiles = grid.GetTiles(min,max);
		// Get size?
		Vector3i maxSizeInMatrix = ((max - min) + Vector3i(1,1,1));
		maxSize =  maxSizeInMatrix * sizePerTile;
		// Adjust position, if we change the expansion algorithm earlier?
		position = (max + min) * 0.5;
		/// Set floor and ground-level, though.
		position.y = (float)min.y;
		/// Multiply by grid-size.
		position *= sizePerTile;
		/// Translate so we center on 0,0,0
		position -= mapSize * 0.5f;
		position.y = 0;
		return true;
	}
	return false;	
}

/// o.o
Random roadRandom;
void TIFSGrid::PlaceRoads(int roads)
{
	/*
	LogMain("TIFS::PlaceRoads", DEBUG);
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
	*/
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
bool TIFSGrid::Expand(List<TIFSTile*> & tiles, bool x, bool y, bool z, int options)
{
	List<TIFSTile*> toAdd;
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		List<TIFSTile*> toAddNow;
		if (x)
		{
			// Add all in X we can?
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
		if (AllGood(toAddNow, options))
			toAdd.Add(toAddNow);
	}
	toAdd.RemoveUnsorted(tiles);
	/// Nothing to add? Then return false, we are fully maximized now.
	if (toAdd.Size() == 0)
		return false;
	tiles.Add(toAdd);
	return true;
}

/// Tries to expand all sides by 1 unit in the grid. The results are stored in the argument references.
/// Returns false if no expanion could be performed.
bool TIFSGrid::Expand(Vector3i & minVec, Vector3i & maxVec, int options /*= NO_OPTION*/)
{
	List<Vector3i> additionVectors;
	if (expandX)
		additionVectors.Add(Vector3i(1,0,0), Vector3i(-1,0,0));
	if (expandY)
		additionVectors.Add(Vector3i(0,1,0), Vector3i(0,-1,0));
	if (expandZ)
		additionVectors.Add(Vector3i(0,0,1), Vector3i(0,0,-1));

	Vector3i newMin, newMax, potentialNewMin, potentialNewMax;
	newMin = potentialNewMin = minVec;
	newMax = potentialNewMax = maxVec;
	bool goodArr[6];
	int bads = 0;
	memset(goodArr, 1, sizeof(bool) * 6);
	bool success = false;
	Vector3i relevantTestVector;
	for (int i = 0; i < additionVectors.Size(); ++i)
	{
		if (!goodArr[i])
			continue;
		// Each other expands positive and negative axis.
		bool max = (i % 2 == 0);
		Vector3i & expansionVector = additionVectors[i];
		if (max)
		{
			relevantTestVector = potentialNewMax += expansionVector;
			potentialNewMin = newMin; // revert from failure last loop.
		}
		else
		{
			relevantTestVector = potentialNewMin += expansionVector;
			potentialNewMax = newMax; // revert from failure last loop.
		}
		Vector3i minForTest, maxForTest;
		minForTest = potentialNewMin;
		maxForTest = potentialNewMax;
		if (expansionVector.x)
			minForTest.x = maxForTest.x = relevantTestVector.x;
		if (expansionVector.y)
			minForTest.y = maxForTest.y = relevantTestVector.y;
		if (expansionVector.z)
			minForTest.z = maxForTest.z = relevantTestVector.z;
		// Left..
		List<TIFSTile*> tiles;
		if (grid.OKGet(minForTest, maxForTest))
			tiles = grid.GetTiles(minForTest, maxForTest);
		bool good = AllGood(tiles, options);
		if (good)
		{
			newMax = potentialNewMax;
			newMin = potentialNewMin;
			success = true;
		}
		else 
		{
			++bads;
			goodArr[i] = false;
			if (bads == 6)
				break;
		}
	}
	minVec = newMin;
	maxVec = newMax;
	return success;
}

/// D:
bool TIFSGrid::AllGood(List<TIFSTile*> & tiles, int options)
{
	if (tiles.Size() == 0)
		return false;
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		bool bad = false;
		if (tile == NULL)
		{
			bad = true;
			if (options & IGNORE_NULL)
			{
				// Just ignore it and check next one.
				bad = false;
				tiles.RemoveIndex(i);
				--i;
				continue;
			}
		}
		else if (tile->isOccupied)
		{
			bad = true;
			if (tile->isRoad && options & IGNORE_ROADS)
				bad = false;
		}
		/// o.o Abort!
		if (bad)
		{
			return false;
		}
	}
	return true;
}



/** Creates a road along the given lines. Fails if any position there is not vacant or already a road. 
	Should be called before placing buildings and turrets?
*/
bool SetRoad(List<Vector3i> positions);

