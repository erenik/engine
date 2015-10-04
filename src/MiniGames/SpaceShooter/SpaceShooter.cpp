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
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Particles/Sparks.h"
#include "Graphics/Particles/SparksEmitter.h"

#include "Audio/AudioManager.h"

#include "Time/Time.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"


SpaceShooterWeaponType::SpaceShooterWeaponType(int type)
	: type(type)
{
	damage = 15;
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


SpaceShooter::SpaceShooter()
	: Game2D("SpaceShooter")
{
	useMouseInput = false;
	spaceShooterIntegrator = 0;
	spaceShooterCR = 0;
	spaceShooterCD = 0;

	player1 = NULL;
	scoreEntity = NULL;
	hpEntity = NULL;

	sparks = 0;

	flipX = 1.f;
}

SpaceShooter::~SpaceShooter()
{
	MapMan.DeleteEntities(GetEntities());
	if (GraphicsManager::GraphicsProcessingActive())
	{
		Graphics.QueueMessage(new GMUnregisterParticleSystem(sparks, true));
	}
	else 
		delete sparks;
	sparks = NULL;
	// Stop da music! o.o
	if (AudioManager::AudioProcessingActive())
		AudioMan.QueueMessage(new AMStopBGM());
}

/// Performs one-time initialization tasks. This should include initial allocation and initialization.
void SpaceShooter::Initialize()
{
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
	return players + projectiles + powerups + scoreEntities + enemies + explosions + hpEntity;
}

List<Entity*> SpaceShooter::StaticEntities()
{
	return scoreEntities + hpEntity;
}


/// If set (with true), will enabled tracking/movement of the player with the mouse.
void SpaceShooter::UseMouseInput(bool useItOrNot)
{
	player1Properties->useMouseInput = useItOrNot;
}


void SpaceShooter::Reset()
{
	playerDead = false;
	SetupPlayingField();
}

Lighting spaceShooterLighting;
	

// Call to re-create the playing field as it started out.
void SpaceShooter::SetupPlayingField()
{
	// Load lighting
	spaceShooterLighting.LoadFrom("lighting/SpaceShooter");
	Graphics.QueueMessage(new GMSetLighting(&spaceShooterLighting));


	// Delete old projectiles and enemies.
	Physics.QueueMessage(new PMUnregisterEntities(enemies + projectiles));
	Graphics.QueueMessage(new GMUnregisterEntities(enemies + projectiles));

	SetupPhysics();
	
	if (!sparks)
	{
		// New global sparks system.
		sparks = new Sparks(true);
		// Register it for rendering.
		Graphics.QueueMessage(new GMRegisterParticleSystem(sparks, true));
	}
//	Graphics.QueueMessage(new GMSetParticleSystem(sparks, GT_PARTICLE_SCALE, 1.f));

	// Create player if not already done so.
	if (!player1)
	{
		player1 = MapMan.CreateEntity("SpaceShooterPlayer1", ModelMan.GetModel("SpaceShooter/SpaceShooterShip"), TexMan.GetTexture("Green"));
		player1Properties = new SpaceShooterPlayerProperty(this, player1);
		players.Add(player1);
		player1->properties.Add(player1Properties);
		player1Properties->allied = true;
		player1Properties->isPlayer = true;
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
	top = gameSize[1] * 0.5f;
	bottom = -top;
	right = gameSize[0] * 0.5f;
	left = -right;

	// Set 
	spaceShooterIntegrator->frameMin = min;
	spaceShooterIntegrator->frameMax = max;
	

	// X-values, left and right.
	Vector2f playerLines(min[0], max[0]);
	
	if (!hpEntity)
	{
		hpEntity = MapMan.CreateEntity("HPEntity", NULL, NULL);
	}
	float textSize = 50.f;
	float alpha = 0.4f;
	// Reset score.
	score = 0;
	Graphics.QueueMessage(new GMSetEntitys(hpEntity, GT_TEXT, "HP: 100/100"));
	Graphics.QueueMessage(new GMSetEntityf(hpEntity, GT_TEXT_SIZE_RATIO, textSize));
	Graphics.QueueMessage(new GMSetEntityVec4f(hpEntity, GT_TEXT_COLOR, Vector4f(1.f,1.f,1.f,alpha)));
	Physics.QueueMessage(new PMSetEntity(hpEntity, PT_POSITION, Vector3f(0, top - textSize * 0.2f, -3)));

	if (!scoreEntity)
	{
		scoreEntity = MapMan.CreateEntity("Score", NULL, NULL);
		scoreEntities.Add(scoreEntity);
	}
	Graphics.QueueMessage(new GMSetEntitys(scoreEntity, GT_TEXT, "Score: 0"));
	Graphics.QueueMessage(new GMSetEntityf(scoreEntity, GT_TEXT_SIZE_RATIO, textSize));
	Graphics.QueueMessage(new GMSetEntityVec4f(scoreEntity, GT_TEXT_COLOR, Vector4f(1,1,.5f,alpha)));
	Physics.QueueMessage(new PMSetEntity(scoreEntity, PT_POSITION, Vector3f(0, bottom + textSize * 0.2f, -3)));



	// Middle!
	Vector2f scoreColumns(min[0], max[0]);
	scoreColumns *= 0.5f;



	Physics.Pause();

//			AudioMan.PlayBGM("PongSong.ogg", 1.f);
	AudioMan.QueueMessage(new AMPlayBGM("SpaceShooter/2014-07-25_SpaceShooter.ogg", 0.5f));
//	AudioMan.PlayBGM("Breakout/Breakout.ogg", 1.f);

	// Move stuff to edges of field.
	float playerX = -gameSize[0] * 0.4f;
	Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION, Vector3f(playerX * flipX, 0, constantZ)));
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, shipScale));
	SetupCollisionFilter(players, true);

	// Make players kinematic.
	Physics.QueueMessage(new PMSetEntity(players, PT_PHYSICS_TYPE, PhysicsType::KINEMATIC));

	// Set scale of all to same?
	float scale = shipScale;
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, scale));
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_ROTATION, Quaternion(Vector3f(1, 0, 0), PI * 0.5f)));
	Physics.QueueMessage(new PMSetEntity(players, PT_ROTATE, Quaternion(Vector3f(0, 0, -1 * flipX), PI * 0.5f)));

	player1Properties->initialScale = Vector3f(scale,scale,scale);
	player1Properties->OnSpawn();
			
	// Scale the frame.
	float colWidth = gameSize[0] * 0.05f, 
		colHeight = gameSize[1], 
		rowWidth = gameSize[0], 
		rowHeight = gameSize[1] * 0.05f;

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
	SpawnEnemies(level);

	Physics.Resume();
}

/// Creates a new projectile entity, setting up model and scale appropriately.
Entity * SpaceShooter::NewProjectile(SpaceShooterWeaponType weaponType, ConstVec3fr atPosition, ConstVec3fr initialVelocity)
{
	// Play a Sound-effect for it's spawning!
	float volume = 0.1f;
	// Check distance to player.
	Vector3f vectorDistance = (player1->position - atPosition);
	vectorDistance /= 100.f;
	float distSquared = vectorDistance.Length();
	float distanceModifierToVolume = 1 / distSquared;
	if (distanceModifierToVolume > 1.f)
		distanceModifierToVolume = 1.f;
	volume = distanceModifierToVolume * 0.4f;
	
	AudioMan.QueueMessage(new AMPlaySFX("SpaceShooter/214990__peridactyloptrix__laser-blast-x3_4.wav", volume));

	Entity * newProjectile = NULL;
	SpaceShooterProjectileProperty * projectileProp;
	// Check if we have a sleeping projectile.
	for (int i = 0; i < projectiles.Size(); ++i)
	{
		Entity * entity = projectiles[i];
		projectileProp = (SpaceShooterProjectileProperty*) entity->GetProperty("SpaceShooterProjectileProperty");
		if (projectileProp && projectileProp->sleeping && !entity->registeredForPhysics)
		{
			// Set position straight away. As it is not registered currently (or should be, we should be able to write to the position straight away.
			entity->SetPosition(atPosition);
			newProjectile = entity;
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
			if (initialVelocity[0] > 0)
				Graphics.QueueMessage(new GMSetEntityTexture(newProjectile, DIFFUSE_MAP, TexMan.GetTexture("0x00FFFF")));
			else 
				Graphics.QueueMessage(new GMSetEntityTexture(newProjectile, DIFFUSE_MAP, TexMan.GetTexture("0xFFFF00")));
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

Entity * SpaceShooter::NewExplosion(ConstVec3fr atPosition, int type)
{
	Entity * newExplosion = NULL;

	// Add a temporary emitter to the particle system to add some sparks to the collision
	SparksEmitter * tmpEmitter = new SparksEmitter(atPosition);
	// Check distance to player.
	Vector3f vectorDistance = (player1->position - atPosition);
	vectorDistance /= 100.f;
	float distSquared = vectorDistance.Length();
	float distanceModifierToVolume = 1 / distSquared;
	if (distanceModifierToVolume > 1.f)
		distanceModifierToVolume = 1.f;

	float explosionSFXVolume;
	switch(type)
	{
		case ExplosionType::PROJECTILE:
			tmpEmitter->SetEmissionVelocity(50.f);
			tmpEmitter->particlesPerSecond = 1000;
			explosionSFXVolume = 0.1f;
			break;
		case ExplosionType::SHIP:
			tmpEmitter->SetEmissionVelocity(60.f);
			tmpEmitter->particlesPerSecond = 2000;
			explosionSFXVolume = 0.15f;
			break;
	}
	float volume = distanceModifierToVolume * explosionSFXVolume;
	// Play SFX!
	AudioMan.QueueMessage(new AMPlaySFX("SpaceShooter/235968__tommccann__explosion-01.wav", volume));


	tmpEmitter->deleteAfterMs = 100;	
	tmpEmitter->SetScale(2.0f);
	tmpEmitter->SetColor(Vector4f(1.f, 0.5f, 0.1f, 1.f));
	Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, this->sparks));

	for (int i = 0; i < explosions.Size(); ++i)
	{
		Entity * explosion = explosions[i];
		SpaceShooterExplosionProperty * prop = explosion->GetProperty<SpaceShooterExplosionProperty>();
		if (prop && prop->sleeping && !explosion->registeredForPhysics)
		{
			newExplosion = explosion;
			prop->SetType(type);
			// Set position before re-spawning
			newExplosion->SetPosition(atPosition);
			// Make it visible again with the custom blending.
			prop->OnSpawn();
			break;
		}
	}
	// If no old explosion could be re-used, create a new one.
	if (newExplosion == NULL)
	{
		newExplosion = MapMan.CreateEntity("Explosion", ModelMan.GetModel("Sprite"), TexMan.GetTexture("Explosion"), atPosition);
		SpaceShooterExplosionProperty * explode = new SpaceShooterExplosionProperty(newExplosion);
		newExplosion->properties.Add(explode);

		// Make it render correctly, should only need to call this once.
		Graphics.QueueMessage(new GMSetEntityb(newExplosion, GT_REQUIRE_DEPTH_SORTING, true));
		Graphics.QueueMessage(new GMSetEntityi(newExplosion, GT_BLEND_MODE_SRC, GL_SRC_ALPHA));
		Graphics.QueueMessage(new GMSetEntityi(newExplosion, GT_BLEND_MODE_DST, GL_ONE));
		Graphics.QueueMessage(new GMSetEntityb(newExplosion, GT_DEPTH_TEST, false));

		// Set type and spawn it!
		explode->SetType(type);
		explode->OnSpawn();
	}
	// Register for graphics!
	Graphics.QueueMessage(new GMRegisterEntity(newExplosion));
	return newExplosion;
}



/// Is it outside the frame?
bool SpaceShooter::IsPositionOutsideFrame(ConstVec3fr pos)
{
	if (pos[0] > right ||
		pos[0] < left ||
		pos[1] > top ||
		pos[1] < bottom)
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


/// Determines what enemies are spawned, and where.
void SpaceShooter::SetLevel(int l)
{
	this->level = l;
	gameState = SETTING_UP_PLAYFIELD;
}

void SpaceShooter::SetZ(float z)
{
	constantZ = z;
	if (spaceShooterIntegrator)
		spaceShooterIntegrator->constantZ = z;
	gameState = SETTING_UP_PLAYFIELD;
};

void SpaceShooter::SetFlipX(float newX)
{
	flipX = newX;
	gameState = SETTING_UP_PLAYFIELD;
}

void SpaceShooter::SetShipScale(float scale)
{
	shipScale = scale;
	// Update all entities scales?
	Physics.QueueMessage(new PMSetEntity(players, PT_SET_SCALE, shipScale));
}



void SpaceShooter::SetPlayerPosition(ConstVec3fr position)
{
	if (!player1)
		return;
	if (player1Properties->useMouseInput)
		return;
	// Move it!
	if (yOnly)
	{
		Physics.QueueMessage(new PMSetEntity(player1, PT_POSITION_Y, position[1]));
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

		// Set physics properties once when spawning, should be enough!
		Physics.QueueMessage(new PMSetEntity(enemy, PT_SET_SCALE, shipScale));
		Physics.QueueMessage(new PMSetEntity(enemy, PT_PHYSICS_SHAPE, PhysicsShape::MESH));

		enemies.Add(enemy);
	}
	// For all enemies to spawn..
	for (int i = 0; i < enemiesToSpawn; ++i)
	{
		Entity * enemy = enemies[i];

		// Reset rotation if the table was flipped!
		Physics.QueueMessage(new PMSetEntity(enemy, PT_SET_ROTATION, Quaternion(Vector3f(1, 0, 0), PI * 0.5f)));
		Physics.QueueMessage(new PMSetEntity(enemy, PT_ROTATE, Quaternion(Vector3f(0, 0, 1 * flipX), PI * 0.5f)));

		SpaceShooterPlayerProperty * enemyProperty = (SpaceShooterPlayerProperty *) enemy->GetProperty(SpaceShooterPlayerProperty::ID());
		enemyProperty->OnSpawn();
		enemyProperty->weaponType = SpaceShooterWeaponType(SpaceShooterWeaponType::RAILGUN);
		enemyProperty->weaponType.coolDown *= 25.f / (5.f + level);
		SetupPhysics(enemy);

		Vector3f position;
		position[0] = (gameSize[0] * 0.5f + i * 50.f) * flipX;
		position[1] = enemyRandom.Randf(gameSize[1]) - gameSize[1] * 0.5f;
		// Ensure enemy is not registered for physics when setting position explicitly like this? <- No need? Physics was paused at the start of this function o.o'
		enemy->SetPosition(position);
//		Physics.QueueMessage(new PMSetEntity(enemy, PT_POSITION, position));

		// Give them a score-value based on level..
		enemyProperty->score = 100 + level * 50;
		enemyProperty->hp = enemyProperty->maxHP = 50 + i * 10;

		// Give it some speed.
		float speed = 15 + level;
		Vector3f dir(-1 * flipX, 0, 0);
		Vector3f velocity = dir * speed;
		Physics.QueueMessage(new PMSetEntity(enemy, PT_VELOCITY, velocity));

		// Make active!
		Physics.QueueMessage(new PMRegisterEntity(enemy));
		Graphics.QueueMessage(new GMRegisterEntity(enemy));
	}

	SetupCollisionFilter(enemies, false);

	// Set scale of all to same?
	float scale = shipScale;
	
	Physics.Resume();
}


/// Sets up the scrolling background as appropriate.
void SpaceShooter::SetupBackground(int forLevel)
{

}


// Update text on both entities displaying the scores.
void SpaceShooter::OnScoreUpdated()
{
	Graphics.QueueMessage(new GMSetEntitys(scoreEntity, GT_TEXT, "Score: "+String(score)));
}

/// o.o
void SpaceShooter::UpdatePlayerHP()
{
	Graphics.QueueMessage(new GMSetEntitys(hpEntity, GT_TEXT, "HP: "+String(player1Properties->hp)+"/"+String(player1Properties->maxHP)));
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
		// Go to first level?
		level = 0;
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
