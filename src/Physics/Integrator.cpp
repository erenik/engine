/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#include "Integrator.h"
#include "Physics/Springs/Spring.h"

void RecalculateMatrices(List<Entity*> entities)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		entity->RecalculateMatrix(Entity::TRANSLATION_ONLY);
	}
}


Integrator::Integrator()
{
	constantZ = 0.f;
	gravity = Vector3f(0, -9.82f, 0);
}

void Integrator::IsGood()
{
	// std::cout<<"\nAll is well.. o.o";
}

void Integrator::CalculateForces(List<Entity*> & entities)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		PhysicsProperty * pp = entity->physics;
		pp->totalForce = Vector3f();

		for (int i = 0; i < pp->springs.Size(); ++i)
		{
			Spring * spring = pp->springs[i];
			Vector3f springForce = spring->GetForce(entity);
			pp->totalForce += springForce;
		}

		/// Add gravitation!
		pp->totalForce += gravity * pp->mass * pp->gravityMultiplier;
	}
}
void Integrator::UpdateMomentum(List<Entity*> & entities, float timeInSeconds)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		PhysicsProperty * pp = entity->physics;
		pp->linearMomentum += pp->totalForce * timeInSeconds;
		pp->angularMomentum += pp->totalTorque * timeInSeconds;

		/// Apply some damping
		pp->linearMomentum *= pp->linearDampingPerPhysicsFrame;
		pp->angularMomentum *= pp->angularDampingPerPhysicsFrame;

	}

}
void Integrator::DeriveVelocity(List<Entity*> & entities)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		/// Recalculate 'auxiliary variables'...
		PhysicsProperty * pp = entity->physics;
		pp->velocity = pp->linearMomentum * pp->inverseMass;
	}
}
