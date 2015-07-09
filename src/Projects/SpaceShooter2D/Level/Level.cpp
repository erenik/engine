/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "../SpaceShooter2D.h"
#include "SpawnGroup.h"
#include "Text/TextManager.h"
#include "LevelMessage.h"
#include "Explosion.h"

Camera * levelCamera = NULL;

// See header file. Position boundaries.
float removeInvuln = 0;
float spawnPositionRight = 0;
float despawnPositionLeft = 0;

Level * activeLevel = NULL;

Time levelTime; // Time used in level-scripting. Will be paused arbitrarily to allow for easy scripting.
Time flyTime; // The actual player-felt time. 
bool gameTimePaused = false;
bool defeatedAllEnemies = true;
bool failedToSurvive = false;

Level::Level()
{
	height = 20.f;
	endCriteria = NO_MORE_ENEMIES;
	levelCleared = false;
	activeLevelMessage = 0;
}

Level::~Level()
{
	spawnGroups.ClearAndDelete();
	messages.ClearAndDelete();
	ships.ClearAndDelete();
}

// Used for player and camera. Based on millisecondsPerPixel.
Vector3f Level::BaseVelocity()
{
	return Vector3f(1,0,0) * (1000.f / millisecondsPerPixel);
}

/// Creates player entity within this level. (used for spawning)
Entity * Level::AddPlayer(Ship * playerShip, ConstVec3fr atPosition)
{	
	playerShip->movementDisabled = false;
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
	pp->collisionCallback = true;				
	pp->collisionCategory = CC_PLAYER;
	pp->collisionFilter = CC_ENEMY | CC_ENEMY_PROJ;
	pp->velocity = BaseVelocity();
	pp->type = PhysicsType::DYNAMIC;
	// Set player to mid position.
	entity->position = atPosition;
	// Rotate ship with the nose from Z- to Y+
	float radians = PI / 2;
//	entity->rotation[0] = radians;
//	entity->rotation[1] = -radians;
	entity->SetRotation(Vector3f(radians, -radians, 0));
//	entity->RecalculateMatrix();
	pp->velocity = BaseVelocity();
	// Register player for rendering.
	MapMan.AddEntity(entity);
	return entity;
}

void Level::SetupCamera()
{
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera("LevelCamera");
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
	activeLevel = this;

	removeInvuln = levelEntity->position[0] + playingFieldHalfSize[0] + playingFieldPadding + 1.f;
	spawnPositionRight = removeInvuln + 15.f;
	despawnPositionLeft = levelEntity->position[0] - playingFieldHalfSize[0] - 1.f;

	// Check for game over.
	if (playerShip->hp <= 0)
	{
		// Game OVER!
		if (onDeath.Length() == 0)
			spaceShooter->GameOver();
		else if (onDeath.StartsWith("RespawnAt"))
		{
			playerShip->hp = playerShip->maxHP;
			this->AddPlayer(playerShip, Vector3f(levelEntity->position.x, 10.f, 0));
			// Reset level-time.
			String timeStr = onDeath.Tokenize("()")[1];
			levelTime.ParseFrom(timeStr);
			OnLevelTimeAdjusted();
		}
		else 
			std::cout<<"\nBad Game over (onDeath) critera.";
		return;
	}

	flyTime.AddMs(timeInMs);

	/// Clearing the level
	if (LevelCleared())
	{
		spaceShooter->OnLevelCleared();
		return; // No more processing if cleared?
	}
	if (gameTimePaused)
		return;
	else
	{
		levelTime.AddMs(timeInMs);
	}

	/// Process active explosions.
	for (int i = 0; i < explosions.Size(); ++i)
	{
		Explosion * exp = explosions[i];
		float detonationVelocity = 10.f;
		exp->currentRadius += timeInMs * 0.001f * detonationVelocity;
		bool finalTurn = false;
		if (exp->currentRadius > exp->weapon.explosionRadius)
		{
			exp->currentRadius = exp->weapon.explosionRadius;
			finalTurn = true;
		}
		/// Apply damage to nearby ships (if any)
		List<float> distances;
		List<Ship*> relShips = GetShipsAtPoint(exp->position, exp->currentRadius, distances);
		for (int j = 0; j < relShips.Size(); ++j)
		{
			Ship * ship = relShips[j];
			if (exp->affectedShips.Exists(ship))
				continue;
			float amount = exp->weapon.damage;
			/// Decrease damage linearly with distance to center of explosion?
			float dist = distances[j];
			float relDist = dist / exp->weapon.explosionRadius;
			float relDmg = 1 - relDist;
			float finalDmg = relDmg * amount;
			exp->totalDamageInflicted += finalDmg;
			ship->Damage(finalDmg, false);
			exp->affectedShips.AddItem(ship); // Ensure no double-triggering.
			std::cout<<"\nAffected ships: "<<exp->affectedShips.Size()<<" total dmg inflicted: "<<exp->totalDamageInflicted;
		}

		if (finalTurn)
		{
			explosions.RemoveItem(exp);
			delete exp;
			--i;
		}
	}

	/// Check messages.
	if (messages.Size())
	{
		for (int i = 0; i < messages.Size(); ++i)
		{
			LevelMessage * lm = messages[i];
			if (lm->hidden)
				continue;
			if (lm->startTime < levelTime && !lm->displayed)
			{
				if (activeLevelMessage)
					continue;
				if (lm->Display())
					activeLevelMessage = lm;
			}
			if (lm->displayed && lm->stopTime < levelTime)
			{
				// Retain sorting.
				lm->Hide();
				if (activeLevelMessage == lm)
				{
					activeLevelMessage = 0;
				}
			}
		}
	}

	/// Check spawn-groups to spawn.
	if (spawnGroups.Size())
	{
		for (int i = 0; i < spawnGroups.Size(); ++i)
		{
			SpawnGroup * sg = spawnGroups[i];
			if (sg->defeated)
			{
				continue;
			}
			if (sg->spawned)
			{
				// Check if it's defeated?
				if (sg->shipsDefeatedOrDespawned > sg->number)
				{
					sg->defeated = true;
					if (sg->pausesGameTime)
						gameTimePaused = false;
				}
				continue;
			}
			int msToSpawn = (sg->spawnTime - levelTime).Milliseconds();
			if (msToSpawn < 0 && msToSpawn > -2000) // Don't spawn things that shouldn't have spawned more than a few seconds ago.
			{
				defeatedAllEnemies = false;
				sg->Spawn();
				if (sg->pausesGameTime)
					gameTimePaused = true;
			}
		}
	}
}

void Level::ProcessMessage(Message * message)
{
	String & msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "EndLevel")
			{
				levelCleared = true;
			}
			if (msg == "PauseGameTime")
				gameTimePaused = true;
			if (msg == "ResumeGameTime")
				gameTimePaused = false;
			if (msg == "ResetFailedToSurvive")
				failedToSurvive = false;
			break;		
		}
	}
}

void Level::ProceedMessage()
{
	if (activeLevelMessage)
		activeLevelMessage->Hide();
	activeLevelMessage = 0;
}

void Level::SetTime(Time newTime)
{
	Time oldTime;
	levelTime = newTime;
	OnLevelTimeAdjusted();
}
/// enable respawing on shit again.
void Level::OnLevelTimeAdjusted()
{
	for (int i = 0; i < spawnGroups.Size(); ++i)
	{
		SpawnGroup * sg = spawnGroups[i];
		if (sg->spawnTime > levelTime)
			spawnGroups[i]->Reset();
	}
	for (int i = 0; i < messages.Size(); ++i)
	{
		LevelMessage * lm = messages[i];
		if (lm->startTime > levelTime)
		{
			lm->displayed = lm->hidden = false;
		}
	}
}


// Check spawn groups.
bool Level::LevelCleared()
{
	if (endCriteria == NO_MORE_ENEMIES)
	{
		if (levelTime.Seconds() < 3)
			return false;
		if (spawnGroups.Size())
			return false;
		if (shipEntities.Size())
			return false;
		if (messages.Size())
			return false;
	}
	else if (endCriteria == EVENT_TRIGGERED)
		return levelCleared;
	return true;
}

Entity * Level::ClosestTarget(bool ally, ConstVec3fr position)
{
	if (!ally)
	{
		return playerShip->entity;
	}
	Entity * closest = NULL;
	float closestDist = 100000.f;
	for (int i = 0; i < shipEntities.Size(); ++i)
	{
		Entity * e = shipEntities[i];
		float dist = (e->position - position).LengthSquared();
		if (dist < closestDist)
		{
			closest = e;
			closestDist = dist;
		}
	}
	return closest;
}

/// o.o'
void Level::Explode(Weapon & weapon, ConstVec3fr position)
{
	Explosion * explosion = new Explosion();
	explosion->weapon = weapon;
	explosion->position = position;
	explosions.AddItem(explosion);
}

List<Ship*> Level::GetShipsAtPoint(ConstVec3fr position, float maxRadius, List<float> & distances)
{
	List<Ship*> relevantShips;
	distances.Clear();
	float maxDist = maxRadius;
	for (int i = 0; i < ships.Size(); ++i)
	{
		Ship * ship = ships[i];
		if (ship->destroyed || !ship->spawned)
			continue;
		if (ship->entity == 0)
			continue;
		float dist = (ship->entity->position - position).Length();
		float radius = ship->entity->radius;
		float distMinRadius = dist - radius;
		if (distMinRadius > maxDist)
			continue;
		relevantShips.AddItem(ship);
		distances.AddItem(distMinRadius);
	}
	return relevantShips;
}

