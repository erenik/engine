/// Emil Hedemalm
/// 2015-02-15
/// Grid used for creating maps for the TIFS game.

#ifndef TIFS_GRID_H
#define TIFS_GRID_H

#include "Maps/Grids/Tile.h"
#include "Maps/Grids/TileGrid3D.h"

class TIFSTile : public Tile
{
public:
	TIFSTile();
	// Called at construction, sets default values to the booleans. Does not change positions.
	void Initialize();
	bool isOccupied;
	bool isGround; // Default false, set ground manually.
	bool isRoad;
	bool isBuilding;
	bool isTurret;
};

class TIFSGrid 
{
public:
	TIFSGrid();
	~TIFSGrid();

	/// Clears any pointers from the grid, making it re-usable.
	void Clear();
	/** Resizes to the chosen amount of tiles, centered on Vector3f(0,0,0)
		MapSize will dictate the distance between each tile, rounded to the nearest integer.
	*/
	void Resize(Vector3i gridSize, ConstVec3fr mapSize);

	/// Used for the various algorithms inside.
	void SetExpansionFlags(bool x, bool y, bool z);

	/// This may be either on ground or at some strategical point (key buildings?)
	bool GetNewPlayerPosition(Vector3f & playerPos);
	/// This may be either on ground or at some strategical point (key buildings?)
	bool GetNewTurretPosition(Vector3f & turretPos);
	/** Requested slot-size in meters. List of available slot-sizes can be queried.
		Size is requested size in meters. Size will be updated to the size the new building can maximally take up.
		Position is stored in position.
	*/
	bool GetNewBuildingPosition(Vector3f & maxSize, Vector3f & position, List<TIFSTile*> & relevantTiles);

	/// o.o
	void PlaceRoads(int roads);
	/** Creates a road along the given lines. Fails if any position there is not vacant or already a road. 
		Should be called before placing buildings and turrets?
	*/
	bool SetRoad(List<Vector3i> positions);

	/// Returns a list of all currently available slot-sizes.
	List<Vector3i> AvailableSlotSizes();


	int maxTilesPerBuilding;
	int roadWidth;
	int triesPerBuilding;
private:
	bool expandX, expandY, expandZ;
	Vector3f mapSize;
	/// o.o 
	Vector3f sizePerTile;
	/// Expands the list with all neighbours. Max size is increased based on the positions of the tiles and the grid.
	bool ExpandXZ(List<TIFSTile*> & tiles);
	/// Expands the list with all neighbours. Max size is increased based on the positions of the tiles and the grid.
	bool ExpandZ(List<TIFSTile*> & tiles);
	/// Expands the list with all neighbours. Max size is increased based on the positions of the tiles and the grid.
	bool ExpandX(List<TIFSTile*> & tiles);
	
	enum {
		NO_OPTION = 0,
		IGNORE_ROADS = 1,
		IGNORE_NULL = 2,
	};
	/// Expands the list with all neighbours. Max size is increased based on the positions of the tiles and the grid.
	bool Expand(List<TIFSTile*> & tiles, bool x, bool y, bool z, int option = NO_OPTION);

	/// Tries to expand all sides by 1 unit in the grid. The results are stored in the argument references.
	/// Returns false if no expanion could be performed.
	bool Expand(Vector3i & min, Vector3i & max, int option = NO_OPTION);
	/// D:
	bool AllGood(List<TIFSTile*> & tiels, int options);

//	Grid * grid;

	/// Will become pointer-based in the grid.
	TileGrid3D<TIFSTile> grid;

	int type;
};

#endif
