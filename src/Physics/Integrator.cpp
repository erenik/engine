/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#include "Integrator.h"

void RecalculateMatrices(List<Entity*> entities)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		entity->RecalculateMatrix(false);
	}
}


Integrator::Integrator()
{
	constantZ = 0.f;
}

void Integrator::IsGood()
{
	// std::cout<<"\nAll is well.. o.o";
}