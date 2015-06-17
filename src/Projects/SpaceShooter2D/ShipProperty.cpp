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
#include "Window/AppWindowManager.h"

#include "SpaceShooter2D.h"

ShipProperty::ShipProperty(Ship * ship, Entity * owner)
: EntityProperty("ShipProperty", ID(), owner), ship(ship)
{
	sleeping = false;
	spawnInvulnerability = true;
//	LoadDataFrom(ship);
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
	if (paused)
		return;

	/// o.o
	if (spawnInvulnerability)
	{
		if (owner->position.x < removeInvuln)
		{
			// Change color.
			QueueGraphics(new GMSetEntityTexture(owner, DIFFUSE_MAP | SPECULAR_MAP, TexMan.GetTextureByColor(Color(255,255,255,255))));
			spawnInvulnerability = false;
		}
	}
	// Move?
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
	if (!ship->canShoot)
		return;

	// Fire stuff?
	if (ship->ai)
	{
		// Do stuff.
		for (int i = 0; i < ship->weapons.Size(); ++i)
		{
			Weapon & weapon = ship->weapons[i];
			// Aim.
			weapon.Aim(ship);
			// Dude..
			if (projectileEntities.Size() > 1000)
				continue;
			weapon.Shoot(ship);
		}
	}
	else 
	{
		if (!ship->shoot)
			return;
		if (ship->activeWeapon == 0)
			ship->activeWeapon = &ship->weapons[0];
		// Shoot with current weapon for player.
		if (ship->activeWeapon)
			ship->activeWeapon->Shoot(ship);
	}
}
	

void ShipProperty::ProcessAI(int timeInMs)
{
	if (ship->rotationPatterns.Size() == 0)
		return;
	// Rotate accordingly.
	Rotation & rota = ship->rotationPatterns[ship->currentRotation];
	rota.OnFrame(timeInMs);
	// Increase time spent in this state accordingly.
	ship->timeInCurrentRotation += timeInMs;
	if (ship->timeInCurrentRotation > rota.durationMs && rota.durationMs > 0)
	{
		ship->currentRotation = (ship->currentRotation + 1) % ship->rotationPatterns.Size();
		ship->timeInCurrentRotation = 0;
		Rotation & rota2 = ship->rotationPatterns[ship->currentRotation];
		rota2.OnEnter(ship);
	}
	if (!ship->canMove)
		return;
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
//	std::cout<<"\nShipProperty::OnCollision for entity "<<owner->name;
	if (sleeping)
	{
//		std::cout<<"\nSleeping, skipping stuffs";
		return;
	}
	Entity * other = withEntity;

	ShipProperty * sspp = (ShipProperty *) other->GetProperty(ShipProperty::ID());
	// Player-player collision? Sleep 'em both.
	if (sspp)
	{
//		std::cout<<"\nCollision with ship! o.o";
		if (sspp->sleeping)
			return;
		// Check collision damage cooldown for if we should apply damage.
		if (ship->lastShipCollision < levelTime - ship->collisionDamageCooldown)
		{
			ship->Damage(sspp->ship->collideDamage, true);
			ship->lastShipCollision = levelTime;
		}
		// Same for the other ship.
		if (sspp->ship->lastShipCollision < levelTime - sspp->ship->collisionDamageCooldown)
		{
			sspp->ship->Damage(ship->collideDamage, false);
			sspp->ship->lastShipCollision = levelTime;
		}

		// Add a temporary emitter to the particle system to add some sparks to the collision
		Vector3f position = (owner->position + other->position) * 0.5f;
		SparksEmitter * tmpEmitter = new SparksEmitter();
		tmpEmitter->newType = true;
		tmpEmitter->positionEmitter.type = EmitterType::POINT;
		tmpEmitter->positionEmitter.vec = position;
		// Set up velocity emitter direction.
		tmpEmitter->velocityEmitter.type = EmitterType::LINE_BOX;
		Vector3f vec1 = (owner->position - other->position).NormalizedCopy().CrossProduct(Vector3f(0,0,1)).NormalizedCopy();
		tmpEmitter->velocityEmitter.vec = vec1;
		tmpEmitter->velocityEmitter.vec2 = vec1.CrossProduct(Vector3f(0,0,1)).NormalizedCopy() * 0.2f;
		
		tmpEmitter->SetRatioRandomVelocity(1.0f);
		float velocity = (owner->Velocity().Length() + other->Velocity().Length()) * 0.5f;
		velocity += 1.f;
		tmpEmitter->SetEmissionVelocity(velocity);
		tmpEmitter->constantEmission = 50;
		tmpEmitter->instantaneous = true;
		tmpEmitter->SetScale(0.15f);
		tmpEmitter->SetParticleLifeTime(1.5f);
		tmpEmitter->SetColor(Vector4f(1.f, 0.5f, 0.1f, 1.f));
		Graphics.QueueMessage(new GMAttachParticleEmitter(tmpEmitter, sparks));
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

bool ShipProperty::IsAllied()
{
	return ship->allied;
}

