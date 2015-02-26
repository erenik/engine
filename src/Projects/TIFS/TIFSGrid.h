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
	Entity * entity; // Base entity associated with this tile. during map creation mainly.
};

struct Road
{
	Road();
	enum {
		X_ROAD,
		Z_ROAD,
	};

	bool isLonely;
	int direction;
	Vector3i startPosition;
	Vector3i min, max; // Min-max of all tiles within.
	List<TIFSTile*> tiles;
};

class Message;

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

	void ProcessMessage(Message * message);

	/// Used for the various algorithms inside.
	void SetExpansionFlags(bool x, bool y, bool z);
	/// Used for the various algorithms inside.
	void SetExpansionFlags(bool xPlus, bool xMinus, bool yPlus, bool yMinus, bool zPlus, bool zMinus);

	/// This may be either on ground or at some strategical point (key buildings?)
	bool GetNewPlayerPosition(Vector3f & playerPos);
	/// This may be either on ground or at some strategical point (key buildings?)
	bool GetNewTurretPosition(Vector3f & turretPos);
	/** Requested slot-size in meters. List of available slot-sizes can be queried.
		Size is requested size in meters. Size will be updated to the size the new building can maximally take up.
		Position is stored in position.
	*/
	bool GetNewBuildingPosition(Vector3f & maxSize, Vector3f & position, List<TIFSTile*> & relevantTiles);

	/// All the grid.
	void BasePlates();
	/// o.o
	void PlaceRoads(int roads);
	/// Creates the actual road, filling it with tiles :3
	void CreateRoad(Road * road);
	void ConnectLonelyRoads();

	/// Returns a list of all currently available slot-sizes.
	List<Vector3i> AvailableSlotSizes();

	String basePlateTexture; // For plates covering the whole map. o.o
	String roadTexture;
	float roadScale; // Of maximum, 0 to 1. causes gaps between each tile.
	int minDistanceBetweenParallelRoads; // in tiles.
	int maxTilesPerBuilding;
	int roadWidth;
	int maxRoadLength;
	float parallelDistanceThreshold;
	int triesPerBuilding;
	bool requireRoadConnections;
private:
	bool expandXPlus, expandYPlus, expandZPlus;
	bool expandXMinus, expandYMinus, expandZMinus;
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
	/// Roads o.o
	List<Road*> roads;

	int type;
};

#endif
