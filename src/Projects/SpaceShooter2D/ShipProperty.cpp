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
	ship->Process(timeInMs);
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
		// Here you may generate some graphics effects if you want, but other than that.. don't do anything that has to do with gameplay logic.
		if (ship->onCollision.Length())
		{
			List<String> stuff = ship->onCollision.Tokenize("&");
			for (int i = 0; i < stuff.Size(); ++i)
			{
				String s = stuff[i];
				if (s == "RemoveThis")
				{
					ship->hp = 0;
					ship->Despawn();
					return;
				}
				else 
				{
					MesMan.QueueMessages(s);
				}
			}
		}



//		std::cout<<"\nCollision with ship! o.o";
		if (sspp->sleeping)
			return;
		// Check collision damage cooldown for if we should apply damage.
		if (ship->lastShipCollision < flyTime - ship->collisionDamageCooldown)
		{
			ship->Damage(sspp->ship->collideDamage, true);
			ship->lastShipCollision = flyTime;
		}
		// Same for the other ship.
		if (sspp->ship->lastShipCollision < flyTime - sspp->ship->collisionDamageCooldown)
		{
			sspp->ship->Damage(ship->collideDamage, false);
			sspp->ship->lastShipCollision = flyTime;
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

