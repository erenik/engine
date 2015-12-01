/// Emil Hedemalm
/// 2015-03-03
/// Spawn group, yo.

#include "../SpaceShooter2D.h"
#include "SpawnGroup.h"
#include "Level.h"
#include "Entity/EntityManager.h"
#include "TextureManager.h"
#include "Model/ModelManager.h"
#include "File/LogFile.h"

String Formation::GetName(int forFormationType)
{
	switch(forFormationType)
	{
		case BAD_FORMATION: return "BAD_FORMATION";
		case LINE_X: return "LINE_X";
		case DOUBLE_LINE_X: return "DOUBLE_LINE_X";
		case LINE_Y: return "LINE_Y";
		case LINE_XY: return "LINE_XY";
		case X: return "X";
		case SQUARE: return "SQUARE";
		case CIRCLE: return "CIRCLE";
		case HALF_CIRCLE_LEFT: return "HALF_CIRCLE_LEFT";
		case HALF_CIRCLE_RIGHT: return "HALF_CIRCLE_RIGHT";
		case V_X: return "V_X";
		case V_Y: return "V_Y";
		case SWARM_BOX_XY: return "SWARM_BOX_XY";
		default:
			assert(false);
	}
}

int Formation::GetByName(String name)
{
	for (int i = 0; i < Formation::FORMATIONS; ++i)
	{
		String n = GetName(i);
		if (n == name)
			return i;
	}
	return 0;
}


SpawnGroup::SpawnGroup()
{
	spawnTime = Time(TimeType::MILLISECONDS_NO_CALENDER);
	pausesGameTime = false;
	Reset();
}

void SpawnGroup::Reset()
{
	relativeSpeed = 5.f;
	shoot = true;
	spawned = false;
	defeated = false;
	survived = false;
	shipsDefeatedOrDespawned = 0;
	shipsDespawned = shipsDefeated = 0;
}


Random spawnGroupRand;
void SpawnGroup::Spawn(bool subAggregate)
{
	if (!subAggregate)
		LogMain("Spawning spawn group at time: "+String(spawnTime.ToString("m:S.n")), INFO);

	spawned = true;
	std::cout<<"\nSpawning formation: "<<Formation::GetName(formation);

	switch(formation)
	{
		case Formation::SQUARE:
			/// Number will be the amount of ships per side?
			SpawnGroup copy = *this;
			int num = number;
			Vector2f spacing = size / (num - 1);
			Vector2f lineSize = spacing * (num - 2);
			// Bottom
			copy.formation = Formation::LINE_X;
			copy.number = num - 1;
			copy.groupPosition = groupPosition + Vector2f(-size.x * 0.5f, size.y * 0.5f);
			copy.size = Vector2f(lineSize.x, 0); 
			copy.Spawn(true);
			copy.groupPosition = groupPosition + Vector2f(-size.x * 0.5f + spacing.x, - size.y * 0.5f);
			copy.Spawn(true);
			copy.groupPosition = groupPosition + Vector2f(size.x * 0.5f, spacing.y * 0.5f);
			copy.formation = Formation::LINE_Y;
			copy.size = Vector2f(0, -lineSize.y);
			copy.Spawn(true);
			copy.groupPosition = groupPosition + Vector2f(-size.x * 0.5f, - spacing.y * 0.5f);
			copy.size = Vector2f(0, lineSize.y);
			copy.Spawn(true);
			return;
	}

	/// Fetch amount.
	Vector3f offsetPerSpawn;
	int offsetNum = max(1,number - 1);
	// Always nullify Z.
	size.z = 0;
	switch(formation)
	{
		case Formation::LINE_XY:
			break;
		case Formation::V_X:
		case Formation::V_Y:
			break;
		case Formation::SWARM_BOX_XY:
			break;
		default:
			;//std::cout<<"\nImplement";
	}
	offsetPerSpawn = size / (float) offsetNum;

	/// Adjust offsetPerSpawn accordingly
	switch(formation)
	{
		case Formation::V_X:
			offsetPerSpawn.x = size.x / (floor((number) / 2.0 + 0.5) - 1);
			break;
		case Formation::V_Y:
			offsetPerSpawn.y = size.y / (floor((number) / 2.0 + 0.5) - 1);
			break;
		case Formation::X:
		{
			int steps = floor((number - 1) / 4.0);
			if (steps < 1)
				return;
			offsetPerSpawn = size * 0.5f / steps;
			break;
		}
		case Formation::SWARM_BOX_XY:
			offsetPerSpawn.y = 0; // Spawn erratically in Y only. Have some X offset each spawn?
			break;
		case Formation::LINE_X:
		case Formation::DOUBLE_LINE_X:
			offsetPerSpawn.y = 0;
			break;
		case Formation::LINE_Y:
			offsetPerSpawn.x = 0;
			break;
	}

	/// Check formation to specify vectors for all
	List<Vector3f> positions;
	for (int i = 0; i < number; ++i)
	{

		Vector3f position;
		int offsetIndex = i;
		// Just add offset?
		switch(formation)
		{
			// for them half-way-flippers.
			case Formation::V_X:
			case Formation::V_Y:
				offsetIndex = (i + 1)/ 2;
				break;
			case Formation::X:
				offsetIndex = (i + 3) / 4;
				break;
			case Formation::LINE_X:
//				position.y += size.y * 0.5f;
				break;
			case Formation::LINE_XY:
			case Formation::LINE_Y:
			case Formation::SWARM_BOX_XY:
			case Formation::DOUBLE_LINE_X:
				position.y -= size.y * 0.5f; // Center it.
				break;
			default:
				;// std::cout<<"\nImplement";
		}	
		position += offsetPerSpawn * (float)offsetIndex;
		bool angles = false;
		float degrees = 0;
		float startDegree = 0;
		Vector2f radialMultiplier(1,1);
		// Flip/randomize accordingly, 
		switch(formation)
		{
			case Formation::V_X:
				if ((i + 1) % 2 == 0)
					position.y *= -1;
				break;
			case Formation::V_Y:
				if ((i + 1) % 2 == 0)
					position.x *= -1;
				break;
			case Formation::X:
				/// If .. stuff
				if ((i + 1) % 4 < 2)
					position.x *= -1;
				if ((i + 1) % 2 == 0)
					position.y *= -1;
				break;
			case Formation::SWARM_BOX_XY:
				position.y += spawnGroupRand.Randf(size.y);
				break;
			case Formation::DOUBLE_LINE_X:
				break;
			default:
				break;//std::cout<<"\nImplement";
			case Formation::CIRCLE:
				angles = true;
				degrees = 360.0 / number;
				startDegree = 180;
				break;
			case Formation::HALF_CIRCLE_LEFT:
				angles = true;
				degrees = 180.0 / (number - 1);
				startDegree = 90;
				radialMultiplier.x = 2;
				break;
			case Formation::HALF_CIRCLE_RIGHT:
				angles = true;
				degrees = 180.0 / (number - 1);
				startDegree = -90;
				radialMultiplier.x = 2;
				break;
		}
		/// Radial spawning.
		if (angles)
		{
			position = Vector3f();
			Angle ang = Angle::FromDegrees(degrees * i + startDegree);
			Vector2f angXY = ang.ToVector2f();
			position += angXY * size * 0.5f * radialMultiplier;
		}

		/// Center it - wat?
//		position.y -= size.y * 0.5f;

		/// Add group position offset (if any)
		position += this->groupPosition;

		std::cout<<"\nSpawning ship @ x"<<position.x<<" y"<<position.y;

		/// Add current position offset to it.
		position += Vector3f(spawnPositionRight, activeLevel->height * 0.5f, 0); 
		/// Create entity
		SpawnShip(position);
		if (formation == Formation::DOUBLE_LINE_X)
		{
			SpawnShip(position + Vector3f(0, size.y, 0));
		}
	}
}

// ?!
Entity * SpawnGroup::SpawnShip(ConstVec3fr atPosition)
{
	Ship * newShip = Ship::New(shipType);
	activeLevel->ships.AddItem(newShip);
	Ship * ship = newShip;
	ship->RandomizeWeaponCooldowns();
	ship->spawnGroup = this;

	/// Apply spawn group properties.
	ship->shoot &= shoot;
	ship->speed *= relativeSpeed;

	Entity * entity = EntityMan.CreateEntity(ship->type, ship->GetModel(), TexMan.GetTextureByColor(Color(0,255,0,255)));
	entity->position = atPosition;
	float radians = PI / 2;
//		entity->rotation[0] = radians; // Rotate up from Z- to Y+
//		entity->rotation[1] = radians; // Rorate from Y+ to X-
	entity->SetRotation(Vector3f(radians, radians, 0));
	entity->RecalculateMatrix();
	
	PhysicsProperty * pp = new PhysicsProperty();
	entity->physics = pp;
	// Setup physics.
	pp->type = PhysicsType::DYNAMIC;
	pp->collisionCategory = CC_ENEMY;
	pp->collisionFilter = CC_PLAYER | CC_PLAYER_PROJ;
	pp->collisionCallback = true;
	// By default, set invulerability on spawn.
	ship->spawnInvulnerability = true;
	ShipProperty * sp = new ShipProperty(ship, entity);
	entity->properties.Add(sp);
	ship->entity = entity;
	ship->spawned = true;
	shipEntities.Add(entity);
	MapMan.AddEntity(entity);
	ship->StartMovement();
	return entity;
}

void SpawnGroup::OnShipDestroyed(Ship * ship)
{
	++shipsDefeated;
	++shipsDefeatedOrDespawned;
	if (shipsDefeated >= number)
	{
		defeatedAllEnemies = true;
		defeated = true;
		survived = true;
		if (pausesGameTime)
			gameTimePaused = false;
	}
}

void SpawnGroup::OnShipDespawned(Ship * ship)
{
	++shipsDespawned;
	++shipsDefeatedOrDespawned;
	if (shipsDefeatedOrDespawned >= number)
	{
		defeatedAllEnemies = false;
		survived = true;
		if (pausesGameTime)
			gameTimePaused = false;
	}
}

void SpawnGroup::ParseFormation(String fromString)
{
	for (int i = 0; i < Formation::FORMATIONS; ++i)
	{
		String name = Formation::GetName(i);
		if (fromString == name)
		{
			formation = i;
			return;
		}
	}
}



