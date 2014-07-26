// Emil Hedemalm
// 2013-06-28

#ifndef TILE_MAP2D_H
#define TILE_MAP2D_H

#include "Maps/Map.h"
#include "Maps/Grids/Tile.h"

struct GraphicsState;
class TileMapLevel;
class EntityStateTile2D;
class GridObject;
class GridObjectType;

/// A standard 2D-map based on tiles, including ground, terrain, and objects.
class TileMap2D : public Map
{
	friend class MapManager;
	TileMap2D();
	virtual ~TileMap2D();
public:
	/// Resets all tiles, such as pointers to entities, etc.
	void ResetTiles();

	/// Evaluates
	virtual void OnEnter();	// Called once when entering the map
	virtual void OnExit();	// Called once when exiting the map

	/// Render!
	void Render(GraphicsState * graphicsState);

	/// Loads map data from file.
	virtual bool Load(const char * fromFile);
	/// Saves map data to file.
	virtual bool Save(const char * toFile);
	
	/// Returns amount of tiles in the map (x * y)
//	int GetSize();
//	int SizeX() const { return xSize; };
//	int SizeY() const { return ySize; };
	
	/// Fetch tile.
	Tile * GetTile(int x, int y);
	// Fetch tile by position. Z-parameter of position meaning elevation or level.
	Tile * GetTile(Vector3i position) const;
	/// Adjusting map-size. NOTE that this will discard entities and tiles outside of it's perimeter (if any)
	void SetSize(int x, int y);
	/// Considered for deprecation.
	void SetSize(int left, int right, int top, int bottom);
	/// Expands along each axis as specified.
	void Expand(int left, int right, int top, int bottom);
	void SetViewRange(int i) { viewRange = i;};
	
	// Why do you exist?
	void SetTileType(Vector2i position, TileType * t);
	/// Checks if target tile is vacant and walkable.
	bool IsTileVacant(Vector3i position);
	/// Moves ze entity on the grid! Returns true upon success.
	bool MoveEntity(Entity * e, Vector3i toPosition);
	/// To check for events when arriving at a specified tile.
//	void OnArrive(Entity * e, int x, int y);

	/// Returns a walkable tile close to given coordinates.
	Tile * GetClosestVacantTile(Vector3i position);
	/// Gives random tile types to all!
	void RandomizeTiles();

	/// Wosh. Tries OnInteract events at target tile, with the interacter as reference.
	void Interact(Vector3i position, const Entity * interacter);

	/// Attempts to fetch target entity's Tile2D equivalent/meta-structure.
	EntityStateTile2D * GetEntity2DByEntity(const Entity * entity);
	/// Attempts to fetch target EntityStateTile2D using given position.
	EntityStateTile2D * GetEntityByPosition(Vector3i position);
	/// Generates (allocates) waypoints for target navmesh. It also performs any connections within as appropriate.
	int GenerateWaypoints(NavMesh * navMesh);

	/// Fetches target level (by elevation)
	TileMapLevel * GetLevelByElevation(int elevation) const;
	TileMapLevel * GetLevelByIndex(int index) const;
	bool Allocated() const {return allocated;};
	TileMapLevel * ActiveLevel();

	/// Returns size of the active level of the map
	Vector2i Size();

	/** Removes target entity from the map. */
	virtual bool RemoveEntity(Entity * entity);

private:
	// Update the vectors that define the area which requires recalculation.	
	void ExpandUpdateToInclude(Vector2i position);
	/// Last time we did something with this map.
	long long lastUpdate;
	/** Vectors containing the rectangular area which has been modified since the last update. These will be reset to (-1,-1) after each update to lastUpdate.
		If an update size os -1,-1 the whole map is assumed to require re-calculating.
		Mainly used to calculate the preview texture for now.
	*/
	Vector2i updateMin, updateMax;
	/// Updates the preview texture as needed. To be called only from the graphics-thread!
	void UpdatePreviewTexture();
	/// A preview texture used to render faster at large zooms, or just rendering fast in general.
	Texture * previewTexture;

	/// Creates a new TileMapLevel with given size, returning it upon success. Added automatically to the levels list.
	TileMapLevel * 	CreateLevel(int sizeX, int sizeY, TileMapLevel * levelToBaseItOn);
	/// Deletes target level, removing it from the list of levels.
	void DeleteLevel(TileMapLevel * level);
	/// Specifies the currently active level. Should be one all the time.
	TileMapLevel * activeLevel;
	/// If allocated or not. 
	bool allocated;
	/// If currently rendering.
	bool rendering;
	/// If it should render
	bool render;
	/// Renders entities
	void RenderEntities(GraphicsState * graphicsState);
	/// Render symbolic quads for events.
	void RenderEvents(GraphicsState * graphicsState);

	/// For le adding! Has to be called at least once before using the 2D moving functions. 
	/// If positions are provided these will be attempted to be used.
	virtual bool AddEntity(Entity * e);

	/// Reads map stats (width/height/version)
	bool ReadStats(std::fstream &file);
	/// Reads map stats (width/height/version)
	bool WriteStats(std::fstream &file);
	/// Reads tile data block from file
	bool ReadTiles(std::fstream &file);
	/// Writes tile data block to file
	bool WriteTiles(std::fstream &file);
	/// Events
	bool ReadEvents(std::fstream &file);
	bool WriteEvents(std::fstream &file);

	// Size parameters
//	int xSize, ySize;
	// And other names for them..
//#define sizeX xSize
//#define sizeY ySize
//	int left, right, top, bottom;	
	int viewRange;

	void DeleteGrid();
	void AllocateGrid(int x, int y);


	/// For keeping track of collissions n shit.
//	Entity *** entityGrid;

	/// An individual level, contain X amount of tiles (a tileGrid)
	List<TileMapLevel*> levels;

	/// Document how it works, man..
//	Tile *** tileGrid;

	/// List of 2D-based entities!
	List<EntityStateTile2D*> entitiesTile2D;

	/// Returns a list of all tiles in the map.
	List<Tile*> GetTiles();
	/// Assigns tileTypes to all tiles.
	void AssignTileTypes();
	bool tileTypesAssignedToTiles;
	/// Inherited from Map, list of Entities o-o
///	List<Entity*> entities;
};

#endif