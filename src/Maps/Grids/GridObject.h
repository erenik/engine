/// Emil Hedemalm
/// 2014-02-25
/// An object that can be placed on a Grid. Contains references to all tiles which it occupies or interact with in some way.

#ifndef GRID_OBJECT_H
#define GRID_OBJECT_H

#include "MathLib.h"
#include "Tile.h"

class Entity;

/** Grid Object Type
	Basic information about the grid object, shared by all version of the specified grid object.
	The grid object assumes you are working with 2D square sprites on a 2D map of some sort.
*/
class GridObjectType {
	friend class GridObjectTypeManager;
public:
	GridObjectType();
	~GridObjectType();

	/// Resizes the passability matrix as needed.
	void UpdatePassabilityMatrix();
	/// Updates pivot-position depending on which tiles are marked as passable or not!
	void UpdatePivotPosition();
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);
	/// Name, yo. Unique needed!
	String name;
	/// Size in X and Y.
	Vector2i size;
	/// Texture, yus.
	Texture * texture;
	String textureSource;
	/// Matrix with info of which tiles should be marked as passable or deflagged when painting this object type on the map/grid.
	List<bool*> passability;
	/// Offset to current mouse position in tiles. Used when rendering and placing it on the map.
	Vector2f pivotPosition;
	
	/// Updates the 4 below. Call after changing pivot position or size.
	void UpdatePivotDistances();
	/// Distances from pivot to the edges, to be used for rendering etc.
	float pivotToLeft;
	float pivotToRight;
	float pivotToTop;
	float pivotToBottom;
	int ID(){return id;};
private:
	/// Current matrix size.
	Vector2i matrixSize;
	/// Unique ID, handled by the manager
	int id;
};

/** An object that can be placed on a Grid. Contains references to all tiles which it occupies or interact with in some way.
	The grid object assumes you are working with 2D square sprites on a 2D map of some sort.
*/
class GridObject {
public:
	GridObject(GridObjectType * type = NULL);
	~GridObject();
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);
	
	/// Name of zis object.
	String name;
	/// Entity that is related to this grid-object.
	Entity * entity;
	/// Type of this object.
	GridObjectType * type;
	/// Name of the GridObjectType.
	String typeName;
	/// Position on the grid.
	Vector3f position;
	/// Tiles which this object is currently attached to.
	List<Tile*> attachedTiles;
};

#define GridObjectTypeMan (*GridObjectTypeManager::Instance())

class GridObjectTypeManager {
private:
	GridObjectTypeManager();
	~GridObjectTypeManager();
	static GridObjectTypeManager * gridObjectTypeManager;
public:
	static void Allocate();
	static void Deallocate();
	static GridObjectTypeManager * Instance();

	/// Creates a new grid object type, inserts it to the list of available object types and returns it.
	GridObjectType * New();
	/// Sets path to the file to which we save/laod.
	void SetSavePath(String path);
	/// Saves all types.
	bool Save();
	bool Load();
	GridObjectType * GetTypeByID(int id);
	GridObjectType * GetType(String byName);
	List<GridObjectType*> GetTypes() {return objectTypes;};
private:

	void AssignID(GridObjectType* toType);

	List<GridObjectType*> objectTypes;
	String savePath;
};

#endif
