/// Emil Hedemalm
/// 2013-12-22

#ifndef GRID_H
#define GRID_H

#include <String/AEString.h>

struct NavMesh;

enum {
	GRID_TYPE_NULL,
	TILE_GRID,	/// 2D-rectangular grid.
};

/// An abstract Grid-class that can be used to keep track of an array of potential positions and the connections in-between.
class Grid {
public:
	/// Generates waypoints for target navmesh. Returns number of created waypoints or 0 upon failure. 
	/// Do note that the navMesh will be cleared before new waypoints are added.
	virtual int GenerateWaypoints(NavMesh * navMesh);
	/// Returns the type of this grid. See list above.
	int Type();
	String TypeName();
protected:	
	/// Constructor
	Grid(int type);
	int type;
	virtual ~Grid();
private:

};

#endif