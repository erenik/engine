/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "SpaceShooter2D.h"

Camera * levelCamera = NULL;


// See header file. Position boundaries.
float removeInvuln = 0;
float spawnPositionRight = 0;
float despawnPositionLeft = 0;

bool Level::Load(String fromSource)
{
	source = fromSource;

	/// Clear old stuff.
	ships.Clear();
	millisecondsPerPixel = 250;

	String sourceTxt = source + ".txt";
	String sourcePng = source + ".png";
	music = source+".ogg";
	Vector3i goalColor;

	List<ShipColorCoding> colorCodings;
	List<String> lines = File::GetLines(sourceTxt);
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		// o.o
		if (line.StartsWith("StopLoading"))
			break;
		if (line.StartsWith("//"))
			continue;
		if (line.StartsWith("MillisecondsPerPixel"))
		{
			List<String> tokens = line.Tokenize(" ");
			if (tokens.Size() < 2)
				continue;
			millisecondsPerPixel = tokens[1].ParseInt();
			if (millisecondsPerPixel == 0)
				millisecondsPerPixel = 250;
		}
		if (line.StartsWith("ShipType"))
		{
			List<String> tokens = line.Tokenize(" ");
			ShipColorCoding newCode;
			if (tokens.Size() < 2)
				continue;
			newCode.ship = tokens[1];
			assert(tokens[2] == "RGB");
			if (tokens.Size() < 6)
			{
				std::cout<<"ERrror";
				continue;
			}
			newCode.color[0] = tokens[3].ParseInt();
			newCode.color[1] = tokens[4].ParseInt();
			newCode.color[2] = tokens[5].ParseInt();
			colorCodings.Add(newCode);
		}
		else if (line.StartsWith("Goal"))
		{
			List<String> tokens = line.Tokenize(" ");
			if (tokens.Size() < 5){std::cout<<"\nError"; continue;}
			goalColor[0] = tokens[2].ParseInt();
			goalColor[1] = tokens[3].ParseInt();
			goalColor[2] = tokens[4].ParseInt();
		}
		else if (line.StartsWith("StarSpeed"))
		{
			String vector = line - "StarSpeed";
			starSpeed.ParseFrom(vector);
		}
		else if (line.StartsWith("StarColor"))
		{
			String vector = line - "StarColor";
			starColor.ParseFrom(vector);
		}
	}

	List<String> files ;
	GetFilesInDirectory("Levels/Stage 1", files);  
	Texture * tex = TexMan.LoadTexture(sourcePng, true);
	if (!tex)
		return false;
	tex->releaseOnBufferization = false;
	assert(tex);
	// Parse it.
	for (int x = 0; x < tex->width; ++x)
	{
		// Get top.
		int topY = tex->height - 1;
		for (int y = topY; y > topY - 20; --y)
		{
			Vector3i color = tex->GetPixelVec4i(x,y);
			// Skip if white.
			if (color[0] == 255 && color[1] == 255 && color[2] == 255)
				continue;
			if (color[0] == 0 && color[1] == 0 && color[2] == 0)
				continue;
			if (color == goalColor)
			{
				// o.o
				// Set goal position
				goalPosition = x;
				continue;
			}
			bool found = false;
			for (int i = 0; i < colorCodings.Size(); ++i)
			{
				ShipColorCoding & coding = colorCodings[i];
				if (coding.color == color)
				{
					Ship newShip = Ship::New(coding.ship);
					newShip.position[0] = (float) x;
					newShip.position[1] = (float) 20 - (topY - y);
					// Create ship.
					ships.Add(newShip);
					found = true;
				}
			}
			if (!found)
				std::cout<<"\nUnable to find ship for coded color: "<<color;

		}
	}
	if (ships.Size() == 0)
	{	
		std::cout<<"\nError: No Ships in level.";
		return false;
	}

	// No gravity
	PhysicsMan.QueueMessage(new PMSet(PT_GRAVITY, Vector3f(0,0,0)));

	// Add player?
	return true;
}

// Used for player and camera. Based on millisecondsPerPixel.
Vector3f Level::BaseVelocity()
{
	return Vector3f(1,0,0) * (1000.f / millisecondsPerPixel);
}

void Level::AddPlayer(Ship * playerShip)
{
	if (playerShip->entity)
	{
		MapMan.DeleteEntity(playerShip->entity);
		playerShip->entity = NULL;
	}
	PhysicsProperty * pp = NULL;
	if (!playerShip->entity)
	{
		Model * model = playerShip->GetModel();
		assert(model);
		playerShip->entity = EntityMan.CreateEntity("Player ship", model, TexMan.GetTextureByColor(Color(255,0,0,255)));
		ShipProperty * sp = new ShipProperty(playerShip, playerShip->entity);
		playerShip->entity->properties.Add(sp);
		pp = new PhysicsProperty();
		playerShip->entity->physics = pp;
	}
	// Shortcut..
	Entity * entity = playerShip->entity;
	pp = entity->physics;
	pp->collissionCallback = true;				
	pp->collisionCategory = CC_PLAYER;
	pp->collisionFilter = CC_ENEMY | CC_ENEMY_PROJ;
	pp->velocity = BaseVelocity();
	pp->type = PhysicsType::DYNAMIC;
	// Set player to mid position.
	entity->position = Vector3f(0, 10.f, 0);
	// Rotate ship with the nose from Z- to Y+
	float radians = PI / 2;
	entity->rotation[0] = radians;
	entity->rotation[1] = -radians;
	entity->RecalculateMatrix();
	pp->velocity = BaseVelocity();
	// Register player for rendering.
	MapMan.AddEntity(entity);
}

void Level::SetupCamera()
{
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera();
	// offset to the level entity, just 10 in Z.
	levelCamera->position = Vector3f(0,0,10);
	levelCamera->rotation = Vector3f(0,0,0);
	levelCamera->trackingMode = TrackingMode::ADD_POSITION;
	spaceShooter->ResetCamera();
//	levelCamera->Begin(Direction::RIGHT); // Move it..!
//	levelCamera->
	GraphicsMan.QueueMessage(new GMSetCamera(levelCamera));
}


void Level::Process(int timeInMs)
{
	removeInvuln = levelEntity->position[0] + playingFieldHalfSize[0] + playingFieldPadding + 1.f;
	spawnPositionRight = removeInvuln + 10.f;
	despawnPositionLeft = levelEntity->position[0] - playingFieldHalfSize[0] - 1.f;

	// Check for game over.
	if (playerShip.hitPoints <= 0)
	{
		// Game OVER!
		spaceShooter->GameOver();
		return;
	}

	/// Clearing the level
	if (playerShip.entity->position[0] > goalPosition)
	{
		spaceShooter->LevelCleared();
	}
	else 
	{
		/// PRocess the player ship.
		Process(playerShip);
	}
	// Check if we want to spawn any enemy ships.
	// Add all enemy ships too?
	for (int i = 0; i < ships.Size(); ++i)
	{
		Ship & ship = ships[i];
		if (ship.spawned)
		{
			if (ship.entity == NULL)
				continue;
			if (ship.spawnInvulnerability)
			{
				if (ship.entity->position[0] < removeInvuln)
				{
					// Change color.
					GraphicsMan.QueueMessage(new GMSetEntityTexture(ship.entity, DIFFUSE_MAP | SPECULAR_MAP, TexMan.GetTextureByColor(Color(255,255,255,255))));
					ship.spawnInvulnerability = false;
					continue;				
				}
			}
			// If not, process it?
			else {
				Process(ship);
			}
			continue;
		}
		if (ship.position[0] > spawnPositionRight)
			continue;
		Entity * entity = EntityMan.CreateEntity(ship.type, ship.GetModel(), TexMan.GetTextureByColor(Color(0,255,0,255)));
		entity->position = ship.position;
		float radians = PI / 2;
		entity->rotation[0] = radians; // Rotate up from Z- to Y+
		entity->rotation[1] = radians; // Rorate from Y+ to X-
		entity->RecalculateMatrix();
		
		PhysicsProperty * pp = new PhysicsProperty();
		entity->physics = pp;
		// Setup physics.
		pp->type = PhysicsType::DYNAMIC;
		pp->collisionCategory = CC_ENEMY;
		pp->collisionFilter = CC_PLAYER | CC_PLAYER_PROJ;
		pp->collissionCallback = true;
		// By default, set invulerability on spawn.
		ship.spawnInvulnerability = true;
		ShipProperty * sp = new ShipProperty(&ship, entity);
		entity->properties.Add(sp);
		ship.entity = entity;
		ship.spawned = true;
		shipEntities.Add(entity);
		MapMan.AddEntity(entity);
		ship.StartMovement();
	}

}

/// Process target ship.
void Level::Process(Ship & ship)
{
	if (ship.hasShield)
	{
		// Repair shield
		ship.shieldValue += timeElapsedMs * ship.shieldRegenRate;
		if (ship.shieldValue > ship.maxShieldValue)
			ship.shieldValue = ship.maxShieldValue;
		if (ship.allied)
			spaceShooter->UpdateUIPlayerShield();
	}
}
