/// Contains all creation functions.

#include "SideScroller.h"

/// For the tiling efforts.
namespace BlockType
{
	enum {
		BLOCK, /// A filled block, 2 units wide usually.
		HOLE, // A standard death hole, 2 units wide, 2-4 units deep,
	};
};

/// For tiling-eases tiling effors!
int lastBlockType = BlockType::HOLE;
int nextLastBlockType = BlockType::HOLE; // The previous one. Need last, next last and current one in order to properly set up the tiled sprites for the level.
int currentType = BlockType::HOLE;
Entity * lastBlock;
Entity * nextLastBlock;
Entity * currentBlock;
String tilesetPath = "img/Tileset/";

/// In the current sub-level
List<Entity*> blocksAdded;
float blockSize = 2.f;
String colorStr = "0xb19d7c";
/// Toggled when skipping parts. Defaulf true. When false no block should be created as to speed up processing.
bool blockCreationEnabled = true; 

/// Creates the next level parts.
bool SideScroller::CreateNextLevelParts()
{
	float distToEdge = levelLength - playerEntity->position.x;
	// Pause physics until creation is done?
	if (distToEdge < -1100)
		levelLength += 1000; // Just skip 1K of creation then?

	/// If skipping level, just move past some blocks without actually creating them (since creation/destruction lags).
	if (distToEdge < -55)
	{
		blockCreationEnabled = false;
		// Pause physics since we got a ways to go apparently.
		PhysicsMan.QueueMessage(new PhysicsMessage(PM_PAUSE_SIMULATION));
	}
	else if (distToEdge < 55.f)
	{
		blockCreationEnabled = true;
	}
	else 
	{
		// Done. Resume physics if it was paused.
		QueuePhysics(new PhysicsMessage(PM_RESUME_SIMULATION));
		return false;
	}

	/// If approaching a 1k mark.
	int modK = (int)levelLength % 1000;
	if (modK > 955)
	{
		// 20 breather-blocks. 1 Paco Taco.
		float dist = 1000 - modK;
		float blockWidth = 2.f;
		int numBlocks = dist / blockWidth;
		int j = 0;
		for (float i = 0; i < dist; i += blockWidth, ++j)
		{
			if (j == 2)
				PacoTaco();
			BreatherBlock(blockWidth);
		}
	}
	/// Default level-parts.
	else 
	{
		// Create moar!
		BreatherBlock(); //
		AddLevelPart(); // Must end and start with horizontal tiling
		// Clean-up past level-parts
		CleanupOldBlocks();
	}
//	distToEdge = levelLength - playerEntity->position.x;
	return true;
}

// Appends a block. Default size 2 (blockSize?).
void Block(float size = 2, bool addToVegetization = true) 
{
	if (size <= 0)
		size = blockSize;
//	"0x55"
	if (blockCreationEnabled)
	{
		Entity * block = EntityMan.CreateEntity("LevelPart-block", ModelMan.GetModel("cube.obj"), TexMan.GetTexture(colorStr));
		Vector3f position;
		position.x += levelLength;
		position.x += size * 0.5f;

		/// Scale up ground-tiles.
		float scaleY = 25.f;
		position.y -= scaleY * 0.5f - 0.5f;

		block->position = position;
		block->Scale(Vector3f(size, scaleY, 1));
		PhysicsProperty * pp = block->physics = new PhysicsProperty();
		pp->shapeType = ShapeType::AABB;
		pp->collisionCategory = CC_ENVIRONMENT;
		pp->collisionFilter = CC_PLAYER;
		MapMan.AddEntity(block);
		AddForCleanup(block);
		if (addToVegetization)
			blocksAdded.AddItem(block);

		/// Sprite it up
		Tile(block, BlockType::BLOCK);
	}
	levelLength += size;
}

Entity * RampUpBlock(float width = 2.f, float height = 2.f)
{
	/// Add a regular block underneath.
	Block(width, false);
 
	/// Then add the ramp on top of it.
	String rampColor = "0x55FF";
	Texture * rampTex = TexMan.GetTexture(rampColor);
	Entity * block = EntityMan.CreateEntity("LevelPart-block", 
		ModelMan.GetModel("rampUp1x1.obj"), 
		rampTex);
	Vector3f position;
	position.x += levelLength;
	position.x += width * 0.5f;

	/// Scale up ground-tiles.
	float scaleY = height;
	position.y -= scaleY * 0.5f - 0.5f;

	block->position = position;
	block->position.y += height;
	block->position.x -= width; // Move ramp on top of the block added at the start of the func.
	block->Scale(Vector3f(width, height, 1));
	
	RampProp * prop = new RampProp(block);
	block->properties.AddItem(prop);

	PhysicsProperty * pp = block->physics = new PhysicsProperty();
	pp->shapeType = ShapeType::MESH;
	pp->friction = 0.f; // no friction.
	pp->restitution = 1.f; // Maximum bounce.
	pp->onCollision = true; // Apply speed bonus in physics-thread!
	pp->collisionCategory = CC_ENVIRONMENT;
	pp->collisionFilter = CC_PLAYER;
	MapMan.AddEntity(block);
	AddForCleanup(block);
//	blocksAdded.AddItem(block);
	
	// Add extension-block beneath it?
	block = EntityMan.CreateEntity("UnderRamp",
		ModelMan.GetModel("cube.obj"),
		rampTex);
	position.y -= 5.f;
	block->SetScale(Vector3f(width, 5.f, 1.f));
	MapMan.AddEntity(block, true, true);
	AddForCleanup(block);

	return block;
}

/// Creates the framework for a custom block.
Entity * CustomBlock(float width = 2.f, float height = 0.f)
{
	Entity * block = EntityMan.CreateEntity("LevelPart-block", 
		ModelMan.GetModel("cube.obj"), 
		TexMan.GetTexture(colorStr));
	Vector3f position;
	position.x += levelLength;
	position.x += width * 0.5f;

	/// Scale up ground-tiles.
	float scaleY = 25.f;
	position.y -= scaleY * 0.5f - 0.5f;

	block->position = position;
	block->position.y += height;
	block->Scale(Vector3f(width, scaleY, 1));
	PhysicsProperty * pp = block->physics = new PhysicsProperty();
	pp->shapeType = ShapeType::AABB;
	pp->collisionCategory = CC_ENVIRONMENT;
	pp->collisionFilter = CC_PLAYER;
	MapMan.AddEntity(block);
	AddForCleanup(block);
	blocksAdded.AddItem(block);
	return block;
}

void Hole(float holeSize) // Appends a hole, default side 2.
{
	if (blockCreationEnabled)
	{
		/// Add graphical-only blocks where regular blocks are missing.
		Entity * block = EntityMan.CreateEntity("Hole block", ModelMan.GetModel("cube.obj"), TexMan.GetTexture(colorStr));
		Vector3f position;
		position.x += levelLength;
		position.x += holeSize * 0.5f;

		/// Scale up ground-tiles.
		float scaleY = 25.f;
		position.y -= scaleY * 0.5f - 0.5f; 
		position.y -= 3.0f; // 2 down additionally to simulate hole.

		block->position = position;
		block->Scale(Vector3f(holeSize, scaleY, 1));

		PhysicsProperty * pp = block->physics = new PhysicsProperty();
		pp->shapeType = ShapeType::AABB;
		pp->collisionCategory = CC_ENVIRONMENT;
		pp->collisionFilter = CC_PLAYER;
		pp->collisionCallback = true;

		MapMan.AddEntity(block, true, true);
		AddForCleanup(block);

		// Notify sprite-tiler of the hole
		Tile(block, BlockType::HOLE);
	}

//	blocksAdded.AddItem(block);
	// Increment level-length.
	levelLength += holeSize;
}

void SideScroller::FlatPart() // Just flat, 10 pieces.
{
	for (int i = 0; i < 10; ++i)
	{
		Block();
	}
}

void SideScroller::LinearHoles(int numHoles) // With a number of holes at varying positions, always with 1 block in between. Max 5 holes.
{
	for (int i = 0; i < 10; ++i)
	{
		if (i % 2 == 0 || numHoles < 0)
			Block();
		else 
		{
			Hole(blockSize);
			--numHoles;
		}
	}
}

void SideScroller::DoubleHoles(int numHoles) // With a number of holes at verying positions, always 1 block in between. Max ... 3 holes? 2 + 1 + 2 + 1 + 2
{
	for (int i = 0; i < 10; ++i)
	{
		if (i%3 == 0 || numHoles < 0)
			Block();
		else
		{
			Hole(blockSize);
			--numHoles;
		}
	}
}
	
void SideScroller::TripleHoles(int numHoles) // With a number of holes at varying positions, always 1 block in between. Max 2 holes. 3 + 2 + 3
{
	for (int i = 0; i < 10; ++i)
	{
		if (i % 4 == 0 || numHoles < 0)
			Block();
		else
		{
			Hole(blockSize);
			--numHoles;
		}
	}	
}

/// A big hole, with a platform before it.
void SideScroller::BigHole(float holeSize)
{
	RampUpBlock(2, 1.f);
//	CustomBlock(2, 2.f);
	Hole(holeSize);
}

void SideScroller::BreatherBlock(float width /* = 5.f*/)
{
	if (width > 2.f)
	{
		while(width > 0)
		{
			float bW = width;
			if (bW > 2.f)
				bW = 2.f;
			Block(bW);
			width -= bW;
		}
	}
	else
		Block(width);
}

void SetPhysicsShapeAABB(Entity * entity)
{
	if (!entity->physics)
		entity->physics = new PhysicsProperty();
	entity->physics->shapeType = ShapeType::AABB;
}

void SetCollideWithPlayer(Entity * entity)
{
	if (!entity->physics)
		entity->physics = new PhysicsProperty();
	entity->physics->collisionFilter |= CC_PLAYER;
}

void SideScroller::PacoTaco()
{
	bool didStuff = false;
	if (!paco)
	{
		paco = CreateSprite("img/Outdoor - Mexican town/Taco_guy.png");
		Texture * tex = paco->diffuseMap;
		paco->position.z = -0.01f;
		paco->SetScale(Vector3f(tex->size * 0.005f, 1));
		SetOnGround(paco);
		
		// Set scale accordingly.
		taco = CreateSprite("img/Outdoor - Mexican town/Taco_stand.png");
		taco->position.z = -0.02f;
		taco->SetScale(Vector3f(taco->diffuseMap->size * 0.005f, 1));
		SetOnGround(taco);
		// Add to map.
		MapMan.AddEntity(paco, true, false);
		MapMan.AddEntity(taco, true, false);
	}
	// Set position if far away.
	bool playSFX = true;
	// Get current position.
	float posX = levelLength;
	if (buyTaco >= posX)
		playSFX = false;
	QueuePhysics(new PMSetEntity(paco, PT_POSITION_X, posX));
	QueuePhysics(new PMSetEntity(taco, PT_POSITION_X, posX + 2.0f));
	didStuff = true;
	pacoTacoX = posX;

	// Create the event entities.
	Entity * sfxEntity, * pacoTacoEventEntity;
	EventProperty * eventProp;
	CreateEventEntity(&sfxEntity, &eventProp);
	List<String> tacos;
	tacos.Add("sfx/Tacos.wav", "sfx/Tacos frescos.wav", "sfx/Pacos tacos.wav");
	int i = sfxRand.Randi(tacos.Size()+1) % tacos.Size();
	eventProp->playSFX = tacos[i];
	sfxEntity->position.x = posX - 10;
	MapMan.AddEntity(sfxEntity, false, true);

	CreateEventEntity(&pacoTacoEventEntity, &eventProp);
	pacoTacoEventEntity->position.x = posX + 1;
	eventProp->stopPlayer = true;
	eventProp->message = "OfferTacos";
	MapMan.AddEntity(pacoTacoEventEntity, false, true);

	AddForCleanup(sfxEntity);
	AddForCleanup(pacoTacoEventEntity);
}


// Add some pesos!
void AddPesos()
{
	// o.o
	for (int i = 0; i < blocksAdded.Size(); ++i)
	{
		Entity * block = blocksAdded[i];
		Entity * peso = CreateSprite("img/Pesos/Peso_Mexicano_1921_dos_cont.png");
		peso->properties.Add(new PesoProperty(peso));
		peso->Scale(0.6f);
		peso->SetPosition(block->position + Vector3f(0, block->scale.y * 0.5f + 2,0));
		PhysicsProperty * pp = peso->physics = new PhysicsProperty();
		pp->shapeType = ShapeType::SPHERE;
		pp->recalculatePhysicalRadius = false;
		pp->physicalRadius = 0.5f;
		pp->noCollisionResolutions = true;
		pp->collisionCallback = true;
		pp->collisionCategory = CC_PESO;
		pp->collisionFilter = CC_PLAYER;
		MapMan.AddEntity(peso);
		AddForCleanup(peso);

//		pesos.AddItem(peso); // Add so it is cleaned up later on as we go.
	//	LogMain("Creating peso "+String(((int)peso)%1000)+" at "+String(peso->position.x), INFO);
//		assert(pesos.Duplicates() == 0);
	}
}

void AddClouds()
{
	List<String> cloudTextures;
	cloudTextures.Add("img/Clouds/Cloud1.png",
		"img/Clouds/Cloud2.png");

	// Add some random amount of clouds. Give them some speed.
	int numC = levelRand.Randi(5);
	for (int i = 0; i < numC; ++i)
	{
		int whichCloud = levelRand.Randi(cloudTextures.Size() + 1) % cloudTextures.Size();
		Texture * tex = TexMan.GetTexture(cloudTextures[whichCloud]);
		Entity * cloud = CreateSprite("0xFFAA");
		cloud->diffuseMap = tex;
		cloud->position.x = levelLength + levelRand.Randi(50.f);
		cloud->position.y = levelRand.Randi(25) + 5.f;
		cloud->position.z = -0.9f;
		cloud->SetScale(Vector3f(tex->size, 1) * 0.02f);
		/// No collisions! But some speed o-o
		PhysicsProperty * pp = cloud->physics = new PhysicsProperty();
		pp->collisionsEnabled = false;
		pp->currentVelocity = pp->velocity = Vector3f(-(1 + levelRand.Randf()) * 0.2f,0,0);
		pp->type = PhysicsType::KINEMATIC;

		MapMan.AddEntity(cloud, true, true);
		AddForCleanup(cloud);
	}
}

void LongCactus(Entity * aboveBlock)
{
	Entity * cactus = CreateSprite("img/Outdoor - Mexican town/Big_fucking_cactus.png");
	float cactusSize = 1.5f + levelRand.Randf(1.f);
	cactus->position.x = aboveBlock->position.x;
	cactus->position.y = aboveBlock->position.y + aboveBlock->scale.y * 0.5f + cactusSize * 0.5f;
	cactus->position.z = -0.1f;
	cactus->Scale(Vector3f(1,cactusSize,1));
	MapMan.AddEntity(cactus, true, false);
	AddForCleanup(cactus);
}

void ShortCactus(Entity * aboveBlock)
{
	// Randomize amount?
	int cactii = levelRand.Randi(5) + 1;
	int initialSign = levelRand.Randi(10) > 5? 1 : -1;
	float initialSize = 0.5f + levelRand.Randf(0.5f);
	for (int i = 0; i < cactii; ++i)
	{
		Entity * cactus = CreateSprite("img/Outdoor - Mexican town/Barrel_cactus.png");
		cactus->position.x = aboveBlock->position.x;
		float scale = (1.f - i * 0.2f) * initialSize;
		cactus->scale = Vector3f(1,1,1) * scale; // scale down steadily
		// If non-1 cactii, offset X-position a bit.
		if (i > 0)
		{
			float offset = (i+1) / 2 * 0.5f;
			if (i % 2 == 0)
				offset *= -1;
 			cactus->position.x += offset * initialSign * initialSize;
		}
		cactus->position.y = aboveBlock->position.y + aboveBlock->scale.y * 0.5f + scale * 0.5f;
		cactus->position.z = -0.1f + i * 0.01f; // move steadily forward.
		cactus->hasRescaled = true;
		cactus->RecalculateMatrix();
		// depth-sort when rendering.
		MapMan.AddEntity(cactus, true, false);
		AddForCleanup(cactus);
	}
}

/// Creates a 20+ meters level-part.
void SideScroller::AddLevelPart()
{
	/// Pre-game.
	if (levelLength < 0)
	{
		BreatherBlock(5.f);
		return;
	}
	blocksAdded.Clear();
	// Depending on level.. or possibly random chance.
	int k = levelLength / 1000;
	float r = levelRand.Randf();
	bool addPesos = true;
	switch(k)
	{
		case 0: /// 0 to 1000 meters.
			LinearHoles(levelRand.Randi(6)); break;
		case 1: /// 1k to 2k - introduce the double-holes..!
			if (r > 0.5f)
				LinearHoles(levelRand.Randi(6));
			else
				DoubleHoles(levelRand.Randi(5));
			break;
		case 2: /// 2k to 3k - introduce triple-holes..!
			if (r > 0.7f)
				LinearHoles(levelRand.Randi(6));
			else if (r > 0.5f)
				DoubleHoles(levelRand.Randi(5));
			else
				TripleHoles(levelRand.Randi(4));
			break;
		/// The Holy night.
		case 3: /// 3k to 4k - Big holes!
			if (r > 0.5f)
				BigHole(levelRand.Randi(8.f) + 2.f);
			else
				LinearHoles(levelRand.Randi(4));
			break;
		case 4: // 4K, Bigger holes!
			if (r > 0.4f)
				BigHole(levelRand.Randi(12.f) + 2.f);
			else if (r > 0.2f)
				DoubleHoles(levelRand.Randi(5));
			else
				LinearHoles(levelRand.Randi(6 + 1));
			break;
		case 5: // 5K, Biggestest Holes!
			if (r > 0.8f)
				BigHole(levelRand.Randi(16.f) + 4);
			if (r > 0.3f)
				BigHole(levelRand.Randi(12.f) + 3.f);
			else
				TripleHoles(levelRand.Randi(4));
			break;
		default:
			// Winner?!
			BreatherBlock(10.f);
			lastK = k;
			addPesos = false;
			break;
	}
	// Add some pesos!
	if (addPesos)
		AddPesos();
	AddClouds();

	// Add some cacti.
	for (int i = 0; i < blocksAdded.Size(); ++i)
	{
		// Random chance.
		float r = levelRand.Randf(1.f);
		if (r < 0.6f)
			continue;
		Entity * block = blocksAdded[i];
		r = levelRand.Randf(1.f);
		if (r > 0.5f) // 0.9 to 1.0
			LongCactus(block);
		else // from 0.7 to 0.9
			ShortCactus(block);
	}

}

void AddDBLPart() // Difficulty-By-Length, randomly generated. Used in initial test
{
	float partLength = 20.f;
	// Fetch start and end bounds.
	float startBound = levelLength,
		endBound = levelLength + partLength;
	
	// Create some blocks based on how large this part is.
	float blockSize = 2.f;
	int blocks = partLength / blockSize;
	
	int blocksToCreate = blocks;
	/// Decrease chance of blocks spawning based on distance traveled.
	float ratioBlocks = 0.9f - levelLength * 0.001f;
	for (int i = 0; i < blocksToCreate; ++i)
	{
		/// Check if should place here.
		if (levelRand.Randf(1.f) > ratioBlocks)
		{
			Hole(blockSize);
			continue;
		}
		Block();
	}
}


void Tile(Entity * newBlock, int newBlockType)
{
	List<Entity*> newTileSprites;
	// Push back the old values first.
	nextLastBlock = lastBlock;
	lastBlock = currentBlock;
	nextLastBlockType = lastBlockType;
	lastBlockType = currentType;
	// Assign new ones.
	currentBlock = newBlock;
	currentType = newBlockType;

	// Check neighbor types to determine texture.
	bool isColumn = false;
	bool isLeftStart = false;
	bool isRightEnd = false;
	bool isSimpleGround = true;
	// Check Block type
	if (lastBlockType == BlockType::BLOCK)
	{
		if (nextLastBlockType == BlockType::HOLE)
		{		
			if (currentType == BlockType::HOLE)
				isColumn = true;
			else
				isLeftStart = true;
		}
		else if (currentType == BlockType::HOLE)
			isRightEnd = true;
	}
	if (isColumn || isLeftStart || isRightEnd)
		isSimpleGround = false;

	float holeDepth = 3.f;
	float groundZ = 2.f;
	float groundY = 0.f;
	float scaleX = lastBlock? lastBlock->scale.x : 2.f;
	float posX = lastBlock? lastBlock->position.x : 0.f;
#define SET_DEFAULT_TILE_VALUES(e) \
	e->position.x = posX; \
	e->scale.x = scaleX; \
	e->position.z = groundZ;

	/// Stuff.
	String texture = tilesetPath+"horizontal_ground"; // Default.
	if (isColumn)
	{
		texture = tilesetPath + "pillar_top";
	}
	else if (isLeftStart)
	{
		texture = tilesetPath + "upper_corner_left";
	}
	else if (isRightEnd)
	{
		texture = tilesetPath + "upper_corner_right";
	}

	/// Create tiling for the last block.
	if (lastBlockType == BlockType::HOLE)
		groundY = -3.f; //return; // No tiling for holes. ... or yes! o.o

	// Add ground bit.
	Entity * entity = CreateSprite(texture);
	// Set its location.. in the sky.
	entity->position.y = groundY;
	SET_DEFAULT_TILE_VALUES(entity);
	entity->RecalculateMatrix();
	newTileSprites.AddItem(entity);

	/// Add 'empty' ground underneath.
	entity = CreateSprite(tilesetPath+"Empty");
	if (isSimpleGround)
		entity->position.y = groundY - 1.f;
	else if (lastBlockType == BlockType::HOLE)
		entity->position.y = groundY - 1.f;
	else 
		entity->position.y = groundY - 4.f; // Bit lower.

	entity->scale.y = 20.f;
	entity->position.y -= (entity->scale.y - 1) * 0.5f;
	SET_DEFAULT_TILE_VALUES(entity);
	entity->RecalculateMatrix();
	newTileSprites.AddItem(entity);

	/// More edge-specific shit.
	/// If hole, skip the corner pieces etc.
	if (!isSimpleGround && lastBlockType != BlockType::HOLE)
	{
		String bottomTexture, middleTexture;
		if (isColumn)
		{
			bottomTexture = tilesetPath + "pillar_bottom";
			middleTexture = tilesetPath + "pillar_middle";
		}
		else if (isLeftStart)
		{
			middleTexture = tilesetPath + "vertical_right";
			bottomTexture = tilesetPath + "lower_corner_right";
		}
		else if (isRightEnd)
		{
			bottomTexture = tilesetPath + "lower_corner_left";
			middleTexture = tilesetPath + "vertical_left";
		}
		/// Create middle.
		entity = CreateSprite(middleTexture);
		entity->position.y = groundY - holeDepth * 0.5f; 
		entity->scale.y = holeDepth - 1.f;
		SET_DEFAULT_TILE_VALUES(entity);
		entity->RecalculateMatrix();
		newTileSprites.AddItem(entity);

		/// Create bottom.
		entity = CreateSprite(bottomTexture);
		entity->position.y = groundY - holeDepth; 
		SET_DEFAULT_TILE_VALUES(entity);
		entity->RecalculateMatrix();
		newTileSprites.AddItem(entity);
	}

	AddForCleanup(newTileSprites);
	MapMan.AddEntities(newTileSprites, true, false);

};

