/// Emil Hedemalm
/// 2014-07-30
/// Custom physics integrator for the TIFS game, since no general integrator exists in this engine (yet). Requires more fine-tuning! 

#include "Physics/Integrators/FirstPersonIntegrator.h"

class TIFSIntegrator : public FirstPersonIntegrator
{
public:
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
//	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds);

	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
//	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds);

private:

	void IntegrateVelocity(Entity * forEntity, float timeInSeconds);
};
