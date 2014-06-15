/// Emil Hedemalm
/// 2013-12-22

#include "TileGrid.h"
#include "Tile.h"
#include "TileTypeManager.h"
#include "TextureManager.h"
#include "Pathfinding/NavMesh.h"
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"
#include "Pathfinding/WaypointManager.h"

/// Macros for going through all tiles.
#define FOR_TILE_START for (int y = 0; y < size.y; ++y){\
		List<Tile*> * list = tiles[y];\
		for (int x = 0; x < size.x; ++x){\
		Tile * tile = (*list)[x];
#define FOR_TILE_END }}


/// An Grid-class that can handles rectangular grids with square-sized waypoints/tiles.
TileGrid2D::TileGrid2D()
: Grid(TILE_GRID)
{
	std::cout<<"\nTileGrid2D constructor.";
}
TileGrid2D::~TileGrid2D(){
	Deallocate();
}

// Versions
#define TILE_GRID_VERSION_0 0// Initial version.
int tileGridVersion = TILE_GRID_VERSION_0;

/// Writes to file stream.
void TileGrid2D::WriteTo(std::fstream & file){
	// Write version
	file.write((char*)&tileGridVersion, sizeof(int));
	// Write size
	size.WriteTo(file);
	
	// Write tile data
	FOR_TILE_START
		tile->ReadFrom(file);
	FOR_TILE_END
}

/// Reads from file stream.
void TileGrid2D::ReadFrom(std::fstream & file){
	// Read version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(tileGridVersion == version);

}

/// Generates waypoints for target navmesh. Returns number of created waypoints or 0 upon failure. 
/// Do note that the navMesh will be cleared before new waypoints are added.
int TileGrid2D::GenerateWaypoints(NavMesh * navMesh, float maxNeighbourDistance)
{
	/// Wait until its claimable
	while(!WaypointMan.GetActiveNavMeshMutex())
		;
	int waypointsPre = navMesh->waypoints;
		
	std::cout<<"\nTileGrid2D::GenerateWaypoints";
	/// Create 'em
	FOR_TILE_START
		Waypoint * wp = new Waypoint();
		wp->position = tile->position;
		/// Save the tile as the data-node, so we can access all this data later!
		wp->pData = tile;
//		std::cout<<"\nTile types: "<<TileTypes.Types();
		assert(TileTypes.Types() && "Load and set tile types first?");
		/// Null-types
		if (tile->type == NULL){
			wp->passable = tile->IsVacant();	
		}
		/// Valid types
		else {
			wp->passable = tile->type->walkability;
			/// If any objects exist on this tile, mark it as unwalkable!
			if (tile->objects.Size())
			{
				wp->passable = false;
			}
		}


		navMesh->AddWaypoint(wp);
	FOR_TILE_END

	navMesh->ConnectWaypointsByProximity(maxNeighbourDistance);

	/// Check..
	int waypointsPost = navMesh->waypoints;
	int waypointsCreated = waypointsPost - waypointsPre;
	std::cout<<"\nWaypoints: "<<waypointsPre<<" post:"<<waypointsPost;
	assert(waypointsPost > waypointsPre);	
	std::cout<<"\nWps created: "<<waypointsCreated;
	/// Release it upon finishing
	WaypointMan.ReleaseActiveNavMeshMutex();
	return waypointsCreated;
}
/// For re-sizing.
Vector2i TileGrid2D::Size(){
	return size;
}
void TileGrid2D::Resize(Vector2i newSize){
	std::cout<<"\nTileGrid2D::Resize";
	Deallocate();
	size = newSize;
	for (int y = 0; y < size.y; ++y){
		List<Tile*> * list = new List<Tile*>();
		for (int x = 0; x < size.x; ++x){
			Tile * tile = new Tile();
			tile->position = Vector3i(x,y,0);
			list->Add(tile);
		}
		tiles.Add(list);
	}
}
// Protected functions
void TileGrid2D::Deallocate(){
	for (int i = 0; i < tiles.Size(); ++i)
		tiles[i]->ClearAndDelete();
	tiles.ClearAndDelete();
}

/// Getter
Tile * TileGrid2D::GetTile(int x, int y)
{
//	std::cout<<"\nTileGrid2D::GetTile: GridSize: "<<size;
	if (x < 0 || x >= size.x ||
		y < 0 || y >= size.y)
		return NULL;
	assert(x >= 0 && x < size.x);
	assert(y >= 0 && y < size.y);
	Tile * tile = (*tiles[y])[x];
	return tile;
}

/// Returns tile at target position or NULL if no existy.
Tile * TileGrid2D::GetTile(Vector2i position)
{
	return GetTile(position.x, position.y);
}


/// Rendering! Called from render-thread onry
void TileGrid2D::Render(){
	
		
		
	/// 
	Vector3f camPos = graphicsState.camera->Position();
	Camera & camera = *graphicsState.camera;
	Frustum frustum = camera.GetFrustum();
	Vector3f min = frustum.hitherBottomLeft - Vector3f(1,1,1), max = frustum.fartherTopRight + Vector3f(1,1,1);
	int tilesToRender = (int) ((max.x - min.x) * (max.y - max.y));
	if (tilesToRender > 1000)
		return;

	// Begin rendering.
	FOR_TILE_START
	
	
		/// Tile * tile declared via macro.
		float xPos = float(x);
		float yPos = float(y);		
		if (xPos < min.x || xPos > max.x ||
			yPos < min.y|| yPos > max.y)
			continue;

		TileType * tt = tile->type;
		if (tt == NULL){
			// glColor4f(xPos / size.x, yPos / size.y, 0.5, 1);
		}
		else if (tt->textureSource.Length() < 3){
			Vector3f c = tt->color;
			// glColor4f(c.x, c.y, c.z, 1.f);
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
		if (x < camPos.x - viewRange || x > camPos.x + viewRange)
			continue;
		for (int y = bottom; y < top; ++y){
			if (y < camPos.y - viewRange || y > camPos.y + viewRange)
				continue;
	
		}
	}*/
}
