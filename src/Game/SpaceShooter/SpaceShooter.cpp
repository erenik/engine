/// Emil Hedemalm
/// 2014-07-25
/// A space-shooter game based on the input from computer vision imaging

#include "SpaceShooter.h"

#include "SpaceShooterPlayerProperty.h"
#include "SpaceShooterProjectileProperty.h"
#include "SpaceShooterPowerupProperty.h"
#include "SpaceShooterExplosionProperty.h"

#include "SpaceShooterIntegrator.h"
#include "SpaceShooterCR.h"
#include "SpaceShooterCD.h"

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
	projectileScale = 3.f;

		/// 1 << 4 
	collisionCategoryPlayer = 1 << 4;
	/// 1 << 5
	collisionCategoryEnemy = 1 << 5;

	/// o-o
	gameState = SETTING_UP_PLAYFIELD;

	level = 0;
	playerDead = false;
	lastFrame = Time::Now();

	yOnly = false;
}

SpaceShooter::~SpaceShooter()
{
	// These should be deleted by the physics-manager.
	/*
	SAFE_DELETE(spaceShooterIntegrator);
	SAFE_DELETE(spaceShooterCR);
	SAFE_DELETE(spaceShooterCD);
	*/

	MapMan.DeleteEntities(GetEntities());
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
	else if (msg == "Pause/Break")
	{
		this->SetPause(!paused);
	}
}

/// Call on a per-frame basis.
void SpaceShooter::Process()
{
	Time now = Time::Now();
	int timeInMs = (now - lastFrame).Milliseconds();
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
			if (playerDead)
			{
				deadTimeMs += timeInMs;
				if (deadTimeMs > 2000)
				{
					Reset();
				}
			}
			break; 
		}
	}
	lastFrame = now;
}

/// Fetches all entities concerning this game.
List<Entity*> SpaceShooter::GetEntities()
{
	return players + projectiles + powerups + scoreEntities + enemies + explosions;
}

/// If set (with true), will enabled tracking/movement of the player with the mouse.
void SpaceShooter::UseMouseInput(bool useItOrNot)
{
	player1Properties->useMouseInput = useItOrNot;
}


void SpaceShooter::Reset()
{
	level = 0;
	playerDead = false;
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
	Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION, Vector3f(playerX, 0, constantZ)));
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, shipScale));
	SetupCollisionFilter(players, true);
	// Set player as kinematic

	// Score-boards..
	Physics.QueueMessage(new PMSetEntity(score1Entity, PT_POSITION, Vector3f(0, 0, constantZ + 2)));
//			Physics.QueueMessage(new PMSetEntity(PT_POSITION, score2Entity, Vector3f(scoreColumns.y, 0, z->GetFloat() - 2)));
	Physics.QueueMessage(new PMSetEntity(scoreEntities, PT_COLLISIONS_ENABLED, false));

	// Set scale of score-board text relative to screen size.
	Graphics.QueueMessage(new GMSetEntityf(scoreEntities, GT_TEXT_SIZE_RATIO, gameSize.y * 0.25f));
	Graphics.QueueMessage(new GMSetEntityVec4f(scoreEntities, GT_TEXT_COLOR, Vector4f(0.5,0.5f,0.5f,1)));

	// Make players kinematic.
	Physics.QueueMessage(new PMSetEntity(players, PT_PHYSICS_TYPE, PhysicsType::KINEMATIC));

	// Set scale of all to same?
	float scale = shipScale;
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, scale));
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_ROTATION, Quaternion(Vector3f(1, 0, 0), PI * 0.5f)));
	Physics.QueueMessage(new PMSetEntity(players, PT_ROTATE, Quaternion(Vector3f(0, 0, -1), PI * 0.5f)));

	player1Properties->initialScale = Vector3f(scale,scale,scale);
	player1Properties->OnSpawn();
			
	// Scale the frame.
	float colWidth = gameSize.x * 0.05f, 
		colHeight = gameSize.y, 
		rowWidth = gameSize.x, 
		rowHeight = gameSize.y * 0.05f;

	// Place 'em.

	// Set all to use mesh collissions... no.
//	Physics.QueueMessage(new PMSetEntity(GetEntities(), PT_PHYSICS_SHAPE, PhysicsShape::MESH));
			
	// Disable gravity for the game entities.
	Physics.QueueMessage(new PMSetEntity(GetEntities(), PT_GRAVITY_MULTIPLIER, 0.f));

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
Entity * SpaceShooter::NewProjectile(SpaceShooterWeaponType weaponType, Vector3f atPosition)
{
	Entity * newProjectile = NULL;
	SpaceShooterProjectileProperty * projectileProp;
	// Check if we have a sleeping projectile.
	for (int i = 0; i < projectiles.Size(); ++i)
	{
		Entity * entity = projectiles[i];
		projectileProp = (SpaceShooterProjectileProperty*) entity->GetProperty("SpaceShooterProjectileProperty");
		if (projectileProp && projectileProp->sleeping)
		{
			newProjectile = entity;
			// Set position straight away.
			Physics.QueueMessage(new PMSetEntity(newProjectile, PT_SET_POSITION, atPosition));
			break;
		}
	}
	if (!newProjectile)
	{
		// Check how many are already created. Max 100 active projectiles? It'll lag otherwise?
		if (projectiles.Size() > 100)
			return NULL;

		newProjectile = MapMan.CreateEntity("SpaceShooterProjectile", NULL, NULL, atPosition);
		projectileProp = new SpaceShooterProjectileProperty(this, newProjectile, weaponType);
		newProjectile->properties.Add(projectileProp);
		projectiles.Add(newProjectile);
	}

	switch(weaponType.type)
	{
		case SpaceShooterWeaponType::RAILGUN:
			Graphics.QueueMessage(new GMSetEntity(newProjectile, GT_MODEL, ModelMan.GetModel("Sphere")));
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

	// Remove sleeping from the thingy.
	projectileProp->OnSpawn();

	return newProjectile;
}

Entity * SpaceShooter::NewExplosion(Vector3f atPosition)
{
	Entity * newExplosion = NULL;

	for (int i = 0; i < explosions.Size(); ++i)
	{
		Entity * explosion = explosions[i];
		SpaceShooterExplosionProperty * prop = explosion->GetProperty<SpaceShooterExplosionProperty>();
		if (prop && prop->sleeping)
		{
			newExplosion = explosion;
			prop->OnSpawn();
			Physics.QueueMessage(new PMSetEntity(newExplosion, PT_SET_POSITION, atPosition));
		}
	}

	if (newExplosion == NULL)
	{
		newExplosion = MapMan.CreateEntity("Explosion", ModelMan.GetModel("Sprite"), TexMan.GetTexture("Explosion"), atPosition);
		SpaceShooterExplosionProperty * explode = new SpaceShooterExplosionProperty(newExplosion);
		newExplosion->properties.Add(explode);
		explode->OnSpawn();
	}
	// Make it render correctly
	Graphics.QueueMessage(new GMSetEntityb(newExplosion, GT_REQUIRE_DEPTH_SORTING, true));
	Graphics.QueueMessage(new GMSetEntityi(newExplosion, GT_BLEND_MODE_SRC, GL_SRC_ALPHA));
	Graphics.QueueMessage(new GMSetEntityi(newExplosion, GT_BLEND_MODE_DST, GL_DST_ALPHA));
	Graphics.QueueMessage(new GMSetEntityb(newExplosion, GT_DEPTH_TEST, false));
	// Register for graphics!
	Graphics.QueueMessage(new GMRegisterEntity(newExplosion));

	return newExplosion;
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

/// Will.. remove from rendering/physics relevant entities and set the pause-state.
void SpaceShooter::SetPause(bool pause)
{
	this->paused = pause;
	// Pause..
	Physics.QueueMessage(new PMSetEntity(GetEntities(), PT_PAUSED, pause));
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
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, shipScale));
}



void SpaceShooter::SetPlayerPosition(Vector3f position)
{
	if (!player1)
		return;
	if (player1Properties->useMouseInput)
		return;
	// Move it!
	if (yOnly)
	{
		Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION_Y, position.y));
	}
	else 
	{
		Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION, position));
	}
	if (!player1Properties)
		return;
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
	}
	Integrator * currentIntegrator = Physics.physicsIntegrator;
	if (currentIntegrator != spaceShooterIntegrator)
	{
		Physics.QueueMessage(new PMSet(spaceShooterIntegrator));
		Physics.QueueMessage(new PMSet(spaceShooterCR));
		Physics.QueueMessage(new PMSet(spaceShooterCD));
	}
	spaceShooterIntegrator->constantZ = constantZ;
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
	Physics.QueueMessage(new PMSetEntity(entities, PT_COLLISION_CATEGORY, colCat));
	Physics.QueueMessage(new PMSetEntity(entities, PT_COLLISION_FILTER, colFilter));
}

void SpaceShooter::SetProjectileScale(List<Entity*> entities)
{
	Physics.QueueMessage(new PMSetEntity(entities, PT_SET_SCALE, projectileScale));
}



Random enemyRandom;
	
/// Spawns enemies for a level. This will spawn all enemies, far to the right. 
void SpaceShooter::SpawnEnemies(int level)
{

	Physics.Pause();

	int enemiesToSpawn = level + 10;

	// Remove all entities first.
//	Physics.QueueMessage(new PMUnregisterEntities(enemies));
//	Graphics.QueueMessage(new GMUnregisterEntities(enemies));
	// Mark them as sleeping too!
	for (int i = 0; i < enemies.Size(); ++i)
	{
		Entity * enemy = enemies[i];
		SpaceShooterPlayerProperty * enemyProp = (SpaceShooterPlayerProperty*)enemy->GetProperty(SpaceShooterPlayerProperty::ID());
		enemyProp->Remove();
	}

	// Create new enemy-entities as needed.
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
		position.x = gameSize.x * 0.5f + i * 50.f;
		position.y = enemyRandom.Randf(gameSize.y) - gameSize.y * 0.5f;
		Physics.QueueMessage(new PMSetEntity(enemy, PT_POSITION, position));


		// Give it some speed.
		float speed = 15 + level;
		Vector3f dir(-1,0,0);
		Vector3f velocity = dir * speed;
		Physics.QueueMessage(new PMSetEntity(enemy, PT_VELOCITY, velocity));

		// Make active!
		Physics.QueueMessage(new PMRegisterEntity(enemy));
		Graphics.QueueMessage(new GMRegisterEntity(enemy));
	}

	SetupCollisionFilter(enemies, false);

	// Set scale of all to same?
	float scale = shipScale;
	Physics.QueueMessage(new PMSetEntity(enemies, PT_SET_SCALE, shipScale));
	Physics.QueueMessage(new PMSetEntity(enemies, PT_SET_ROTATION, Quaternion(Vector3f(1, 0, 0), PI * 0.5f)));
	Physics.QueueMessage(new PMSetEntity(enemies, PT_ROTATE, Quaternion(Vector3f(0, 0, 1), PI * 0.5f)));

	Physics.QueueMessage(new PMSetEntity(enemies, PT_PHYSICS_SHAPE, PhysicsShape::MESH));

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
		playerDead = true;
		deadTimeMs = 0;
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
