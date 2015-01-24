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
		ProcessAI(timeInMs);
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
			weapon.Shoot(ship);
		}
	}
}
	

void ShipProperty::ProcessAI(int timeInMs)
{
	// Move?
	Entity * shipEntity = ship->entity;
	Movement & move = ship->movementPatterns[ship->currentMovement];
	move.OnFrame(timeInMs);
	// Increase time spent in this state accordingly.
	ship->timeInCurrentMovement += timeInMs;
	if (ship->timeInCurrentMovement > move.durationMs && move.durationMs > 0)
	{
		ship->currentMovement = (ship->currentMovement + 1) % ship->movementPatterns.Size();
		ship->timeInCurrentMovement = 0;
		Movement & newMove = ship->movementPatterns[ship->currentMovement];
		newMove.OnEnter(ship);
	}
}


/// If reacting to collisions...
void ShipProperty::OnCollision(Collision & data)
{
	// Check what we are colliding with.
	Entity * other = 0;
	if (data.one == owner)
		other = data.two;
	else if (data.two == owner)
		other = data.one;

	// Here you may generate some graphics effects if you want, but other than that.. don't do anything that has to do with gameplay logic.
}	

/// If reacting to collisions...
void ShipProperty::OnCollision(Entity * withEntity)
{
	if (sleeping)
		return;
	Entity * other = withEntity;

	ShipProperty * sspp = (ShipProperty *) other->GetProperty(ShipProperty::ID());
	// Player-player collision? Sleep 'em both.
	if (sspp)
	{
//		std::cout<<"\nCollision with ship! o.o";
		if (sspp->sleeping)
			return;
		ship->Damage(sspp->ship->hitPoints, true);
		sspp->ship->Damage(ship->hitPoints, false);
		return;
	}

	ProjectileProperty * pp = (ProjectileProperty *) other->GetProperty(ProjectileProperty::ID());
	if (pp && !pp->sleeping)
	{
		// Take damage? D:
		ship->Damage(pp->weapon.damage, false);
		pp->Destroy();
	}
}
