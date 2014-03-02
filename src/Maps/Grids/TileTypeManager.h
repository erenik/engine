// Emil Hedemalm
// 2013-06-28

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#define TileTypeMan (*TileTypeManager::Instance())
#define TileTypes (*TileTypeManager::Instance())
#define TileMan (*TileTypeManager::Instance())

#include <Util/Util.h>
struct TileType;

/// For handling 'em
class TileTypeManager{
	TileTypeManager();
	~TileTypeManager();
	static TileTypeManager * tileTypeManager;
public:
	static void Allocate();
	static void Deallocate();
	static TileTypeManager * Instance();

	/// 
	void CreateDefaultTiles();

	/// Adds
	void AddTileType(TileType * type);
	// By name
	TileType * GetTileType(String name);
	TileType * GetTileTypeByIndex(int i);
	TileType * GetRandom();
	/// Fetches all
	List<TileType*> GetTypes() {return tileTypes;};

	int Types() { return tileTypes.Size();};
	// Returns index of the type, or -1 if it's not a valid tile.
	int Index(TileType * type);
	
	/// Returns next valid index available among the tiles.
	int GetNextIndex(int i);
	TileType * GetNext(TileType * type);
	/// Returns previous valid index available among the tiles.
	int GetPreviousIndex(int i);
	TileType * GetPrevious(TileType * type);
	// Prints 'em
	void PrintTypes();

	/// Loads tile-types from specified file o-o
	bool LoadTileTypes(String fromFile);
private:
	List<TileType*> tileTypes;
};

#endif

