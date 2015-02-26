/// Emil Hedemalm
/// 2015-02-15
/// Grid used for creating maps for the TIFS game.

#include "TIFSGrid.h"
#include "Maps/Grids/TileGrid3D.h"
#include "Random/Random.h"
#include "File/LogFile.h"
#include "TIFS.h"

TIFSTile::TIFSTile()
: Tile()
{
	Initialize();
}

// Called at construction, sets default values to the booleans. Does not change positions.
void TIFSTile::Initialize()
{
	isOccupied = isGround = isRoad = false;
	isBuilding = isTurret = false;
	entity = NULL;
}

Road::Road()
{
	isLonely = true;
}

TIFSGrid::TIFSGrid()
{
	maxTilesPerBuilding = 10;
	roadWidth = 1;
	triesPerBuilding = 200;
	roadTexture = "0x225599FF";
	roadScale = 1.f;
	minDistanceBetweenParallelRoads = 2;
	maxRoadLength = 10;
	requireRoadConnections = true;
	parallelDistanceThreshold = 10.f;
}

TIFSGrid::~TIFSGrid()
{
	roads.ClearAndDelete();
}

/** Resizes to the chosen amount of tiles, centered on Vector3f(0,0,0)
	MapSize will dictate the distance between each tile, rounded to the nearest integer.
*/
void TIFSGrid::Resize(Vector3i gridSize, ConstVec3fr mapSize)
{
	LogMain("TIFSGrid::Resize - start", DEBUG);
	// Only re-allocate the grid if necessary.
	bool clean = true;
	if (grid.Size() != gridSize)
	{
		grid.Resize(gridSize);
		clean = false;
	}

	List<TIFSTile*> tiles = grid.GetTiles();
	// Clean it manually here otherwise.
	if (clean)
	{
		for (int i = 0; i < tiles.Size(); ++i)
		{
			TIFSTile * tile = tiles[i];
			tile->Initialize();
		}	
	}
	this->mapSize = mapSize;
	sizePerTile = mapSize / gridSize;
	// Set their positions.
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		// XZ
		tile->position = (tile->matrixPosition / Vector3f(gridSize)) * mapSize - mapSize * 0.5f;
		// Add one half tile, to center it all.
		tile->position += Vector3f(0.5,0,0.5) * sizePerTile;
		// Y
		tile->position.y = tile->matrixPosition.y * mapSize.y;
		if (debug == -2)
			std::cout<<"\nTile position: "<<tile->position;
		/// Mark default ground-plane.
		if (tile->matrixPosition.y == 0)
			tile->isGround = true;
	}	
	LogMain("TIFSGrid::Resize - finish", DEBUG);
}

void TIFSGrid::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			String arg;
			List<String> tokens = msg.Tokenize("()");
			if (tokens.Size() > 1)
				arg = tokens[1];
			
			if (msg.StartsWith("BasePlateTexture"))	basePlateTexture = arg;
			else if (msg.StartsWith("BasePlates"))	BasePlates();
			else if (msg.StartsWith("RoadTexture"))	roadTexture = arg;
			else if (msg.StartsWith("RoadWidth"))	roadWidth = arg.ParseInt();				
			else if (msg.StartsWith("MaxRoadLength"))	maxRoadLength = arg.ParseInt();
			else if (msg.StartsWith("RoadScale"))	roadScale = arg.ParseFloat();
			else if (msg.StartsWith("MinDistanceBetweenParallelRoads"))	minDistanceBetweenParallelRoads = (int) arg.ParseFloat();
			else if (msg.StartsWith("RequireRoadConnections"))	requireRoadConnections = arg.ParseBool();
			else if (msg.StartsWith("ParallelDistanceThreshold"))	parallelDistanceThreshold = arg.ParseFloat();
			else if (msg.StartsWith("PlaceRoads")) PlaceRoads(arg.ParseInt());
			else if (msg.StartsWith("TriesPerBuilding"))	triesPerBuilding = arg.ParseInt();
			else if (msg.StartsWith("MaxTilesPerBuilding"))	maxTilesPerBuilding = arg.ParseInt();
		}
		break;
	}
}	


/// Used for the various algorithms inside.
void TIFSGrid::SetExpansionFlags(bool x, bool y, bool z)
{
	expandXMinus = expandXPlus = x;
	expandYMinus = expandYPlus = y;
	expandZMinus = expandZPlus = z;
}

/// Used for the various algorithms inside.
void TIFSGrid::SetExpansionFlags(bool xPlus, bool xMinus, bool yPlus, bool yMinus, bool zPlus, bool zMinus)
{
	expandXPlus = xPlus;
	expandYPlus = yPlus;
	expandZPlus = zPlus;
	expandXMinus = xMinus;
	expandYMinus = yMinus;
	expandZMinus = zMinus;
}


Random playerRandom;

/// This may be either on ground or at some strategical point (key buildings?)
bool TIFSGrid::GetNewPlayerPosition(Vector3f & playerPos)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	if (roads.Size() == 0)
	{
		playerPos = Vector3f(0,5,0);
		return true;
	}
	// Randomize a few times?
	int tries = 0;
	while (tries < 1000)
	{
		++tries;
		Road * road = roads[playerRandom.Randi(roads.Size()) % roads.Size()];
		List<TIFSTile*> roadTiles = road->tiles;
		/// Get a tile.
		TIFSTile * tile = roadTiles[playerRandom.Randi(roadTiles.Size()) % roadTiles.Size()] ;
		/// Demand road?
		if (!tile->isRoad)
			continue;
		playerPos = tile->position;
		tile->isOccupied = true;
		return true;
	}
	/// No roads? D:
	return false;
}


Random turretRandom;

/// This may be either on ground or at some strategical point (key buildings?)
bool TIFSGrid::GetNewTurretPosition(Vector3f & turretPos)
{
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	int tries = 0;
	while (tries < 100)
	{
		++tries;
		int index = turretRandom.Randi(tiles.Size() - 1);
		TIFSTile * tile = tiles[index];
		if (tile->isOccupied == true)
			continue;
		if (!tile->isGround)
			continue;
		/// Check that the neighbours have at least 1 road-part?
		List<TIFSTile*> neighbours = grid.GetNeighboursXZ(tile);
		bool roadNeighbour = false;
		for (int i = 0; i < neighbours.Size(); ++i)
		{
			TIFSTile * neighbour = neighbours[i];
			if (neighbour->isRoad)
			{
				roadNeighbour = true;
			}
		}
		if (!roadNeighbour)
			continue;
		turretPos = tile->position;
		tile->isOccupied = true;
		return true;
	}
	return false;
}

/** Requested slot-size in meters. List of available slot-sizes can be queried.
	Size is requested size in meters. Size will be updated to the size the new building can maximally take up.
	Position is stored in position.
*/
Random buildingRandom;
bool TIFSGrid::GetNewBuildingPosition(Vector3f & maxSize, Vector3f & position, List<TIFSTile*> & relevantTiles)
{
	SetExpansionFlags(true, false, true);
	// Set their positions.
	List<TIFSTile*> tiles = grid.GetTiles();
	// Randomize a few times?
	int tries = 0;
	while (tries < triesPerBuilding)
	{
		++tries;
		int index = buildingRandom.Randi(tiles.Size()) % tiles.Size();
		TIFSTile * tile = tiles[index];
		/// Test the initial tile.
		if (!AllGood(List<TIFSTile*>(tile), NO_OPTION))
			continue;

		// Check if surrounding tiles comply to the demand of the size?
		List<TIFSTile*> affectedTiles, queryTiles;
		List<TIFSTile*> testTiles = tile;
		// Expand size until we can no more?
		bool expansionOK = true;
		Vector3i min, max, scale, goodMin, goodMax;
		maxSize = min = max = tile->matrixPosition;
		while(expansionOK)
		{
			// Save from last iteration
			goodMin = min;
			goodMax = max;
			expansionOK = Expand(min, max, NO_OPTION);
			if (!expansionOK)
				break;
			scale = max - min + Vector3i(1,1,1);
			int numTiles = scale.x * scale.y * scale.z;
			/// Break if we have consumed enough tiles already. Don't want to exhaust the map?
			if (numTiles > maxTilesPerBuilding)
				break;
		}
		/// Paste over to min and max from the confirmed good values.
		min = goodMin;
		max = goodMax;
		/// Mark all query-tiles as occupied, so we get some spacing. -> Do this when actually placing the building.
		relevantTiles = grid.GetTiles(min,max);
		for (int i = 0; i < relevantTiles.Size(); ++i)
		{
			// Mark the tiles as occupied for now?
			TIFSTile * tile = relevantTiles[i];
			tile->isOccupied = true;
			tile->isBuilding = true;
		}
		// Get size?
		Vector3i maxSizeInMatrix = ((max - min) + Vector3i(1,1,1));
		maxSize =  maxSizeInMatrix * sizePerTile;
		// Adjust position, if we change the expansion algorithm earlier?
		position = (max + min) * 0.5;
		/// Set floor and ground-level, though.
		position.y = (float)min.y;
		/// Multiply by grid-size.
		position *= sizePerTile;
		/// Translate so we center on 0,0,0
		position -= mapSize * 0.5f;
		position.y = 0;
		return true;
	}
	return false;	
}

void TIFSGrid::BasePlates()
{
	List<TIFSTile*> tiles = grid.GetTiles();
	// Generate road-starting-point.
	roads.ClearAndDelete();
	int tries = 0;
	for (int i = 0; i < tiles.Size(); i += 10)
	{
		// Row?
		int row = i / grid.Size().x;
		if (row % 10 != 0)
			continue;
		// Get starting point.
		TIFSTile * tile = tiles[i];
		bool randomTile = false;
		int direction;
		Entity * basePlate = EntityMan.CreateEntity("Base plate", ModelMan.GetModel("cube"), TexMan.GetTexture("img/Roads/"+basePlateTexture));
		PhysicsProperty * pp = basePlate->physics = new PhysicsProperty();
		pp->shapeType = PhysicsShape::AABB;
		/// Setup instancing if enabled.
		GraphicsProperty * gp = basePlate->graphics = new GraphicsProperty(basePlate);
		gp->renderInstanced = true;
		basePlate->position = tile->position;
		/// move it up a bit
		float roadHeight = 0.15f; // 2 dm high roads?
		basePlate->position.y += roadHeight * 0.5f;
		Vector3f sizePerGroundTile = sizePerTile * 10.f;
		basePlate->Scale(Vector3f(1,0,1) * sizePerGroundTile * 0.975 + Vector3f(0,roadHeight,0));
		GraphicsQueue.Add(new GMRegisterEntity(basePlate));
//		PhysicsQueue.Add(new PMRegisterEntity(basePlate));
	}
}

/// o.o
Random roadRandom;
void TIFSGrid::PlaceRoads(int roadsToPlace)
{
	List<TIFSTile*> tiles = grid.GetTiles();
	// Generate road-starting-point.
	roads.ClearAndDelete();
	int tries = 0;
	while(roads.Size() < roadsToPlace)
	{
		++tries;
		if (tries > roadsToPlace * 10)
			break;
		// Get starting point.
		TIFSTile * tile = NULL;
		bool randomTile = false;
		int direction;
		Vector3i startPosition;

		/// If required connections, base it on an existing road.. unless they be deaderee.
		if (requireRoadConnections && roads.Size())
		{
			// Take direction of initial road into account.
			Road * existing = roads[roadRandom.Randi(100000) % roads.Size()];
			direction = (existing->direction + 1) % 2;
			tile = existing->tiles[roadRandom.Randi(100000) % existing->tiles.Size()];
		//	if (!tile->isEndpoint)
		//		;
		}
		/// No requirements? random starting point.
		else 
			randomTile = true;
		if (randomTile)
		{
			tile = tiles[buildingRandom.Randi(tiles.Size()) % tiles.Size()];
			// Direction.
			direction = roadRandom.Randi(100) % 2;
		}
		startPosition = tile->matrixPosition;

		// Skip tile if it is a road................... no
//		if (tile->isRoad)
//			continue;

		bool good = true;
		for (int i = 0; i < roads.Size(); ++i)
		{
			Road * otherRoad = roads[i];
			if (otherRoad->direction == direction)
			{
				float distance;
				Vector3f dist = startPosition - otherRoad->startPosition;
				Vector3i compareMin = otherRoad->min,
					compareMax = otherRoad->max;
				switch(direction)
				{
					case Road::X_ROAD: 
						distance = dist.z; 
						compareMin.z = compareMax.z = startPosition.z;
						compareMin.y = compareMax.y = startPosition.y;
						break;
					case Road::Z_ROAD: 
						distance = dist.x; 
						compareMin.x = compareMax.x = startPosition.x;
						compareMin.y = compareMax.y = startPosition.y;
						break;
				}
				if (AbsoluteValue(distance) < minDistanceBetweenParallelRoads + roadWidth * 2)
				{
					// Check min-max to other road?
					float dist1 = (compareMin - startPosition).Length();
					float dist2 = (compareMax - startPosition).Length();
					float closest = min(dist1, dist2);
					if (closest < parallelDistanceThreshold)
						good = false;
					break;
				}
			}
		}
		if (!good)
		{
			continue;
		}
		//  Create it.
		Road * road = new Road();
		road->startPosition = startPosition;
		road->direction = direction;
		CreateRoad(road);
		roads.AddItem(road);
	}
}

/// Creates the actual road, filling it with tiles :3
void TIFSGrid::CreateRoad(Road * road)
{
	int tries = 0;
	// Get random point.
	Vector3i startingPoint = road->startPosition;
	// Expand size until we can no more?
	bool expansionOK = true;
	Vector3i min, max, scale, goodMin, goodMax;
	Vector3i maxSize = min = max = startingPoint;
	int expandX = this->roadWidth;
	int expandZ = maxRoadLength;
	if (road->direction == Road::X_ROAD)
	{
		expandX = maxRoadLength; 
		expandZ = roadWidth;
	}
	while(expansionOK)
	{
		SetExpansionFlags(expandX > 0, expandX > 1, 0, 0, expandZ > 0, expandZ > 1);
//			SetExpansionFlags(expandX-- > 0, 0, expandZ-- > 0);
		// Save from last iteration
		goodMin = min;
		goodMax = max;
		Vector3i oldScale = max - min;
		expansionOK = Expand(min, max, IGNORE_ROADS | IGNORE_NULL);
		if (!expansionOK)
			break;
		// OK? decrease what was expanded.
		Vector3i newScale = max - min;
		Vector3i scaleDiff = newScale - oldScale;
		expandX -= AbsoluteValue(scaleDiff.x);
		expandZ -= AbsoluteValue(scaleDiff.z);
		scale = max - min + Vector3i(1,1,1);
		int numTiles = scale.x * scale.y * scale.z;
		/// Break if we have consumed enough tiles already. Don't want to exhaust the map?
//			if (numTiles > maxTilesPerBuilding)
//				break;
	}
	// Now then, create entities for each?
	List<TIFSTile *> tiles = grid.GetTiles(min, max);
	road->tiles = tiles;
	for (int j = 0; j < tiles.Size(); ++j)
	{
		TIFSTile * tile = tiles[j];
		if (tile->entity)
			continue;
		tile->isOccupied = true;
		tile->isRoad = true;
		Entity * roadPart = EntityMan.CreateEntity("Road part", ModelMan.GetModel("cube"), TexMan.GetTexture("img/Roads/"+roadTexture));
		tile->entity = roadPart;
		PhysicsProperty * pp = roadPart->physics = new PhysicsProperty();
		pp->shapeType = PhysicsShape::AABB;
		/// Setup instancing if enabled.
		if (tifsInstancingEnabled)
		{
			GraphicsProperty * gp = roadPart->graphics = new GraphicsProperty(roadPart);
			gp->renderInstanced = true;
		}
		roadPart->position = tile->position;
		/// move it up a bit
		float roadHeight = 0.15f; // 2 dm high roads?
		roadPart->position.y += roadHeight * 0.5f;
		roadPart->Scale(Vector3f(1,0,1) * this->sizePerTile * roadScale + Vector3f(0,roadHeight,0));
		GraphicsQueue.Add(new GMRegisterEntity(roadPart));
		PhysicsQueue.Add(new PMRegisterEntity(roadPart));
	}
	// Save min max in road.
	road->min = goodMin;
	road->max = goodMax;
}

void TIFSGrid::ConnectLonelyRoads()
{
	for (int i = 0; i < roads.Size(); ++i)
	{
		Road * road = roads[i];
		if (!road->isLonely)
			continue;
		List<TIFSTile*> tiles = road->tiles;
		for (int j = 0; j < roads.Size(); ++j)
		{
			Road * road2 = roads[i];
			List<TIFSTile*> tiles2 = road2->tiles;
			/// Get closest tiles between em?
//			float leastDiff 
		}
	}
}


/// Expands the list with all neighbours.
bool TIFSGrid::ExpandZ(List<TIFSTile*> & tiles)
{
	return Expand(tiles, false, false, true);
}

/// Expands the list with all neighbours.
bool TIFSGrid::ExpandX(List<TIFSTile*> & tiles)
{
	return Expand(tiles, true, false, false);
}

/// Expands the list with all neighbours.
bool TIFSGrid::ExpandXZ(List<TIFSTile*> & tiles)
{
	return Expand(tiles, true, false, true);
}

/// Expands the list with all neighbours. Max size is increased based on the positions of the tiles and the grid.
bool TIFSGrid::Expand(List<TIFSTile*> & tiles, bool x, bool y, bool z, int options)
{
	List<TIFSTile*> toAdd;
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		List<TIFSTile*> toAddNow;
		if (x)
		{
			// Add all in X we can?
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(1,0,0)));
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(-1,0,0)));
		}
		if (z)
		{
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,0,1)));
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,0,-1)));
		}
		if (y)
		{
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,1,0)));
			toAddNow.Add(grid.GetTileByIndex(tile->matrixPosition + Vector3i(0,-1,0)));
		}
		if (AllGood(toAddNow, options))
			toAdd.Add(toAddNow);
	}
	toAdd.RemoveUnsorted(tiles);
	/// Nothing to add? Then return false, we are fully maximized now.
	if (toAdd.Size() == 0)
		return false;
	tiles.Add(toAdd);
	return true;
}

/// Tries to expand all sides by 1 unit in the grid. The results are stored in the argument references.
/// Returns false if no expanion could be performed.
bool TIFSGrid::Expand(Vector3i & minVec, Vector3i & maxVec, int options /*= NO_OPTION*/)
{
	List<Vector3i> additionVectors;
	if (expandXPlus)
		additionVectors.AddItem(Vector3i(1,0,0));
	if (expandXMinus)
		additionVectors.AddItem(Vector3i(-1,0,0));
	if (expandYPlus)
		additionVectors.AddItem(Vector3i(0,1,0));
	if (expandYMinus)
		additionVectors.AddItem(Vector3i(0,-1,0));
	if (expandZPlus)
		additionVectors.AddItem(Vector3i(0,0,1));
	if (expandZMinus)
		additionVectors.AddItem(Vector3i(0,0,-1));

	Vector3i newMin, newMax, potentialNewMin, potentialNewMax;
	newMin = potentialNewMin = minVec;
	newMax = potentialNewMax = maxVec;
	bool goodArr[6];
	int bads = 0;
	memset(goodArr, 1, sizeof(bool) * 6);
	bool success = false;
	Vector3i relevantTestVector;
	for (int i = 0; i < additionVectors.Size(); ++i)
	{
		if (!goodArr[i])
			continue;
		// Each other expands positive and negative axis.
		Vector3i & expansionVector = additionVectors[i];
		bool max;
		if (expansionVector.x > 0 || expansionVector.y > 0 || expansionVector.z > 0)
			max = true;
		else 
			max = false;
		if (max)
		{
			relevantTestVector = potentialNewMax += expansionVector;
			potentialNewMin = newMin; // revert from failure last loop.
		}
		else
		{
			relevantTestVector = potentialNewMin += expansionVector;
			potentialNewMax = newMax; // revert from failure last loop.
		}
		Vector3i minForTest, maxForTest;
		minForTest = potentialNewMin;
		maxForTest = potentialNewMax;
		if (expansionVector.x)
			minForTest.x = maxForTest.x = relevantTestVector.x;
		if (expansionVector.y)
			minForTest.y = maxForTest.y = relevantTestVector.y;
		if (expansionVector.z)
			minForTest.z = maxForTest.z = relevantTestVector.z;
		// Left..
		List<TIFSTile*> tiles;
		if (grid.OKGet(minForTest, maxForTest))
			tiles = grid.GetTiles(minForTest, maxForTest);
		bool good = AllGood(tiles, options);
		if (good)
		{
			newMax = potentialNewMax;
			newMin = potentialNewMin;
			success = true;
		}
		else 
		{
			++bads;
			goodArr[i] = false;
			if (bads == 6)
				break;
		}
	}
	minVec = newMin;
	maxVec = newMax;
	return success;
}

/// D:
bool TIFSGrid::AllGood(List<TIFSTile*> & tiles, int options)
{
	if (tiles.Size() == 0)
		return false;
	for (int i = 0; i < tiles.Size(); ++i)
	{
		TIFSTile * tile = tiles[i];
		bool bad = false;
		if (tile == NULL)
		{
			bad = true;
			if (options & IGNORE_NULL)
			{
				// Just ignore it and check next one.
				bad = false;
				tiles.RemoveIndex(i);
				--i;
				continue;
			}
		}
		else if (tile->isOccupied)
		{
			bad = true;
			if (tile->isRoad && options & IGNORE_ROADS)
				bad = false;
		}
		/// o.o Abort!
		if (bad)
		{
			return false;
		}
	}
	return true;
}



/** Creates a road along the given lines. Fails if any position there is not vacant or already a road. 
	Should be called before placing buildings and turrets?
*/
bool SetRoad(List<Vector3i> positions);

