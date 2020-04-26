/// Emil Hedemalm
/// 2014-07-26
/// Custom integrator for a flight-simulator game, focusing on aerical movement and velocities changing due to rotation.

#include "Physics/Integrator.h"

class AircraftIntegrator : public Integrator 
{
public:
	AircraftIntegrator();
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List< std::shared_ptr<Entity> > dynamicEntities, float timeInSeconds);
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List< std::shared_ptr<Entity> > kinematicEntities, float timeInSeconds);

private:
	virtual void IntegrateVelocity(EntitySharedPtr forEntity, float timeInSeconds);
};


