/// Emil Hedemalm
/// 2013-10-02

#include "Collision.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"

Collision::Collision()
{
	results = 0; 
	one = two = NULL; 
	resolved = false;
};

/// Fills the lists of involved entities.
void Collision::ExtractData()
{
	ExtractEntityData(one);
	ExtractEntityData(two);
}

void Collision::ExtractEntityData(Entity* entity)
{
	switch(entity->physics->type)
	{
		case PhysicsType::DYNAMIC:
			dynamicEntities.Add(entity);
			break;
		case PhysicsType::KINEMATIC:
			kinematicEntities.Add(entity);
			break;
		case PhysicsType::STATIC:
			staticEntities.Add(entity);
			break;
	}
}
