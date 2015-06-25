/// Emil Hedemalm
/// 2015-03-03
/// Spawn group, yo.

#include "SpaceShooter2D.h"
#include "SpawnGroup.h"
#include "Level.h"
#include "Entity/EntityManager.h"
#include "TextureManager.h"
#include "Model/ModelManager.h"

String Formation::GetName(int forFormationType)
{
	switch(forFormationType)
	{
		case LINE_X: return "LINE_X";
		case LINE_Y: return "LINE_Y";
		case LINE_XY: return "LINE_XY";
		case V_X: return "V_X";
		case V_Y: return "V_Y";
		default:
			assert(false);
	}
}

SpawnGroup::SpawnGroup()
{
	spawnTime = Time(TimeType::MILLISECONDS_NO_CALENDER);
	spawned = false;
}

void SpawnGroup::Spawn()
{
	spawned = true;
	/// Fetch amount.
	Vector3f offsetPerSpawn;
	int offsetNum = max(1,number - 1);
	// Always nullify Z.
	size.z = 0;
	switch(formation)
	{
		case Formation::LINE_X:
			size.y = 0; // Nullify if bad.
			break;
		case Formation::LINE_XY:
			break;
		case Formation::LINE_Y:
			size.x = 0; // Nullify if bad.
			break;
		case Formation::V_X:
		case Formation::V_Y:
			break;
	}
	offsetPerSpawn = size / (float) offsetNum;
	
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
		}	
		position += offsetPerSpawn * (float)offsetIndex;
		// Flip accordingly
		switch(formation)
		{
			case Formation::V_X:
				if ((i + 1) % 2 == 0)
					position.y *= -1;
				break;
			case Formation::V_Y:
				if ((i + 1) % 2 == 0)
					position.x *= -1;
		}

		/// Add current position offset to it.
		position += Vector3f(spawnPositionRight, activeLevel->height * 0.5f, 0); 
		/// Add group position offset (if any)
		position += this->groupPosition;
		
		/// Create entity
		SpawnShip(position);
	}
}

	// ?!
Entity * SpawnGroup::SpawnShip(ConstVec3fr atPosition)
{
	Ship * newShip = Ship::New(shipType);
	activeLevel->ships.AddItem(newShip);
	Ship * ship = newShip;

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



