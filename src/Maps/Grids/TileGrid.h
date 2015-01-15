/// Emil Hedemalm
/// 2013-12-22

#ifndef TILE_GRID_H
#define TILE_GRID_H

#include "Grid.h"
#include "MathLib.h"
struct Tile;
class GraphicsState;

namespace GridType {
enum gridTypes {
	RECTANGLES,
	HEXAGONS,
};};


/// An Grid-class that can handles rectangular grids with square-sized waypoints/tiles.
class TileGrid2D : public Grid 
{
public:
	TileGrid2D();
	virtual ~TileGrid2D();

	/// Writes to file stream.
	void WriteTo(std::fstream & file);
	/// Reads from file stream.
	void ReadFrom(std::fstream & file);

	/// Generates waypoints for target navmesh. Returns number of created waypoints or 0 upon failure. Also performs connections between the waypoints.
	/// Do note that the navMesh will be cleared before new waypoints are added.
	virtual int GenerateWaypoints(NavMesh * navMesh, float maxNeighbourDistance);
	/// For re-sizing.
	Vector2i Size();

	/// See GridTypes above.
	void SetType(int gridType);
	/// For quadratic surfaces.
	void Resize(Vector2i newSize);
	/// Getter
	Tile * GetTile(int x, int y);
	/// Returns tile at target position or NULL if no existy.
	Tile * GetTile(Vector2i position);
	/// Rendering! Called from render-thread onry
	virtual void Render(GraphicsState & graphicsState);
protected:	
	int gridType;
	void Deallocate();
	Vector2i size;
	List<List<Tile*>*> tiles;
};

#endif