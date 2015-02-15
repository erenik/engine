/// Emil Hedemalm
/// 2015-02-15

#ifndef TILE_GRID_3D_H
#define TILE_GRID_3D_H

#include "Tile.h"
#include "Grid.h"
#include "MathLib.h"
#include "Matrix/Matrix.h"

class GraphicsState;

/// An Grid-class that can handles rectangular grids with square-sized waypoints/tiles.
template <class T>
class TileGrid3D : public Grid 
{
public:
	TileGrid3D();
	virtual ~TileGrid3D();

	/// For quadratic surfaces.
	void Resize(Vector3i newSize);
	/// For re-sizing.
	Vector3i Size();

	List<T*> GetTiles();
	/// Tries to fetch a tile close to target position.
	T * GetFirstTile(ConstVec3fr atPosition, float maxDistance);
	T * GetClosestTile(ConstVec3fr atPosition, float maxDistance);
	T * GetTileByIndex(Vector3i index);

	/// Writes to file stream.
//	void WriteTo(std::fstream & file);
	/// Reads from file stream.
//	void ReadFrom(std::fstream & file);

	/// Generates waypoints for target navmesh. Returns number of created waypoints or 0 upon failure. Also performs connections between the waypoints.
	/// Do note that the navMesh will be cleared before new waypoints are added.
//	virtual int GenerateWaypoints(NavMesh * navMesh, float maxNeighbourDistance);
	
	/// Rendering! Called from render-thread onry
	virtual void Render(GraphicsState & graphicsState);
	

protected:	
	Vector3i size;
	void Deallocate();
	/// 3D-matrix for the tiles.
	Matrix<T *> tileMatrix;
};


/// Emil Hedemalm
/// 2013-12-22

// #include "TileGrid3D.h"
#include "Tile.h"
#include "TileTypeManager.h"
#include "TextureManager.h"
#include "Pathfinding/NavMesh.h"
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"
#include "Pathfinding/WaypointManager.h"

/// Macros for going through all tiles.
/*
#define FOR_TILE_START \
		for (int y = 0; y < size.y; ++x){\
		List<List<Tile>> & list = tiles[x];\
		for (int y = 0; y < size[1]; ++y){\
		Tile & tile = list[y];
#define FOR_TILE_END }}
*/

/// An Grid-class that can handles rectangular grids with square-sized waypoints/tiles.
template <class T>
TileGrid3D<T>::TileGrid3D()
: Grid(TILE_GRID_3D)
{
	std::cout<<"\nTileGrid3D constructor.";
}

template <class T>
TileGrid3D<T>::~TileGrid3D(){
	Deallocate();
}

template <class T>
void TileGrid3D<T>::Deallocate()
{
	// Grab array.
	tileMatrix.ClearAndDelete();
}

// Versions
#define TILE_GRID_VERSION_0 0// Initial version.

/// For re-sizing.
template <class T>
Vector3i TileGrid3D<T>::Size()
{
	return size;
}

template <class T>
void TileGrid3D<T>::Resize(Vector3i newSize)
{
	std::cout<<"\nTileGrid3D::Resize";
	Deallocate();
	size = newSize;
	/// Space in X between the columns, use for creating hexagonal grids, since the distance between each tile should always be 1.0 if possible.
	float spaceX = 0;
	/// Offset in Y, used for creating hexagonal grids.
	Vector2f eachOtherRowOffset;
	Vector2f rowSpacing(1,1);

	tileMatrix.ClearAndDelete();
	tileMatrix.Allocate(newSize);

	T ** tileArray = tileMatrix.GetArray();
	for (int i = 0; i < tileMatrix.Elements(); ++i)
	{
		T * tile = new T();
		tileArray[i] = tile;
		// Set indices of the tiles, for reference later.
		tile->matrixPosition = tileMatrix.GetLocationOf(tile);
	}
}

template <class T>
List<T*> TileGrid3D<T>::GetTiles()
{
	List<T*> tiles;
	tiles.AddArray(tileMatrix.Elements(), tileMatrix.GetArray());
	return tiles;
}


/// Getter
template <class T>
T * TileGrid3D<T>::GetFirstTile(ConstVec3fr position, float maxDistance)
{
	T ** tileArray = tileMatrix.GetArray();
	for (int i = 0; i < tileMatrix.Elements(); ++i)
	{
		T * tile = tileArray[i];
		if ((tile->position - position).Length() < maxDistance)
			return tile;
	}
	return NULL;
}

template <class T>
T * TileGrid3D<T>::GetClosestTile(ConstVec3fr atPosition, float maxDistance)
{
	Tile ** tileArray = tileMatrix.GetArray();
	Tile * closest = NULL;
	float closestDist = 100000.f;
	for (int i = 0; i < tileMatrix.Elements(); ++i)
	{
		Tile * tile = tileArray[i];
		float dist = (tile->position - atPosition).Length();
		if (dist > maxDistance)
			continue;
		if (dist < closestDist)
		{
			closest = tile;
			closestDist = dist;
		}
	}
	return closest;
}

template <class T>
T * TileGrid3D<T>::GetTileByIndex(Vector3i index)
{
	if (index.x < 0 || index.y < 0 || index.z < 0)
		return NULL;
	if (index.x >= size.x || index.y >= size.y || index.z >= size.z)
		return NULL;
	T * t = tileMatrix.At(index);
	return t;
}


/// Rendering! Called from render-thread onry
template <class T>
void TileGrid3D<T>::Render(GraphicsState & graphicsState)
{	
/*
	/// 
	Vector3f camPos = graphicsState.camera->Position();
	Camera & camera = *graphicsState.camera;
	Frustum frustum = camera.GetFrustum();
	Vector3f min = frustum.hitherBottomLeft - Vector3f(1,1,1), max = frustum.fartherTopRight + Vector3f(1,1,1);
	int tilesToRender = (int) ((max[0] - min[0]) * (max[1] - max[1]));
	if (tilesToRender > 1000)
		return;

	// Begin rendering.
	FOR_TILE_START
	
	
		/// Tile * tile declared via macro.
		float xPos = float(x);
		float yPos = float(y);		
		if (xPos < min[0] || xPos > max[0] ||
			yPos < min[1]|| yPos > max[1])
			continue;

		TileType * tt = tile->type;
		if (tt == NULL){
			// glColor4f(xPos / size[0], yPos / size[1], 0.5, 1);
		}
		else if (tt->textureSource.Length() < 3){
			Vector3f c = tt->color;
			// glColor4f(c[0], c[1], c[2], 1.f);
		}
		else if (tt->texture == NULL){
	        Texture * t = TexMan.LoadTexture(tt->textureSource);
	        tt->texture = t;
	        if (t)
	            TexMan.BufferizeTexture(t);
	        else {
	            std::cout<<"\nUnable to load tileType texture "<<tt->textureSource;
	        }
		}
		else {
		    // glColor4f(1,1,1,1);
	        // Texture enabled.
	        glEnable(GL_TEXTURE_2D);
	        glBindTexture(GL_TEXTURE_2D, tt->texture->glid);
		}

		glBegin(GL_QUADS);
	        glTexCoord2f(0.0f, 0.4f);
			glVertex3f(xPos-0.5f, yPos-0.5f, 0);
			glTexCoord2f(0.0f, 0.6f);
			glVertex3f(xPos-0.5f, yPos+0.5f, 0);
			glTexCoord2f(0.1f, 0.6f);
			glVertex3f(xPos+0.5f, yPos+0.5f, 0);
			glTexCoord2f(0.1f, 0.4f);
			glVertex3f(xPos+0.5f, yPos-0.5f, 0);
		glEnd();

	    glBindTexture(GL_TEXTURE_2D, 0);
	    glDisable(GL_TEXTURE_2D);

	FOR_TILE_END
/*
	for (int x = left; x < right; ++x){
		if (x < camPos[0] - viewRange || x > camPos[0] + viewRange)
			continue;
		for (int y = bottom; y < top; ++y){
			if (y < camPos[1] - viewRange || y > camPos[1] + viewRange)
				continue;
	
		}
	}*/
}


#endif