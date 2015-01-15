/// Emil Hedemalm
/// 2013-12-22
/// Level that contains a grid of tiles, an elevation, objects, etc!

#ifndef TILE_MAP_LEVEL_H
#define TILE_MAP_LEVEL_H

#include "MathLib.h"
#include "Maps/Grids/TileGrid.h"
#include "Maps/Grids/GridObject.h"

/// Subclass of TileGrid! :)
class TileMapLevel : public TileGrid2D {
public:
	TileMapLevel(TileMapLevel * base = NULL);
	virtual ~TileMapLevel();

	/// Returns all tiles within a given boundary.
	List<Tile*> GetTilesWithinBoundary(float left, float right, float top, float bottom);
	/// Which tiles will be occupied when trying to paint target object. The list may contain NULL-pointers, meaning that the object cannot be placed at this location.
	List<Tile*> TilesRequired(GridObjectType * byObject, Vector2f ifPaintingOnLocation, Vector2f * resultingPivotLocation);
	/// Queries if it is possible.
	bool CanCreateObject(GridObjectType * ofType, Vector2f atLocation);
	/// Main function to add objects to the level. Binds it with the tiles and sorts it into the object list Y-wise for rendering. 
	bool AddObject(GridObject * got);
	/// Deletes target object, removing any bindings it had with current tiles.
	void DeleteObject(GridObject * go);
	/// Deletes all objects that exist on the given tiles. Take care to pause rendering before calling this.
	void DeleteObjects(List<Tile*> fromTiles);
	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	/// Resizes the level/grid-size.
//	void SetSize(Vector2i newSize);
//	Vector2i Size();
	int Elevation() const;

	virtual void Resize(Vector2i newSize);

	virtual void Render(GraphicsState & graphicsState);
	/// Returns the closest vacant & walkable tile to given coordinates.
	Tile * GetClosestVacantTile(Vector3i position);
	/// Returns a list of all tiles in the level.
	List<Tile*> GetTiles();

	List<GridObject*> objects;
protected:	
	/// Elevation, as in floor or whatever you prefer, though preferably indexes with 0 as base. Negative indices are OK.
	int elevation;
	
private:

};

#endif