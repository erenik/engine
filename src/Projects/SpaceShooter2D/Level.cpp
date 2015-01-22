/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "SpaceShooter2D.h"

Camera * levelCamera = NULL;

bool Level::Load(String fromSource)
{
	source = fromSource;
	// Load weapons.
	Weapon::LoadTypes("Ship Data/Alien/Weapons.csv");
	// Load ship-types.
	Ship::LoadTypes("Ship Data/Alien/Ships.csv");

	/// Clear old stuff.
	ships.Clear();
	millisecondsPerPixel = 250;

	String sourceTxt = source + ".txt";
	String sourcePng = source + ".png";
	Vector3i goalColor;

	List<ShipColorCoding> colorCodings;
	List<String> lines = File::GetLines(sourceTxt);
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
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
			newCode.color.x = tokens[3].ParseInt();
			newCode.color.y = tokens[4].ParseInt();
			newCode.color.z = tokens[5].ParseInt();
			colorCodings.Add(newCode);
		}
		else if (line.StartsWith("Goal"))
		{
			List<String> tokens = line.Tokenize(" ");
			if (tokens.Size() < 5){std::cout<<"\nError"; continue;}
			goalColor.x = tokens[2].ParseInt();
			goalColor.y = tokens[3].ParseInt();
			goalColor.z = tokens[4].ParseInt();
		}
	}

	List<String> files ;
	GetFilesInDirectory("Levels/Stage 1", files);  
	Texture * tex = TexMan.LoadTexture(sourcePng, true);
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
			if (color.x == 255 && color.y == 255 && color.z == 255)
				continue;
			if (color.x == 0 && color.y == 0 && color.z == 0)
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
					newShip.position.x = (float) x;
					newShip.position.y = (float) 20 - (topY - y);
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
	if (!playerShip->entity)
	{
		playerShip->entity = EntityMan.CreateEntity("Player ship", ModelMan.GetModel("sphere.obj"), TexMan.GetTextureByColor(Color(255,0,0,255)));
		ShipProperty * sp = new ShipProperty(playerShip, playerShip->entity);
		playerShip->entity->properties.Add(sp);
	}
	// Shortcut..
	Entity * playerShipEntity = playerShip->entity;
	// Set player to mid position.
	PhysicsMan.QueueMessage(new PMSetEntity(playerShipEntity, PT_SET_POSITION, Vector3f(0, 10.f, 0)));
	// Set player collision stats.
	PhysicsMan.QueueMessage(new PMSetEntity(playerShipEntity, PT_COLLISION_CATEGORY, CC_PLAYER));
	PhysicsMan.QueueMessage(new PMSetEntity(playerShipEntity, PT_COLLISION_FILTER, CC_ENEMY));
	// Register player for rendering.
	MapMan.AddEntity(playerShipEntity);

	// Begin movement of player too?
	PhysicsMan.QueueMessage(new PMSetEntity(playerShipEntity, PT_VELOCITY, BaseVelocity()));
}

void Level::SetupCamera()
{
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera();
	// Set it up.
	levelCamera->velocity = BaseVelocity();
	levelCamera->position = Vector3f(0,10,10);
	levelCamera->rotation = Vector3f(0,0,0);
	spaceShooter->ResetCamera();
//	levelCamera->Begin(Direction::RIGHT); // Move it..!
//	levelCamera->
	GraphicsMan.QueueMessage(new GMSetCamera(levelCamera));
}

