/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#ifndef PHYSICS_INTEGRATOR_H
#define PHYSICS_INTEGRATOR_H

#include "Physics/PhysicsProperty.h"
#include "Entity/Entity.h"

class Integrator 
{
public:

	void IsGood();
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds) = 0;
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds) = 0;
	
private:
};

#endif


