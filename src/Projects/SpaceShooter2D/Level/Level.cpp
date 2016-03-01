/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "../SpaceShooter2D.h"
#include "SpawnGroup.h"
#include "Text/TextManager.h"
#include "LevelMessage.h"
#include "Explosion.h"
#include "File/LogFile.h"
#include "Message/MathMessage.h"
#include "StateManager.h"
#include "../Properties/ExplosionProperty.h"

#define SPAWNED_ENEMIES_LOG "SpawnedEnemies.srl"

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
SpawnGroup testGroup;
List<SpawnGroup> storedTestGroups;

Level::Level()
{
	testGroup.number = 1;

	height = 20.f;
	endCriteria = NO_MORE_ENEMIES;
	levelCleared = false;
	activeLevelMessage = 0;
}

Level::~Level()
{
	spawnGroups.ClearAndDelete();
	messages.ClearAndDelete();
	enemyShips.ClearAndDelete();
	alliedShips.ClearAndDelete();
}

/// Starts BGM, starts clocks/timers if any, etc.
void Level::OnEnter()
{
	// Play music?
	if (music.Length())
	{
		AudioMan.QueueMessage(new AMPlay(AudioType::BGM, music, 1.f));
	}
}

// Used for player and camera. Based on millisecondsPerPixel.
Vector3f Level::BaseVelocity()
{
	return Vector3f(1,0,0) * (1000.f / millisecondsPerPixel);
}

/// Creates player entity within this level. (used for spawning)
Entity * Level::AddPlayer(Ship * playerShip, ConstVec3fr atPosition)
{	
	Entity * entity = playerShip->Spawn(atPosition, 0);
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


void Level::Cleanup()
{
	/// Remove projectiles which have been passed by.
	for (int i = 0; i < projectileEntities.Size(); ++i)
	{
		Entity * proj = projectileEntities[i];
		ProjectileProperty * pp = (ProjectileProperty*) proj->GetProperty(ProjectileProperty::ID());
		if (pp->sleeping || 
				(proj->worldPosition[0] < despawnPositionLeft ||
				proj->worldPosition[0] > spawnPositionRight ||
				proj->worldPosition[1] < -1.f ||
				proj->worldPosition[1] > playingFieldSize[1] + 2.f
				)
			)
		{
			MapMan.DeleteEntity(proj);
			projectileEntities.Remove(proj);
			--i;
		}
	}

	/// Clean ships.
	for (int i = 0; i < ships.Size(); ++i)
	{
		Ship * ship = ships[i];
		if (ship->destroyed)
			continue;
		if (!ship->spawned)
			continue;
		if (!ship->entity)
			continue;
		// Check if it should de-spawn.
		if (ship->despawnOutsideFrame && ship->entity->worldPosition[0] < despawnPositionLeft && ship->parent == NULL)
		{
			ship->Despawn(false);
		}
	}

}

void Level::Process(int timeInMs)
{
	activeLevel = this;

	removeInvuln = levelEntity->worldPosition[0] + playingFieldHalfSize[0] + playingFieldPadding + 1.f;
	spawnPositionRight = removeInvuln + 15.f;
	despawnPositionLeft = levelEntity->worldPosition[0] - playingFieldHalfSize[0] - 1.f;

	// Check for game over.
	if (playerShip->hp <= 0)
	{
		LogMain("Game over! Player HP 0", INFO);
		// Game OVER!
		if (onDeath.Length() == 0)
			spaceShooter->GameOver();
		else if (onDeath.StartsWith("RespawnAt"))
		{
			playerShip->hp = (float)playerShip->maxHP;
			this->AddPlayer(playerShip, Vector3f(levelEntity->worldPosition.x, 10.f, 0));
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

	/// Process active explosions. No.
	/*
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
	*/

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
			if (sg->FinishedSpawning())
			{
				// Check if it's defeated?
				if (sg->DefeatedOrDespawned())
				{
					if (sg->pausesGameTime)
						gameTimePaused = false;
				}
				continue;
			}
			int msToSpawn = (int) (sg->spawnTime - levelTime).Milliseconds();
			if (msToSpawn < 0) 
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
		case MessageType::SET_STRING:
		{
			String value = ((SetStringMessage*)message)->value;
			if (msg == "DropDownMenuSelection:ShipTypeToSpawn")
			{
				testGroup.shipType = value;
			}
			else if (msg == "DropDownMenuSelection:SpawnFormation")
			{
				testGroup.formation = Formation::GetByName(value);
			}
			break;
		}
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			if (msg == "SetTestEnemiesAmount")
			{
				testGroup.number = im->value;
			}
			break;	
		}
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*)message;
			if (msg == "SetFormationSize")
			{
				testGroup.size = vm->GetVector2f();
			}
			else if (msg == "SetSpawnLocation")
			{
				testGroup.position = vm->GetVector3f();
			}
			break;	
		}
		case MessageType::STRING:
		{
 			if (msg.StartsWith("GenerateLevel"))
			{
				GenerateLevel(msg);
			}
			if (msg == "ListPhysicalEntities")
			{
				Entities entities = PhysicsMan.GetEntities();
				for (int i = 0; i < entities.Size(); ++i)
				{
					std::cout<<"\nPhysical entities: "<<entities[i]->name<<" "<<entities[i]->worldPosition;
				}
/*				GraphicsMan.Re
				for (int i = 0; i < entities.Size(); ++i)
				{
					std::cout<<"\nPhysical entities: "<<entities[i]->name;
				}*/
			}
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

			else if (msg == "SetupForTesting")
			{
				File::ClearFile(SPAWNED_ENEMIES_LOG);
				// Disable game-over/dying/winning
				Clear();
				levelTime.intervals = 0;
			}
			else if (msg.StartsWith("ShipTypeToSpawn:"))
			{
				testGroup.shipType = msg.Tokenize(":")[1];
			}
			else if (msg == "StoreTestEnemies")
			{
				storedTestGroups.AddItem(testGroup);
	//			QueueGraphics(new GMSetUIs("StoreTestEnemies", GMUI::TEXT, "Store for spawning ("+String(storedTestGroups.Size())+")"));
			}
			else if (msg == "SpawnTestEnemies")
			{
				storedTestGroups.AddItem(testGroup);
//				QueueGraphics(new GMSetUIs("StoreTestEnemies", GMUI::TEXT, "Store for spawning", ));
				for (int i = 0; i < storedTestGroups.Size(); ++i)
				{
					SpawnGroup sg = storedTestGroups[i];
					String str = sg.GetLevelCreationString(flyTime);
					File::AppendToFile(SPAWNED_ENEMIES_LOG, str);
					LogMain(str, INFO);
					sg.Spawn();
				}
				storedTestGroups.Clear();
			}
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
//	ferewr
	switch(endCriteria)
	{
		case NEVER:
			return false;
		case NO_MORE_ENEMIES:
			if (levelTime.Seconds() < 3)
				return false;
			if (shipEntities.Size() > this->PlayerShips().Size())
				return false;
			if (!FinishedSpawning())
				return false;
			if (messages.Size())
				return false;
			return true;
		case EVENT_TRIGGERED:
			return levelCleared;
		default:
			return false;
	}
	return false;
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
		float dist = (e->worldPosition - position).LengthSquared();
		if (dist < closestDist)
		{
			closest = e;
			closestDist = dist;
		}
	}
	return closest;
}

#include "PhysicsLib/EstimatorFloat.h"

/// o.o'
void Level::Explode(Weapon & weapon, Entity * causingEntity, bool enemy)
{
	Entity * explosionEntity = EntityMan.CreateEntity("ExplosionEntity", ModelMan.GetModel("Sphere"), 0 /*TexMan.GetTexture("0xFFFF")*/);
	ExplosionProperty * explosionProperty = new ExplosionProperty(weapon, explosionEntity);
	explosionEntity->properties.AddItem(explosionProperty);
	explosionProperty->weapon = weapon;
	explosionProperty->enemy = enemy;
	explosionProperty->player = !enemy;
	explosionProperty->duration = 1000; // MS.

	PhysicsProperty * prop = new PhysicsProperty();
	prop->collisionCategory = enemy? CC_ENEMY_EXPL : CC_PLAYER_EXPL;
	prop->collisionFilter = enemy? CC_ALL_PLAYER : CC_ALL_ENEMY;
	prop->collisionCallback = true;
	prop->physicalRadius = 5.f;
	prop->velocity = causingEntity->Velocity() * 0.5f;
	prop->type = PhysicsType::DYNAMIC;
	prop->noCollisionResolutions = true;
	explosionEntity->physics = prop;

	explosionEntity->localPosition = causingEntity->worldPosition;
	explosionEntity->RecalculateMatrix();

	/// Add physics thingy straight away.
	EstimatorFloat * estimator = new EstimatorFloat();
	estimator->variablesToPutResultTo.Add(&explosionEntity->scale.x, &explosionEntity->scale.y, &explosionEntity->scale.z);
	estimator->AddStateMs(0.1, 0);
	estimator->AddStateMs(weapon.explosionRadius, explosionProperty->duration);
	prop->estimationEnabled = true;
	prop->estimators.AddItem(estimator);
	MapMan.AddEntity(explosionEntity, false, true);
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
		float dist = (ship->entity->worldPosition - position).Length();
		float radius = ship->entity->radius;
		float distMinRadius = dist - radius;
		if (distMinRadius > maxDist)
			continue;
		relevantShips.AddItem(ship);
		distances.AddItem(distMinRadius);
	}
	return relevantShips;
}

void Level::RemoveRemainingSpawnGroups()
{
	for (int i = 0; i < spawnGroups.Size(); ++i)
	{
		SpawnGroup * sg = spawnGroups[i];
		sg->SetFinishedSpawning();
	}
}

void Level::RemoveExistingEnemies()
{
	String lg;
	for (int i = 0; i < ships.Size(); ++i)
	{
		Ship * ship = ships[i];
		lg += ship->name+" ";
		ship->Despawn(false);
	}
	LogMain("Deleting entities "+lg, INFO);

	Sleep(50);
	for (int i = 0; i < ships.Size(); ++i)
	{
		Ship * s = ships[i];
		s->spawnGroup = 0;
	}
	ships.ClearAndDelete();
}

void Level::JumpToTime(String timeString)
{
	// Jump to target level-time. Adjust position if needed.
	levelTime.ParseFrom(timeString);
	for (int i = 0; i < spawnGroups.Size(); ++i)
	{
		SpawnGroup * sg = spawnGroups[i];
		if (sg->spawnTime < levelTime)
		{
			sg->SetFinishedSpawning();
			sg->SetDefeated();
		}
	}
	OnLevelTimeAdjusted();
}

List<Ship*> Level::PlayerShips()
{
	List<Ship*> playerShips;
	playerShips.AddItem(playerShip);
	return playerShips;
}
