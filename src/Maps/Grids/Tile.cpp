// Emil Hedemalm
// 2013-06-28

#include "Tile.h"
#include "TileTypeManager.h"
#include <fstream>
#include "Script/Script.h"
#include "Maps/Grids/GridObject.h"

List<Tile*> Tile::tiles;

Tile::Tile()
: type(NULL), position(-1,-1,-1), event(NULL)
{ 
	tiles.Add(this); 
};

Tile::~Tile(){ 
	tiles.Remove(this); 
};


// Versions
#define TILE_VERSION_0 0// Initial version.
int tileVersion = TILE_VERSION_0;

/// Writes to file stream.
void Tile::WriteTo(std::fstream & file)
{
	// Write version
	file.write((char*)&tileVersion, sizeof(int));
	// Fetch type index.
	this->typeIndex = -1;
	if (type)
		typeIndex = type->type;
	// Write type index
	file.write((char*)&typeIndex, sizeof(int));
	// Write position.
	position.WriteTo(file);
	// Write stuff bound to the tile.
	int stuff = 0;
	if (event)
		stuff |= 0x001;
	file.write((char*)&stuff, sizeof(int));
	// Write the path to the source of the event.
	if (event)
		event->source.WriteTo(file);
	// Write description of additional details to file.
	description.WriteTo(file);
}

/// Reads from file stream.
void Tile::ReadFrom(std::fstream & file){
	// Read version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(tileVersion == version);
	// Read type
	file.read((char*)&typeIndex, sizeof(int));
	// Read position.
	position.ReadFrom(file);
	
	// Write stuff bound to the tile.
	int stuff = 0;
	file.read((char*)&stuff, sizeof(int));
	// If got event
	if (stuff & 0x001){
		// Load event source
		event = new Script();
		event->source.ReadFrom(file);
		// Load it straight away, or...?
	}
	// Write description of additional details to file.
	description.WriteTo(file);
}


/// Assigns all tiles an index that is related to their type.
void Tile::EnsureIndices(){
	for (int i = 0; i < tiles.Size(); ++i){
		Tile * t = tiles[i];
		t->typeIndex = TileTypes.Index(t->type);
	}
}

/// Saves indices for the type and nullifies the pointer.
void Tile::PrepareForReload(){
	for (int i = 0; i < tiles.Size(); ++i){
		Tile * t = tiles[i];
		t->typeIndex = TileTypes.Index(t->type);
		t->type = NULL;
	}
}
/// Re-directs the type pointer to the newly loaded tile-types.
void Tile::Reload(){
	for (int i = 0; i < tiles.Size(); ++i){
		Tile * t = tiles[i];
		t->type = TileTypes.GetTileTypeByIndex(t->typeIndex);
	}
}

TileType::TileType()
: type(NULL), texture(NULL){
}

TileType::TileType(int type, String name, Vector3f color, Texture * t)
: type(type), name(name), color(color), texture(t){
}


/// Checks where the tile is walkable, but also if there already exists an entity on this tile, etc. Events may be present, however.
bool Tile::IsVacant()
{
//	std::cout<<"\nTile::IsVacant called";
	if (entities.Size() || !type)
		return false;
	if (!type->walkability)
		return false;
	if (objects.Size())
		return false;
	return true;
}