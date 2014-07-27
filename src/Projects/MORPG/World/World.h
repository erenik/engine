/// Emil Hedemalm
/// 2014-07-27
/// Class encompassing an entire world.

#ifndef WORLD_H
#define WORLD_H

#include <fstream>
#include "String/AEString.h"
#include "MathLib.h"

class Texture;
class Model;
class Zone;
class Character;
class Nation;
class Quest;

class World 
{
public:
	World();

	/// Should there be a single name for it. Most often there is.
	String name;
	/// In x and y, tiles/zones/grid.
	Vector2i size;

	/// When ready to load or generate. True at start.
	bool empty;

	/// Deletes all contents in this world. Makes it ready for loading again.
	void Delete();

	/// Saves this world to target file. Will save all zones, characters and quests to the same file.
	bool WriteTo(std::fstream & file);
	/// Loads from target file. Will load all zones, characters and quests from the same file.
	bool ReadFrom(std::fstream & file);

	/// Based on zones.
	void GeneratePreviewTexture();
	/// Based on zones.
	void GenerateWorldModel();

	/// Getters.
	Nation * GetNationByID(int id);
	
	/// Preview texture for this world (pretty much the world-map?)
	Texture * texture;
	/// Dynamically created model. Should/could be used together with the preview texture! :)
	Model * model;

	/// For regional conflicts.
	List<Nation*> nations;
	/// All zones.
	List<Zone*> zones;
	/// All characters in the world.
	List<Character*> characters;
	/// All quests in the world.
	List<Quest*> quests;
};

/// There is but one world... define it in World.cpp preferably.
extern World world;

#endif

