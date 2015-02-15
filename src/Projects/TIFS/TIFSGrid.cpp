/// Emil Hedemalm
/// 2015-02-15
/// Grid used for creating maps for the TIFS game.

#include "TIFSGrid.h"
#include "Maps/Grids/TileGrid3D.h"
#include "Random/Random.h"

TIFSTile::TIFSTile()
: Tile(), occupied(false), isGround(false)
{
}


TIFSGrid::TIFSGrid()
{
}

TIFSGrid::~TIFSGrid()
{
}

/** Resizes to the chosen amount of tiles, centered on Vector3f(0,0,0)
	MapSize will dictate the distance between each tile, rounded to the nearest integer.
*/
void TIFSGrid::Resize(Vector3i gridSize, ConstVec3fr mapSize)
{
	grid.Resize(gridSize);
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
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

/// This may be either on ground or at some strategical point (key buildings?)
bool TIFSGrid::GetNewTurretPosition(Vector3f & turretPos)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	Random r;
	int tries = 0;
	while (tries < 100)
	{
		++tries;
		int index = r.Randi(tiles.Size() - 1);
		TIFSTile * tile = tiles[index];
		if (tile->occupied == true)
			continue;
		if (!tile->isGround)
			continue;
		turretPos = tile->position;
		tile->occupied = true;
		return true;
	}
	return false;
}

/** Requested slot-size in meters. List of available slot-sizes can be queried.
	Size is requested size in meters. Size will be updated to the size the new building can maximally take up.
	Position is stored in position.
*/
bool TIFSGrid::GetNewBuildingPosition(Vector3f & maxSize, Vector3f & position)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	Random r;
	int tries = 0;
	while (tries < 100)
	{
		++tries;
		int index = r.Randi(tiles.Size() - 1);
		TIFSTile * tile = tiles[index];
		if (tile->occupied == true)
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
			if (affectedTiles.Size() > 10)
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
			at->occupied = true;
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

/// Expands the list with all neighbours.
bool TIFSGrid::ExpandXZ(List<TIFSTile*> & tiles)
{
	List<TIFSTile*> toAdd;
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		List<TIFSTile*> toAddNow;
		toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(1,0,0)));
		toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(-1,0,0)));
		toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,0,1)));
		toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,0,-1)));
		
		// Check if possible now.
		for (int i = 0; i < toAddNow.Size(); ++i)
		{
			TIFSTile * tile = toAddNow[i];
			if (tile == NULL)
				return false;
			if (tile->occupied)
				return false;
		}
		toAdd.Add(toAddNow);
	}
	toAdd.RemoveUnsorted(tiles);
	tiles.Add(toAdd);
	return true;
}


/** Creates a road along the given lines. Fails if any position there is not vacant or already a road. 
	Should be called before placing buildings and turrets?
*/
bool SetRoad(List<Vector3i> positions);

