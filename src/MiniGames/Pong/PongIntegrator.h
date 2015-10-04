/// Emil Hedemalm
/// 2014-07-16
/// Custom integrator for pong.

#include "Physics/Integrator.h"

class PongIntegrator : public Integrator 
{
public:
	PongIntegrator(float zPlane);
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds);
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds);
	// If non-0, will enforce a constant Z on all entities.
	float constantZ;

	Vector2f frameMin, frameMax;

private:
	virtual void IntegrateVelocity(Entity * forEntity, float timeInSeconds);
};


