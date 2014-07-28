/// Emil Hedemalm
/// 2014-07-25
/// A space-shooter game based on the input from computer vision imaging

#include "SpaceShooter.h"

#include "Entity/Properties/SpaceShooter/SpaceShooterPlayerProperty.h"
#include "Entity/Properties/SpaceShooter/SpaceShooterProjectileProperty.h"
#include "Entity/Properties/SpaceShooter/SpaceShooterPowerupProperty.h"

#include "Physics/Integrators/SpaceShooter/SpaceShooterIntegrator.h"
#include "Physics/CollisionResolvers/SpaceShooter/SpaceShooterCR.h"
#include "Physics/CollisionDetectors/SpaceShooter/SpaceShooterCD.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Audio/AudioManager.h"

#include "Time/Time.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "TextureManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

SpaceShooterWeaponType::SpaceShooterWeaponType(int type)
	: type(type)
{
	switch(type)
	{
		default:
		case RAILGUN:
			type = RAILGUN;
			coolDown = 1000;
			initialVelocity = 55.f;
			break;
		case MISSILE:
		case LASER:
			assert(false);
	}
}


SpaceShooterIntegrator * spaceShooterIntegrator = 0;
SpaceShooterCR * spaceShooterCR = 0;
SpaceShooterCD * spaceShooterCD = 0;

SpaceShooter::SpaceShooter()
	: Game("SpaceShooter")
{
	player1 = NULL;
	score1Entity = NULL;
	player1Properties = 0;
	constantZ = 0;
	shipScale = 20.f;
	projectileScale = 5.f;

		/// 1 << 4 
	collisionCategoryPlayer = 1 << 4;
	/// 1 << 5
	collisionCategoryEnemy = 1 << 5;

	/// o-o
	gameState = SETTING_UP_PLAYFIELD;

	level = 0;

}

SpaceShooter::~SpaceShooter()
{
	// These should be deleted by the physics-manager.
	/*
	SAFE_DELETE(spaceShooterIntegrator);
	SAFE_DELETE(spaceShooterCR);
	SAFE_DELETE(spaceShooterCD);
	*/
}

void SpaceShooter::ProcessMessage(Message * message)
{
	String msg = message->msg;
	if (msg == "NextLevel")
	{
		SpawnEnemies(++level);
	}
	else if (msg == "Reset")
	{
		Reset();
	}
}

/// Call on a per-frame basis.
void SpaceShooter::Process()
{
	
	// 
	switch(gameState)
	{
		case SETTING_UP_PLAYFIELD:
		{
			SetupPlayingField();
			break;
		}
		case GAME_BEGUN:
		{
			break;
		}
	}
}

/// Fetches all entities concerning this game.
List<Entity*> SpaceShooter::GetEntities()
{
	return players + projectiles + powerups + scoreEntities + enemies;
}

void SpaceShooter::Reset()
{
	level = 0;
	SetupPlayingField();
}

// Call to re-create the playing field as it started out.
void SpaceShooter::SetupPlayingField()
{
	// Delete old projectiles and enemies.
	Physics.QueueMessage(new PMUnregisterEntities(enemies + projectiles));
	Graphics.QueueMessage(new GMUnregisterEntities(enemies + projectiles));

	SetupPhysics();

	// Create player if not already done so.
	if (!player1)
	{
		player1 = MapMan.CreateEntity("SpaceShooterPlayer1", ModelMan.GetModel("SpaceShooter/SpaceShooterShip"), TexMan.GetTexture("Green"));
		player1Properties = new SpaceShooterPlayerProperty(this, player1);
		players.Add(player1);
		player1->properties.Add(player1Properties);
		player1Properties->allied = true;
		player1Properties->isPlayer = true;

		score1Entity = MapMan.CreateEntity("Score", 0, 0);
		scoreEntities.Add(score1Entity);
	}
	// Make player active too..
	else {
		Physics.QueueMessage(new PMRegisterEntity(player1));
		Graphics.QueueMessage(new GMRegisterEntity(player1));
	}
	player1Properties->OnSpawn();

	// Min and max of the playing field. Calculate them accordingly.
	Vector2f halfSize = gameSize * 0.5f;
	Vector3f min, max;
	min = - halfSize;
	max = halfSize;

	// o-o Store it for future use.
	top = gameSize.y * 0.5f;
	bottom = -top;
	right = gameSize.x * 0.5f;
	left = -right;

	// Set 
	spaceShooterIntegrator->frameMin = min;
	spaceShooterIntegrator->frameMax = max;
	

	// X-values, left and right.
	Vector2f playerLines(min.x, max.x);
	
	// Middle!
	Vector2f scoreColumns(min.x, max.x);
	scoreColumns *= 0.5f;



	Physics.Pause();

//			AudioMan.PlayBGM("PongSong.ogg", 1.f);
	AudioMan.QueueMessage(new AMPlayBGM("Breakout/2014-07-22_Growth.ogg"));
//	AudioMan.PlayBGM("Breakout/Breakout.ogg", 1.f);

	// Move stuff to edges of field.
	float playerX = -gameSize.x * 0.4f;
	Physics.QueueMessage(new PMSetEntity(PT_POSITION, player1, Vector3f(playerX, 0, constantZ)));
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, players, shipScale));
	SetupCollisionFilter(players, true);
	// Set player as kinematic

	// Score-boards..
	Physics.QueueMessage(new PMSetEntity(PT_POSITION, score1Entity, Vector3f(0, 0, constantZ + 2)));
//			Physics.QueueMessage(new PMSetEntity(PT_POSITION, score2Entity, Vector3f(scoreColumns.y, 0, z->GetFloat() - 2)));
	Physics.QueueMessage(new PMSetEntity(COLLISIONS_ENABLED, scoreEntities, false));

	// Set scale of score-board text relative to screen size.
	Graphics.QueueMessage(new GMSetEntityf(scoreEntities, TEXT_SIZE_RATIO, gameSize.y * 0.25f));
	Graphics.QueueMessage(new GMSetEntityVec4f(scoreEntities, TEXT_COLOR, Vector4f(0.5,0.5f,0.5f,1)));

	// Make players kinematic.
	Physics.QueueMessage(new PMSetEntity(PHYSICS_TYPE, players, PhysicsType::KINEMATIC));

	// Set scale of all to same?
	float scale = shipScale;
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, players, scale));
	Physics.QueueMessage(new PMSetEntity(SET_ROTATION, players, Quaternion(Vector3f(1, 0, 0), PI * 0.5f)));
	Physics.QueueMessage(new PMSetEntity(ROTATE, players, Quaternion(Vector3f(0, 0, -1), PI * 0.5f)));

	player1Properties->initialScale = Vector3f(scale,scale,scale);
	player1Properties->OnSpawn();
			
	// Scale the frame.
	float colWidth = gameSize.x * 0.05f, 
		colHeight = gameSize.y, 
		rowWidth = gameSize.x, 
		rowHeight = gameSize.y * 0.05f;

	// Place 'em.

	// Set all to use mesh collissions.
	Physics.QueueMessage(new PMSetEntity(PHYSICS_SHAPE, GetEntities(), PhysicsShape::MESH));
			
	// Disable gravity for the game entities.
	Physics.QueueMessage(new PMSetEntity(GRAVITY_MULTIPLIER, GetEntities(), 0.f));

	// Disable collisions between the entities and the wall?
			
	// Reset score.
	player1Properties->score = 0;
	//player2Properties->score = 0;
	OnScoreUpdated();

	gameState = GAME_BEGUN;

	// Spawn enemies!
	SpawnEnemies();

	Physics.Resume();
}

/// Creates a new projectile entity, setting up model and scale appropriately.
Entity * SpaceShooter::NewProjectile(SpaceShooterWeaponType weaponType)
{
	Entity * newProjectile = NULL;
	// Check if we have a sleeping projectile.
	for (int i = 0; i < projectiles.Size(); ++i)
	{
		Entity * entity = projectiles[i];
		SpaceShooterProjectileProperty * prop = (SpaceShooterProjectileProperty*) entity->GetProperty("SpaceShooterProjectileProperty");
		if (prop && prop->sleeping)
		{
			newProjectile = entity;
			// Remove sleeping from the thingy.
			prop->OnSpawn();
			break;
		}
	}
	if (!newProjectile)
	{
		newProjectile = MapMan.CreateEntity("SpaceShooterProjectile", NULL, NULL);
		SpaceShooterProjectileProperty * projectileProp = new SpaceShooterProjectileProperty(this, newProjectile, weaponType);
		newProjectile->properties.Add(projectileProp);
		projectiles.Add(newProjectile);
	}

	switch(weaponType.type)
	{
		case SpaceShooterWeaponType::RAILGUN:
			Graphics.QueueMessage(new GMSetEntity(newProjectile, MODEL, ModelMan.GetModel("Cube")));
			Graphics.QueueMessage(new GMSetEntityTexture(newProjectile, DIFFUSE_MAP, TexMan.GetTexture("Cyan")));
			break;
		default:
			assert(false);
	}
	// Register it for graphics/physics.
	Physics.QueueMessage(new PMRegisterEntity(newProjectile));
	Graphics.QueueMessage(new GMRegisterEntity(newProjectile));

	// Set scale
	SetupPhysics(newProjectile);
	SetProjectileScale(newProjectile);

	return newProjectile;
}


/// Is it outside the frame?
bool SpaceShooter::IsPositionOutsideFrame(Vector3f pos)
{
	if (pos.x > right ||
		pos.x < left ||
		pos.y > top ||
		pos.y < bottom)
		return true;
	return false;
}

void SpaceShooter::SetZ(float z)
{
	constantZ = z;
	if (spaceShooterIntegrator)
		spaceShooterIntegrator->constantZ = z;
	gameState = SETTING_UP_PLAYFIELD;
};

void SpaceShooter::SetFrameSize(Vector2i size)
{
	gameSize = size;
	gameState = SETTING_UP_PLAYFIELD;
}

void SpaceShooter::SetShipScale(float scale)
{
	shipScale = scale;
	// Update all entities scales?
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, players, shipScale));
}



void SpaceShooter::SetPlayerPosition(Vector3f position)
{
	// Move it!
	Physics.QueueMessage(new PMSetEntity(PT_POSITION_Y, player1, position.y));
	player1Properties->lastUserInput = Time::Now();
}

/// Sets up physics integrator, etc. as needed.
void SpaceShooter::SetupPhysics()
{
	if (!spaceShooterIntegrator)
	{
		spaceShooterIntegrator = new SpaceShooterIntegrator(1.f);
		spaceShooterCR = new SpaceShooterCR();
		spaceShooterCD = new SpaceShooterCD();
		spaceShooterIntegrator->constantZ = constantZ;
	}
	if (Physics.physicsIntegrator != spaceShooterIntegrator)
	{
		Physics.QueueMessage(new PMSet(spaceShooterIntegrator));
		Physics.QueueMessage(new PMSet(spaceShooterCR));
		Physics.QueueMessage(new PMSet(spaceShooterCD));
	}
}

/// Sets up stuff specific for the entities in this little game.
void SpaceShooter::SetupPhysics(List<Entity*> forEntities)
{

}

/// Sets up collision filters appropriately. 2 sides!
void SpaceShooter::SetupCollisionFilter(List<Entity*> entities, bool allied /*= false*/)
{
	int colCat = allied? collisionCategoryPlayer : collisionCategoryEnemy;
	int colFilter = allied? collisionCategoryEnemy : collisionCategoryPlayer;
	Physics.QueueMessage(new PMSetEntity(COLLISION_CATEGORY, entities, colCat));
	Physics.QueueMessage(new PMSetEntity(COLLISION_FILTER, entities, colFilter));
}

void SpaceShooter::SetProjectileScale(List<Entity*> entities)
{
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, entities, projectileScale));
}



/// Spawns enemies for a level. This will spawn all enemies, far to the right. 
void SpaceShooter::SpawnEnemies(int level)
{

	Physics.Pause();

	Random enemyRandom;
	int enemiesToSpawn = level + 10;

	// Remove all entities first.
	Physics.QueueMessage(new PMUnregisterEntities(enemies));
	Graphics.QueueMessage(new GMUnregisterEntities(enemies));

	int enemiesToCreate = enemiesToSpawn - enemies.Size();
	for (int i = 0; i < enemiesToCreate; ++i)
	{

		Entity * enemy = MapMan.CreateEntity("SpaceShooterEnemy", ModelMan.GetModel("SpaceShooter/SpaceShooterShip"), TexMan.GetTexture("Red"));
		SpaceShooterPlayerProperty * enemyProperty = new SpaceShooterPlayerProperty(this, enemy);
		enemy->properties.Add(enemyProperty);
		enemies.Add(enemy);
	}

	for (int i = 0; i < enemiesToSpawn; ++i)
	{
		Entity * enemy = enemies[i];
		SpaceShooterPlayerProperty * enemyProperty = (SpaceShooterPlayerProperty *) enemy->GetProperty(SpaceShooterPlayerProperty::ID());
		enemyProperty->OnSpawn();
		enemyProperty->weaponType = SpaceShooterWeaponType(SpaceShooterWeaponType::RAILGUN);
		enemyProperty->weaponType.coolDown *= 25.f / (5.f + level);
		SetupPhysics(enemy);

		Vector3f position;
		position.x = i * 50.f;
		position.y = enemyRandom.Randf(gameSize.y) - gameSize.y * 0.5f;
		Physics.QueueMessage(new PMSetEntity(PT_POSITION, enemy, position));


		// Give it some speed.
		float speed = 15 + level;
		Vector3f dir(-1,0,0);
		Vector3f velocity = dir * speed;
		Physics.QueueMessage(new PMSetEntity(PT_VELOCITY, enemy, velocity));

		// Make active!
		Physics.QueueMessage(new PMRegisterEntity(enemy));
		Graphics.QueueMessage(new GMRegisterEntity(enemy));
	}

	SetupCollisionFilter(enemies, false);

	// Set scale of all to same?
	float scale = shipScale;
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, enemies, shipScale));
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, enemies, scale));
	Physics.QueueMessage(new PMSetEntity(SET_ROTATION, enemies, Quaternion(Vector3f(1, 0, 0), PI * 0.5f)));
	Physics.QueueMessage(new PMSetEntity(ROTATE, enemies, Quaternion(Vector3f(0, 0, 1), PI * 0.5f)));

	Physics.QueueMessage(new PMSetEntity(PHYSICS_SHAPE, enemies, PhysicsShape::MESH));

	Physics.Resume();
}


/// Sets up the scrolling background as appropriate.
void SpaceShooter::SetupBackground(int forLevel)
{

}


// Update text on both entities displaying the scores.
void SpaceShooter::OnScoreUpdated()
{

}

/// Check if level is completed -> Spawn 'em again.
void SpaceShooter::OnPlayerDestroyed(Entity * player)
{
	// Deaded D:
	if (player == player1)
	{
		std::cout<<"\n PLYAER HIT!";
		MesMan.QueueMessages("Reset");
		return;
	}

	int stillAlive = 0;
	for (int i = 0; i <  enemies.Size(); ++i)
	{
		Entity * e = enemies[i];
		SpaceShooterPlayerProperty * sspp = (SpaceShooterPlayerProperty *)e->GetProperty(SpaceShooterPlayerProperty::ID());
		if (sspp && !sspp->sleeping)
		{
			++stillAlive;
		}
	}
	if (stillAlive)
		return;

	// New level!
	MesMan.QueueMessages("NextLevel");
}
