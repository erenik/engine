// Emil Hedemalm
// 2013-06-28

#ifndef TILE_H
#define TILE_H

class Texture;
class Entity;
class Script;
class GridObject;

#include "String/AEString.h"
#include "MathLib.h"

/// Tile type
struct TileType{
	TileType();
	TileType(int type, String name, Vector3f color, Texture * t = NULL);
	/// Tile type index
	int type;
	/// Name of the tile-type.
	String name;
	/// To use when rendering
	Texture * texture;
	String textureSource;
	/// For stuff.
	Vector4f color;
	/// Extra flags.
	int flags;
	/// Simple flag.
	bool walkability;
};

/// Tile
struct Tile {
	friend class TileMap2D;
	friend class TileTypeManager;
public:
	Tile();
	~Tile();
	
	/// Writes to file stream.
	virtual void WriteTo(std::fstream & file);
	/// Reads from file stream.
	virtual void ReadFrom(std::fstream & file);

	/// Tile type index
	int typeIndex;
	TileType * type;
	/// Standard tile position: integral using only X and Y
	Vector3i position;
//	const int X() const { return x; };
//	const int Y() const { return y; };

	/// Checks where the tile is walkable, but also if there already exists an entity on this tile, etc. Events may be present, however.
	bool IsVacant();
	
	/// Objects currently in contact with this tile.
	List<GridObject*> objects;

	/// Any entities currently occupying this tile. Temporary variable that will NOT be saved to file.
	List<Entity*> entities; 

private:
	/// String containing any extra statistics about this tile. Any custom terrain, object or entity creation should be stored and handled here. This will be saved to file. 
	String description;
	
	/// Any events that should be triggered/relate to this tile. Source of the event will be saved to file.
	Script * event;

	/// Position. Only available in-game and not in saved format.
//	int x,y;
	/// Assigns all tiles an index that is related to their type.
	static void EnsureIndices();
	/// Saves indices for the type and nullifies the pointer.
	static void PrepareForReload();
	/// Re-directs the type pointer to the newly loaded tile-types.
	static void Reload();
	/// Private list for when tile types are updated/removed. o-o
	static List<Tile*> tiles;
};

#endif