// Emil Hedemalm
// 2013-06-28

#include <cstring>

#include "TileMapLevel.h"
#include "TileMap2D.h"
#include "EntityStateTile2D.h"
#include "Maps/Grids/TileTypeManager.h"
#include "Maps/Grids/GridObject.h"
#include "Maps/2D/TileMapLevel.h"
#include "Entity/Entity.h"
#include "Texture.h"
#include "TextureManager.h"
#include "Graphics/GraphicsManager.h"
#include <fstream>
#include "Event/Event.h"
#include "Event/EventProperty.h"
#include "Event/EventManager.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"
#include "../MapManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Pathfinding/NavMesh.h"
#include "Pathfinding/Waypoint.h"
#include "Graphics/GraphicsProperty.h"
#include "Model.h"
#include "OS/Sleep.h"
#include <cassert>


TileMap2D::TileMap2D(){
	/// For keeping track of collissions n shit.
	viewRange = 100;
	mapType = TILE_MAP_2D;
	allocated = false;
	activeLevel = NULL;
	tileTypesAssignedToTiles = false;
	render = true;
	/// A preview texture used to render faster at large zooms, or just rendering fast in general.
	previewTexture = NULL;
	/// Last time we did something with this map.
	lastUpdate = Timer::GetCurrentTimeMs();
}
TileMap2D::~TileMap2D(){
	entities.Clear();
	levels.ClearAndDelete();
}

/// Fetches target level (by elevation)
TileMapLevel * TileMap2D::GetLevelByElevation(int elevation) const{
	for (int i = 0; i < levels.Size(); ++i){
		TileMapLevel * level = levels[i];
		if (level->Elevation() == elevation)
			return level;
	}
	return NULL;
}
TileMapLevel * TileMap2D::GetLevelByIndex(int index) const{
	assert(index >= 0 && index < levels.Size());
	return levels[index];
}

/// Fetch tile.
Tile * TileMap2D::GetTile(int x, int y)
{
	TileMapLevel * level = GetLevelByElevation(0);
	if (!level)
		return NULL;
	assert(level);
	return level->GetTile(x,y);
}

// Fetch tile by position. Z-parameter of position meaning elevation or level.
Tile * TileMap2D::GetTile(Vector3i position) const{
	TileMapLevel * level = GetLevelByElevation(position.z);
	assert(level);
	return level->GetTile(position);
}

/// Evaluates
void TileMap2D::OnEnter(){
	std::cout<<"\nTileMap2D::OnEnter map: "<<name<<" ";
	Graphics.QueueMessage(new GMSet(ACTIVE_2D_MAP_TO_RENDER, this));
	std::cout<<"\nTileMap2D::OnEnter ended? ";
	/// Let default map load all events and stuff though!
	Map::OnEnter();
	std::cout<<"\nTileMap2D::OnEnter ended? ";
}
void TileMap2D::OnExit(){
	std::cout<<"\nTileMap2D::OnExit: "<<name<<" ";
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_ACTIVE_2D_MAP));
	/// Let default map load all events and stuff though!
	Map::OnExit();
}


#define PRINT_ERROR	std::cout<<"\nGLError in Render "<<error;

/// Render!
void TileMap2D::Render(GraphicsState & graphicsState){
	if (!render)
		return;
	rendering = true;

//	std::cout<<"\nTileMap2D::Render: "<<name;
	if (!activeLevel){
		std::cout<<"\nTileMap2D::Render: No active level. Returning.";
		rendering = false;
		return;
	}
	GLuint error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError before TileMap2D::Render "<<error;
	}

	Graphics.SetShaderProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(graphicsState.projectionMatrixD.getPointer());
	glMatrixMode(GL_MODELVIEW);
	Matrix4d modelView = graphicsState.viewMatrixD * graphicsState.modelMatrixD;
	glLoadMatrixd(modelView.getPointer());

	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in TileMap2D::Render "<<error;
	}
	// Enable blending
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState.currentTexture = NULL;
	// Disable lighting
	glDisable(GL_LIGHTING);
	error = glGetError();
	if (error != GL_NO_ERROR){
		PRINT_ERROR
	}
	glDisable(GL_COLOR_MATERIAL);

	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in TileMap2D::Render "<<error;
	}

	// Specifies how the red, green, blue and alpha source blending factors are computed
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	error = glGetError();
	if (error != GL_NO_ERROR){
		PRINT_ERROR
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0101);
	glLineWidth(1.0f);

	Vector3f camPos = graphicsState.camera->Position();

	
	/// Render whole texture map?
	{
	    glColor4f(1,1,1,1);
		/// Generate preview texture as needed.
		UpdatePreviewTexture();
		if (previewTexture)
		{
			// Texture enabled.
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, previewTexture->glid);
		}
	}

	Vector2i mapSize = this->Size();

	bool renderPreviewTexture = true;
	if (renderPreviewTexture)
	{
		z = -0.01f;
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.f);
			glVertex3f(-0.5f, 0, z);
			glTexCoord2f(0.0f, 1.f);
			glVertex3f(-0.5f, mapSize.y-0.5f, z);
			glTexCoord2f(1.f, 1.f);
			glVertex3f(mapSize.x-0.5f, mapSize.y-0.5f, z);
			glTexCoord2f(1.f, 0.f);
			glVertex3f(mapSize.x-0.5f, 0-0.5f, z);
		glEnd();
	}

	/// If stuff, render preview map
	if (graphicsState.camera->zoom > 20)
	{
		return;
	}


	/// Render the file-grid levels
	int elevationToRender = 0;
	activeLevel->Render(graphicsState);
	for (int i = 0; i < levels.Size(); ++i){
		TileMapLevel * level = levels[i];
		if (elevationToRender != level->Elevation())
			continue;
		level->Render(graphicsState);
	}

	/// Stuff needed to cull decently.
	Camera & camera = *graphicsState.camera;
	Frustum frustum = camera.GetFrustum();
	Vector3f min = frustum.hitherBottomLeft - Vector3f(1,1,1), max = frustum.fartherTopRight + Vector3f(1,1,1);	
	/// Render objects
	for (int i = 0; i < activeLevel->objects.Size(); ++i)
	{
		// Let's just hope it's sorted.. lol
		GridObject * go = activeLevel->objects[i];
		
		/// Fetch it's position and texture, assume the anchor point is correct and just paint it.
		Vector3f position = go->position;
		/// Skip all out of sight.		
		if (position.x < min.x || position.x > max.x ||
			position.y < min.y|| position.y > max.y)
			continue;


		GridObjectType * got = go->type;
		if (!got){
			// Fetch got if possible
			got = GridObjectTypeMan.GetType(go->typeName);
			if (!got)
				continue;
			go->type = got;
		}
		float left = position.x + got->pivotToLeft,
			right = position.x + got->pivotToRight,
			top = position.y + got->pivotToTop,
			bottom = position.y + got->pivotToBottom;
		float z = 0;
		
		glColor4f(1,1,1,1);
		glEnable(GL_TEXTURE_2D);
		if (!got->texture){
			got->texture = TexMan.GetTexture(got->textureSource);
		}
		if (got->texture){
			if (got->texture->glid == -1)
				got->texture->Bufferize();
			glBindTexture(GL_TEXTURE_2D, got->texture->glid);
		}
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex3f(left, bottom, z);
			glTexCoord2f(1,0);
			glVertex3f(right, bottom, z);
			glTexCoord2f(1,1);
			glVertex3f(right, top, z);
			glTexCoord2f(0,1);
			glVertex3f(left, top, z);
		glEnd();

	}


	/// Wosh
	RenderEntities(graphicsState);
	// Render le vevents!
	RenderEvents(graphicsState);


	// Throw into TileMapLevel-> Draw?
	Vector2i levelSize = activeLevel->Size();
	float xSize = (float)levelSize.x;
	float ySize = (float)levelSize.y;
	// Draw a quad around it all, yo.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
		glVertex3f(-0.5f, -0.5f,0);
		glVertex3f(-0.5f, ySize-0.5f,		0);
		glVertex3f(xSize-0.5f, ySize-0.5f,0);
		glVertex3f(xSize-0.5f, -0.5f,		0);
	glEnd();


	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGLError in TileMap2D::Render "<<error;
	}
	rendering = false;
}

void TileMap2D::RenderEntities(GraphicsState & graphicsState){
	/// Might wanna replace the following code with one that sorts the entities depending on Y
	Vector3f camPos = graphicsState.camera->Position();
	GLuint error;

	/// Sort entities by Y

	glDisable(GL_DEPTH_TEST);

	int entitiesToRender = entitiesTile2D.Size(), entitiesRendered = 0;
	// Prepare for rendering.
	for (int i = 0; i < entitiesTile2D.Size(); ++i){
		EntityStateTile2D * entityTile2D = entitiesTile2D[i];
		/// Occlude
		Vector3i & pos = entityTile2D->position;
		if (pos.x < camPos.x - viewRange || pos.x > camPos.x + viewRange)
			continue;
		if (pos.y < camPos.y - viewRange || pos.y > camPos.y + viewRange)
			continue;

		++entitiesRendered;
		/// Render
		float xPos = (float)pos.x;
		float yPos = (float)pos.y;
		// Draw entities too, yo.
		Entity * e = entityTile2D->entity;
	//	std::cout<<"\nRendering entity "<<i<<": "<<e->name;

		float z = 0.1f;
		//	glBlendFunc(GL_ONE, GL_ONE);
		Graphics.SetShaderProgram("Flat");
		/// Load in matrices, yawow!
		glUniformMatrix4fv(graphicsState.activeShader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
		glUniformMatrix4fv(graphicsState.activeShader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
		glColor4f(1,1,1,1.0f);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		//	glDisable(GL_DEPTH_TEST);
//		assert(false && "TODO: Create Render-functions for EntityStateTile2D for simplified and more customized rendering. \
//			Matrices aren't properly updated at the moment, what with the new Vector2i position in.");
		entityTile2D->Render(graphicsState);
//		e->render(graphicsState);
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//	glEnable(GL_DEPTH_TEST);

		bool renderEntityOverlay = true;
		if (renderEntityOverlay){
			glBlendFunc(GL_ONE, GL_ONE);
			Graphics.SetShaderProgram(0);
			glColor4f(0.2f,0.2f,0.2f,0.2f);
			glBegin(GL_QUADS);
			glVertex3f(xPos-0.5f, yPos-0.5f, 0.1f);
			glVertex3f(xPos-0.5f, yPos+0.5f, 0.1f);
			glVertex3f(xPos+0.5f, yPos+0.5f, 0.1f);
			glVertex3f(xPos+0.5f, yPos-0.5f, 0.1f);
			glEnd();
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGLError in TileMap2D::Render "<<error;
		}
	}
	glEnable(GL_DEPTH_TEST);
//	std::cout<<"\nEntities to render: "<<entitiesToRender<<" entitiesRendered: "<<entitiesRendered;
}

/// Render symbolic quads for events.
void TileMap2D::RenderEvents(GraphicsState & graphicsState)
{
	Texture * tex = TexMan.GetTextureBySource("img/Events/DefaultEvent.png");
	if (tex->glid == -1){
		 TexMan.BufferizeTexture(tex);
	}
	assert(tex);
	glBindTexture(GL_TEXTURE_2D, tex->glid);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1,1,1,1);
	float eventZ = 5.0f;
	for (int i = 0; i < events.Size(); ++i){
		Event * event = events[i];
		event->position;
		float xPos = event->position.x;
		float yPos = event->position.y;
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0);
			glVertex3f(xPos-0.5f, yPos-0.5f, eventZ);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xPos-0.5f, yPos+0.5f, eventZ);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xPos+0.5f, yPos+0.5f, eventZ);
			glTexCoord2f(1.0f, 0);
			glVertex3f(xPos+0.5f, yPos-0.5f, eventZ);
		glEnd();


	}
}

/// Returns a list of all tiles in the map.
List<Tile*> TileMap2D::GetTiles(){
	List<Tile*> tiles;
	for (int i = 0; i < levels.Size(); ++i){
		TileMapLevel * level = levels[i];
		List<Tile*> moreTiles = level->GetTiles();
		tiles += moreTiles;
	}
	return tiles;
}

/// Assigns tileTypes to all tiles.
void TileMap2D::AssignTileTypes(){
	List<Tile*> tiles = GetTiles();
	for (int i = 0; i < tiles.Size(); ++i){
		Tile * tile = tiles[i];
		tile->type = TileTypes.GetTileTypeByIndex(tile->typeIndex);
	}
	tileTypesAssignedToTiles = true;
}

// Map version
#define MAP_VERSION_1		0x00000002	/// Removed the "size"-identifier for all blocks, since it's just in the way.
#define MAP_VERSION_2		0x00000003	/// Re-adjusted a lot, for example adding TileMapLevel and enabling elevation via Vector3i
#define CURRENT_MAP_VERSION	MAP_VERSION_2


// Blocks when saving/loading!
#define BLOCK_MAP_STATS		0x00000001
#define BLOCK_TILES			0x00000002
#define BLOCK_TERRAIN		0x00000004
#define BLOCK_EVENTS		0x00000008
#define BLOCK_END_OF_FILE	0x00010000

/// Loads map data from file.
bool TileMap2D::Load(const char * fromFile){
	std::cout<<"\nLoading map from file: "<<fromFile;
	std::fstream file;
	file.open(fromFile, std::ios_base::in | std::ios_base::binary);
	/// If failed to open, close and return false.
	if (!file.is_open()){
		file.close();
		std::cout<<"\nERROR: Unable to open filestream to "<<fromFile;
		return false;
	}
	/// Read file header.
	file.read((char*)&header, sizeof(MapFileHeader));
	std::cout<<"\nMap version: "<<header.version;
	int mapVersion = header.version;
	std::cout<<"\nMap version: "<<mapVersion;

	/// Check good version
	if (mapVersion > CURRENT_MAP_VERSION){
		std::cout<<"\nERROR: Unsupported map version. Faulty map file?";
		return false;
	}


	// Delete any entities and events we might have
	events.ClearAndDelete();
	RemoveAllEntities();

	if (header.version <= MAP_VERSION_1){
		bool endOfFile = false;
		while (!file.eof() && !endOfFile){
			/// Read next integer to inrepret type of next data block
			int type;
			int blockSize;
			file.read((char*) &type, sizeof(int));

			/// If we're on the previous version, read in the block size. It's skipped in newer versions :)
			if (mapVersion < MAP_VERSION_1)
				file.read((char*) &blockSize, sizeof(int));

			// Switch depending on block-type.
			switch(type){
				case BLOCK_MAP_STATS:
					ReadStats(file);
					break;
				case BLOCK_TILES:
					ReadTiles(file);
					break;
				case BLOCK_EVENTS:
					ReadEvents(file);
					break;
				case BLOCK_END_OF_FILE:
					endOfFile = true;
					break;
				default:
					std::cout<<"\nUnknown block type: "<<type;
					std::cout<<"\nAborting loading procedure because of unknown data in stream.";
					file.close();
					return false;
			}
		}
	}
	else if (header.version == MAP_VERSION_2){
		/// Load optional texture that represents this map.
		bool hasTexture = false;
		file.read((char*)&hasTexture, sizeof(bool));

		/// Load lighting scheme
		this->lighting.ReadFrom(file);

		/// Load levels
		int numLevels = 0;
		file.read((char*)&numLevels, sizeof(int));
		for (int i = 0; i < numLevels; ++i){
			TileMapLevel * level = new TileMapLevel();
			level->ReadFrom(file);
			levels.Add(level);
			this->activeLevel = level;
		}

		AssignTileTypes();
		assert(tileTypesAssignedToTiles);
		// Set tiles as blerghphf.
	//	Tile::Reload();
	}

	/// Close file ^^
	file.close();

	mapDataOK = true;
	std::cout<<"\nFile successfully read.";
	return true;
}

/// Saves map data to file.
bool TileMap2D::Save(const char * toFile){
	std::cout<<"\nSaving map to file: "<<toFile;
	std::fstream file;
	file.open(toFile, std::ios_base::out | std::ios_base::binary);
	/// If failed to open, close and return false.
	if (!file.is_open()){
		file.close();
		std::cout<<"\nERROR: Unable to open filestream to "<<toFile;
		return false;
	}

	/// Set version to latest
	header.version = CURRENT_MAP_VERSION;
	header.dateCreated = (long)time(0);	// Get current time
	strcpy(header.mapName, name);	// Set header map name to the one we're using in the editor

	int size = sizeof(MapFileHeader);	// Should be int(4) + long(4) + 260 chars
	file.write((char*)&header, sizeof(MapFileHeader));

	/// Previous writing procedure...
	if (header.version == MAP_VERSION_1){
		assert(false);
		/*
		/// Write map stats first of all!
		WriteStats(file);

		/// Block types for writing
		int blockType = -1;
		// Write map data if valid size.
		if (sizeX > 0 && sizeY > 0){
			WriteTiles(file);
		}
		if (events.Size()){
			WriteEvents(file);
		}
		*/
	}
	else if (header.version == MAP_VERSION_2){


		/// Save optional texture that represents this map.
		bool hasTexture = false;
		file.write((char*)&hasTexture, sizeof(bool));

		/// Save lighting scheme
		this->lighting.WriteTo(file);

		/// Save levels
		int numLevels = levels.Size();
		file.write((char*)&numLevels, sizeof(int));
		for (int i = 0; i < levels.Size(); ++i){
			levels[i]->WriteTo(file);
		}
		// Make one of the levels active?
		/// Write levels!
	//	assert(false && "Implement");
	}

	/// Write end of file!
	int end = BLOCK_END_OF_FILE;
	file.write((char*)&end, sizeof(int));
	int endNull = 0;
	file.write((char*)&endNull, sizeof(int));
	file.close();
	return true;
}



// Only contains width and height of the map!
#define MAP_STATS_VERSION_1		0x00000001
#define MAP_STATS_VERSION_2		0x00000002		/// Added with levels, which deprecates this but tries to remain compatible
#define ACTIVE_MAP_STAT_VERSION	MAP_STATS_VERSION_1

/// Reads map stats (width/height/version)
bool TileMap2D::ReadStats(std::fstream &file){
	std::cout<<"\nMap version: "<<header.version;
	int v = MAP_STATS_VERSION_1;
	if (header.version == MAP_STATS_VERSION_1 ){
		int version;
		int mapSizeX, mapSizeY;
		file.read((char*) &version, sizeof(int));
		file.read((char*) &mapSizeX, sizeof(int));
		file.read((char*) &mapSizeY, sizeof(int));
		/// Create the map based on this?
		SetSize(mapSizeX, mapSizeY);
		return true;
	}
/** Old reader
	// Write the version of this block.
	int version;
	file.read((char*) &version, sizeof(int));
	// Read sizes..!
	int mapSizeX, mapSizeY;
	file.read((char*) &mapSizeX, sizeof(int));
	file.read((char*) &mapSizeY, sizeof(int));

	// Resize map straight away?
	SetSize(mapSizeX, mapSizeY);
*/
	assert(false && "Bad version in TileMap2D::ReadStats! Deprecated btw.");
	return false;
}
/// Reads map stats (width/height/version)
bool TileMap2D::WriteStats(std::fstream &file){
	// First write what block we're beginning
	int blockType = BLOCK_MAP_STATS;
	file.write((char*) &blockType, sizeof(int));
	// Write the version of this block.
	int version = ACTIVE_MAP_STAT_VERSION;
	file.write((char*) &version, sizeof(int));
	// New! Write amount of levels just, lol.
	assert(false && "Deprecated?");
	/* // Old below
	// Write sizes..!
	file.write((char*) &xSize, sizeof(int));
	file.write((char*) &ySize, sizeof(int));
	*/
	return true;
}


#define TILE_BLOCK_VERSION_1	0x00000001  // Initial version
#define TILE_BLOCK_VERSION_2	0x00000002	/// This version added various levels (elevations) and re-structured usage to more files.
#define ACTIVE_BLOCK_VERSION	TILE_BLOCK_VERSION_2

/// Reads active entity data block from file
bool TileMap2D::ReadTiles(std::fstream &file){
	// Write the version of this block.
	int version;
	file.read((char*) &version, sizeof(int));


	if (version == TILE_BLOCK_VERSION_1){
		// Read amount of tiles to read in.
		int tiles;
		file.read((char*) &tiles, sizeof(int));
		assert(activeLevel);
		Vector2i size = activeLevel->Size();
		// Read tiles..
		for (int x = 0; x < size.x; ++x){
			for (int y = 0; y < size.y; ++y){
				/// Read type for the tile at given index.
				Tile * tile = activeLevel->GetTile(Vector2i(x,y));
				file.read((char*) &tile->typeIndex, sizeof(int));
				tile->type = NULL;
			//	tileGrid[x][y]->typeIndex
			//	tileGrid[x][y]->type = NULL;
			}
		}
	}
	else if (version == TILE_BLOCK_VERSION_2){
		assert(false && "Deprecate");
	}

	AssignTileTypes();
	assert(tileTypesAssignedToTiles);

	// Set tiles as blerghphf.
	Tile::Reload();
	return true;
}

/// Writes entity data block to file
bool TileMap2D::WriteTiles(std::fstream &file){
	// Write block-type.
	int blockType = BLOCK_TILES;
	file.write((char*) &blockType, sizeof(int));

	// Write the version of this block.
	int version = ACTIVE_BLOCK_VERSION;
	file.write((char*) &version, sizeof(int));
	assert(false && "Implement");
	// Version 1 below:
	/*
		// Write amount of tiles to save.
		int tiles = sizeX * sizeY;
		file.write((char*) &tiles, sizeof(int));

		Tile::EnsureIndices();
		// Write tiles..
		for (int x = 0; x < sizeX; ++x){
			for (int y = 0; y < sizeY; ++y){
				file.write((char*) &tileGrid[x][y]->typeIndex, sizeof(int));
			}
		}
	*/
	return true;
}

/// Event block version
#define EVENT_BLOCK_VERSION_1	0x00000001
#define ACTIVE_EVENT_BLOCK_VERSION	EVENT_BLOCK_VERSION_1

/// Events
bool TileMap2D::ReadEvents(std::fstream &file){
	if(events.Size() != 0){
		std::cout<<"\nEvents not null! Clearing and deleting the current list! o.o";
		events.ClearAndDelete();
	}

	// Write the version of this block.
	int version;
	file.read((char*) &version, sizeof(int));
	assert(version == ACTIVE_EVENT_BLOCK_VERSION);
	// Read amount of events to read in.
	int numEvents;
	file.read((char*) &numEvents, sizeof(int));
	std::cout<<"\nEvents to read: "<<numEvents;

	// Read tiles..
	for (int i = 0; i < numEvents; ++i){
		Event * event = new Event();
		event->ReadFrom(file);
		events.Add(event);
	}
	return true;
}
bool TileMap2D::WriteEvents(std::fstream &file){
	// Write block-type.
	int blockType = BLOCK_EVENTS;
	file.write((char*) &blockType, sizeof(int));

	// Write the version of this block.
	int version = ACTIVE_EVENT_BLOCK_VERSION;
	file.write((char*) &version, sizeof(int));
	std::cout<<"\nEvent block version: "<<ACTIVE_EVENT_BLOCK_VERSION;

	// Write total block size.
	int numEvents = events.Size();
	file.write((char*) &numEvents, sizeof(int));
	std::cout<<"\nEvents to write: "<<numEvents;


	// Write events to file..!
	for (int i = 0; i < numEvents; ++i){
		Event * event = events[i];
		event->WriteTo(file);
	}
	return true;
}

/// Adjusting map-size. NOTE that this will discard entities and tiles outside of it's perimeter (if any)
void TileMap2D::SetSize(int x, int y){
	std::cout<<"\nTileMap2D::SetSize(int,int)";
	/// Save old level
	TileMapLevel * oldLevel = activeLevel;
	/// Create new one
	activeLevel = CreateLevel(x,y,oldLevel);
	/// Delete the old level
	if (oldLevel)
		DeleteLevel(oldLevel);
	///
	this->lastUpdate = Timer::GetCurrentTimeMs();
	/// Generate new preview.
}
void TileMap2D::SetSize(int left, int right, int top, int bottom){
	assert(false && "Deprecate?");
	/*
	std::cout<<"\nTileMap2D::SetSize(int,int,int,int)";
	DeleteGrid();
	xSize = right - left;
	ySize = top - bottom;
	AllocateGrid(xSize, ySize);
	this->left = left;
	this->right = right;
	this->top = top;
	this->bottom = bottom;
	*/
}

void TileMap2D::DeleteGrid(){
	assert(allocated);
	entitiesTile2D.ClearAndDelete();

	Graphics.QueueMessage(new GraphicsMessage(GM_UNREGISTER_ALL_ENTITIES));
	allocated = false;
}
void TileMap2D::AllocateGrid(int newX, int newY){
	assert(!allocated);
	TileMapLevel * level = CreateLevel(newX,newY,0);
	levels.Add(level);
	activeLevel = level;
	allocated = true;
	std::cout<<"\nTileMap2D::AllocateGrid: Allocation done.";
}


/// Updates the preview texture as needed. To be called only from the graphics-thread!
void TileMap2D::UpdatePreviewTexture()
{
	if (previewTexture == 0){
		previewTexture = TexMan.NewDynamic();
	}
	if (previewTexture == 0)
		return;

	/// Change size as needed.
	Vector2i mapSize = Size();
	if (previewTexture->width != mapSize.x ||
		previewTexture->height != mapSize.y)
	{
		/// Rescale
		previewTexture->Resize(mapSize);
		previewTexture->lastUpdate = 0;
	}
	/// Check last update time.
	if (previewTexture->lastUpdate >= lastUpdate)
		return;	

	/// Update that it all?
	bool gotOneTexture = false;
	/// If still negative, set to entire map-size.
	if (updateMin.x == -1)
	{
		updateMin = Vector2i();
		updateMax = Size();
	}
	Vector2i sizeToUpdate = updateMax - updateMin;
	int tilesToUpdate = sizeToUpdate.x * sizeToUpdate.y;
	if (tilesToUpdate)
		std::cout<<"\nTiles to update: "<<tilesToUpdate;
	for (int x = updateMin.x; x < updateMax.x; ++x)
	{
		for (int y = updateMin.y; y < updateMax.y; ++y){
			Tile * tile = GetTile(x,y);
			assert(tile);
			
			Vector4f color;
			if (tile->type){
				gotOneTexture = true;
				if (tile->type->textureSource.Length()){
					if (tile->type->texture == NULL)
					{
						tile->type->texture = TexMan.GetTextureBySource(tile->type->textureSource);
					}
					if (tile->type->texture)
					{
						// Fetch it from the texture! o.o
						color = tile->type->texture->GetSampleColor();
					}
					else {
						std::cout<<"\nBlubb";
					}
				}
				else {
					color = tile->type->color;
				}
			}
			else if (tile->typeIndex > 0){
				return;
			}
			previewTexture->SetPixel(x,y,color);
		}
	}
	if (!gotOneTexture)
	{
		previewTexture->lastUpdate = 0;
		return;
	}
	previewTexture->Bufferize();
	updateMin = updateMax = Vector2i(-1,-1);
}

// Update the vectors that define the area which requires recalculation.	
void TileMap2D::ExpandUpdateToInclude(Vector2i position)
{
	/// If -1, we have no update since before.
	if (updateMin.x == -1){
		updateMin = updateMax = position;
		return;
	}
	if (position.x < updateMin.x)
		updateMin.x = position.x;
	else if (position.x > updateMax.x)
		updateMax.x = position.x;
	if (position.y < updateMin.y)
		updateMin.y = position.y;
	else if (position.y > updateMax.y)
		updateMax.y = position.y;
	/// Update time while at it, so no need to do that elsewhere.
	lastUpdate = Timer::GetCurrentTimeMs();
}

TileMapLevel * TileMap2D::CreateLevel(int sizeX, int sizeY, TileMapLevel * levelToBaseItOn){
	std::cout<<"\nTileMap2D::CreateLevel";
	TileMapLevel * level = new TileMapLevel(levelToBaseItOn);
	level->Resize(Vector2i(sizeX, sizeY));
	levels.Add(level);
	return level;
}

/// Deletes target level, removing it from the list of levels.
void TileMap2D::DeleteLevel(TileMapLevel * level){
	levels.Remove(level);
	/// TODO: Make sure rendering is not active for this level before deleting it?
	if (Graphics.RenderingEnabled()){
		render = false;
		while(Graphics.RenderingEnabled())
			Sleep(5);
	}
	delete level;
	render = true;
}


void TileMap2D::SetTileType(Vector2i position, TileType * t){
	assert(t);
	Tile * tile = activeLevel->GetTile(position);
	if (!tile)
		return;
	tile->type = t;
//	tileGrid[x][y]->type = t;
	// Update the vectors that define the area which requires recalculation.	
	ExpandUpdateToInclude(position);
}

/// Checks if target tile is vacant and walkable.
bool TileMap2D::IsTileVacant(Vector3i position){
	Tile * tile = activeLevel->GetTile(position);
	if (!tile)
		return false;
	if (!tile->type)
		return false;
	if (!tile->type->walkability)
		return false;
	/// Check entities thearr.
	EntityStateTile2D * entity = GetEntityByPosition(position);
	if (entity)
		return false;
	return true;
}

/// Moves ze entity on the grid! Returns true upon success.
bool TileMap2D::MoveEntity(Entity * entity, Vector3i position){
	std::cout<<"\nTileMap2D::MoveEntity";
	/// Get right level
	TileMapLevel * level = GetLevelByElevation(position.z);
	if (!level){
		std::cout<<"\nTileMap2D::MoveEntity: Bad Z-value. No such level exists.";
		return false;
	}
	Tile * tile = level->GetTile(position);
	if (!tile->IsVacant())
		return false;
	EntityStateTile2D * entityTile2D = GetEntity2DByEntity(entity);
	assert(entity && "Not registered, yo.");
	entityTile2D->position = position;
	entity->positionVector = Vector3f(position);
	entity->recalculateMatrix();
	/// Re-calculate matrix straight away, hm?
	return true;
}

/*
/// To check for events when arriving at a specified tile.
void TileMap2D::OnArrive(Entity * e, int x, int y){
	for (int i = 0; i < events.Size(); ++i){
		Event * event = events[i];
		if (event->triggerCondition != Event::ON_TOUCH)
			continue;
		if ((event->position - Vector3f(x,y,0)).LengthSquared() < 0.9f){
			EventMan.PlayEvent(event);
		}
	}
}*/

/// Returns a walkable tile close to given coordinates.
Tile * TileMap2D::GetClosestVacantTile(Vector3i position){
	TileMapLevel * level = GetLevelByElevation(position.z);
	float closestDistance = 5000000.0f;
	Tile * closest = NULL;
	Tile * t = level->GetClosestVacantTile(position);
	if (!t->IsVacant())
		return NULL;
	return t;
}

/// Gives random tile types to all!
void TileMap2D::RandomizeTiles(){
	TileMapLevel * level = ActiveLevel();
	Vector2i size = level->Size();
	for (int x = 0; x < size.x; ++x){
		for (int y = 0; y < size.y; ++y){
			Tile * t = activeLevel->GetTile(Vector2i(x,y));
			if (t->type == NULL)
				t->type = TileTypes.GetRandom();
		}
	}
}

/// For le adding!
bool TileMap2D::AddEntity(Entity * e){
	/// See if it's already been added.
	EntityStateTile2D * e2 = GetEntity2DByEntity(e);
	assert(!e2 && "Entity already added to map! Stahp! Use move or remove it first!");

	// Place it on ze grid too!
	Vector3i position = e->positionVector;
	/// Make sure it's a valid tile, if not abort the addition (and creation).
	while (GetEntityByPosition(position)){
		assert(false && "Bad location to place entity! Already occupied! Fetch a position first with GetWalkableTile()");
		Tile * tile = GetClosestVacantTile(position);
		assert(tile);
		position = tile->position;
		return false;
	}
	// Create it
	e2 = new EntityStateTile2D(e);
	e2->position = position;
	entitiesTile2D.Add(e2);
	Map::AddEntity(e);
	return true;
}

/** Removes target entity from the map. */
bool TileMap2D::RemoveEntity(Entity * entity){
	EntityStateTile2D * entityTile2D = GetEntity2DByEntity(entity);
	assert(entityTile2D && "Trying to remove entity that has already been removed.");
	bool result = entitiesTile2D.Remove(entityTile2D);
	assert(result);
	delete entityTile2D;
	Map::RemoveEntity(entity);
	return true;
}


void TileMap2D::Interact(Vector3i position, Entity * interacter){
	/// Interactor might be omitted if wished?
	assert(interacter);
	EntityStateTile2D * entityTile2D = GetEntity2DByEntity(interacter);
	/// Check for entities to interact with, or events too!
	EntityStateTile2D * interactee = GetEntityByPosition(position);
	if (!interactee)
		return;
	Entity * entity = interactee->entity;
	if (entity && entity->events && entity->events->onInteract){
		Event * onInteract = entity->events->onInteract;
		EventMan.PlayEvent(onInteract);
	}

}

/// Attempts to fetch target entity's Tile2D equivalent/meta-structure.
EntityStateTile2D * TileMap2D::GetEntity2DByEntity(Entity * entity){
	for (int i = 0; i < entitiesTile2D.Size(); ++i){
		if (entitiesTile2D[i]->entity == entity)
			return entitiesTile2D[i];
	}
	return NULL;
}

/// Attempts to fetch target EntityStateTile2D using given position.
EntityStateTile2D * TileMap2D::GetEntityByPosition(Vector3i position){
	for (int i = 0; i < entitiesTile2D.Size(); ++i){
		EntityStateTile2D * e = entitiesTile2D[i];
		if (e->position == position)
			return e;
	}
	return NULL;
}

/// Generates (allocates) waypoints for target navmesh. It also performs any connections within as appropriate.
int TileMap2D::GenerateWaypoints(NavMesh * navMesh){
	std::cout<<"\nTileMap2D::GenerateWaypoints";
	assert(tileTypesAssignedToTiles);
	for (int i = 0; i < levels.Size(); ++i){
		int waypointsPre = navMesh->waypoints;
		TileMapLevel * level = levels[i];
		level->GenerateWaypoints(navMesh, 1.1f);
		int waypointsPost = navMesh->waypoints;
		std::cout<<"\nWaypoints: "<<waypointsPre<<" post:"<<waypointsPost;
		assert(waypointsPost > waypointsPre);
	}
	// Place any existing entities and events on the waypoints as well?
	/*
	for (int x = 0; x < xSize; ++x){
		for (int y = 0; y < ySize; ++y){
			/// If walkable, create a node, or create anyway?
			Waypoint * waypoint = new Waypoint();
			/// TODO: Set elevation as Z-parameter?
			waypoint->position = Vector3f(x,y,0);
			navMesh->AddWaypoint(waypoint);
		}
	}
	*/
	std::cout<<"\n"<<navMesh->waypoints.Size()<<" waypoints added to navMesh.";
	// Bind neighbours? Distance 1 should give 1.41 distance to corners! :)
	int connectionsMade = navMesh->ConnectWaypointsByProximity(1.1f);
	std::cout<<"\n"<<connectionsMade<<" connections made using NavMesh->ConnectWaypointsByProximity.";
	return connectionsMade;
}



TileMapLevel * TileMap2D::ActiveLevel() {
	return activeLevel;
};


/// Returns size of the active level of the map
Vector2i TileMap2D::Size(){
	return activeLevel->Size();
}
