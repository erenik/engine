/// Emil Hedemalm
/// 2014-08-01
/// Integrator dedicated to the MORPG-project.

#include "Physics/Integrator.h"

class MORPGIntegrator : public Integrator 
{
public:
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds);
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds);

private:
	void IntegrateVelocity(Entity * forEntity, float timeInSeconds);

};

