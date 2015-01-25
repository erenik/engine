// Emil Hedemalm
// 2013-03-23

#include "../Entity/Entity.h"
#include "Collision.h"
#include "../PhysicsProperty.h"

/// Updates the entity's collission state (colliding, in rest, on plane) using it's current velocities and the collission normal.
void UpdateCollisionState(Entity * entity, Vector3f & collisionNormal){
    if (entity->physics->velocity.Length() < 0.001f &&
		entity->physics->acceleration.Length() < ZERO &&
		collisionNormal[1] > 0.90f){
		entity->physics->state &= ~PhysicsState::COLLIDING;	// De-flag COLLIDING
		entity->physics->state |= PhysicsState::IN_REST;	// Flag IN_REST
		entity->physics->velocity = Vector3f();
	}
	else {
		entity->physics->state &= ~PhysicsState::IN_REST;	// De-flag IN_REST
		entity->physics->state |= PhysicsState::COLLIDING;	// Flag COLLIDING
	}
}
