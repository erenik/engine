/// Emil Hedemalm
/// 2013-12-22
/// An abstract Grid-class that can be used to keep track of an array of potential positions and the connections in-between.

#include "Grid.h"
#include "Pathfinding/NavMesh.h"

/// Generates waypoints for target navmesh. Returns number of created waypoints or 0 upon failure. 
/// Do note that the navMesh will be cleared before new waypoints are added.
int Grid::GenerateWaypoints(NavMesh * navMesh){
	std::cout<<"\nGrid::GenerateWaypoints called. Forgot to implement this in the subclass?";
	assert(false);
	return 0;
}
/// Returns the type of this grid. See list above.
int Grid::Type(){
	return type;
}
String Grid::TypeName()
{
	switch(type){
		case TILE_GRID_2D: return "TileGrid2D";
		case TILE_GRID_3D: return "TileGrid3D";
		default: assert(false && "Invalid grid type or lacking type-name");
	}
	return "Bad type.";
}

/// protected:	
/// Constructor
Grid::Grid(int type)
: type(type){

}

Grid::~Grid(){
	
}