/// Emil Hedemalm
/// 2015-01-21
/// Ship property

#include "ShipProperty.h"
#include "Entity/Entity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/PhysicsProperty.h"
#include "PhysicsLib/Shapes/Ray.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GraphicsMessage.h"

#include "Input/InputManager.h"
#include "Window/WindowManager.h"

#include "SpaceShooter2D.h"

ShipProperty::ShipProperty(Ship * ship, Entity * owner)
: EntityProperty("ShipProperty", ID(), owner), ship(ship)
{
	sleeping = false;
}

int ShipProperty::ID()
{
	return SHIP_PROP;
}

void ShipProperty::Remove()
{
	if (sleeping)
		return;
	Graphics.QueueMessage(new GMUnregisterEntity(owner));
	Physics.QueueMessage(new PMUnregisterEntity(owner));
	sleeping = true;
}
	
/// D:
void ShipProperty::Destroy()
{
	if (sleeping)
		return;
	ship->hitPoints = 0;
	// Remove it.. o.o'
	Remove();
	if (ship->allied)
	{
		spaceShooter->UpdatePlayerHP();
	}
	else {
		spaceShooter->score += ship->score;
		spaceShooter->OnScoreUpdated();
	}	
	// Update the game that we died. T_T
	spaceShooter->OnShipDestroyed(ship);
}

// Reset sleep.
void ShipProperty::OnSpawn()
{
	sleeping = false;
}
	

/// Time passed in seconds..!
void ShipProperty::Process(int timeInMs)
{
	if (sleeping)
		return;

	// Move?
	Entity * shipEntity = ship->entity;
	// Don't process inactive ships..
	if (ship->ai == true)
	{
		// Mainly for movement and stuff
		ProcessAI();
	}
	ProcessWeapons(timeInMs);
}

void ShipProperty::ProcessWeapons(int timeInMs)
{
	// Fire stuff?
	if (ship->canShoot)
	{
		// Do stuff.
		for (int i = 0; i < ship->weapons.Size(); ++i)
		{
			// Dude..
			if (spaceShooter->projectileEntities.Size() > 500)
				continue;
			Weapon & weapon = ship->weapons[i];
			assert(weapon.cooldownMs != 0);
			/// Initialize the weapon as if it had just been fired.
			if (weapon.lastShotMs == 0)
				weapon.lastShotMs = nowMs;
			int timeDiff = nowMs - weapon.lastShotMs;
			if (timeDiff > weapon.cooldownMs)
			{
				weapon.Shoot(ship);
			}
		}
	}
}
	

void ShipProperty::ProcessAI()
{
	// Move?
	Entity * shipEntity = ship->entity;
	if (ship->movementPattern == "Straight forward")
	{
		if (!shipEntity->Velocity().LengthSquared())
		{
			Physics.QueueMessage(new PMSetEntity(shipEntity, PT_VELOCITY, Vector3f(-1,0,0) * ship->speed));
		}
	}
}


/// If reacting to collisions...
void ShipProperty::OnCollision(Collision & data)
{
	if (sleeping)
		return;
	// Check what we are colliding with.
	Entity * other = 0;
	if (data.one == owner)
		other = data.two;
	else if (data.two == owner)
		other = data.one;

	ShipProperty * sspp = (ShipProperty *) other->GetProperty(ShipProperty::ID());
	// Player-player collision? Sleep 'em both.
	if (sspp && !sspp->sleeping)
	{
		ship->Damage(sspp->ship->hitPoints);
		sspp->ship->Damage(ship->hitPoints);
		return;
	}

	ProjectileProperty * pp = (ProjectileProperty *) other->GetProperty(ProjectileProperty::ID());
	if (pp && !pp->sleeping)
	{
		// Take damage? D:
		ship->Damage(pp->weapon.damage);
	}
}
	