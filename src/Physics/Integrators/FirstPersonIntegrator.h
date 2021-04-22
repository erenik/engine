/// Emil Hedemalm
/// 2014-08-06
/// An integrator suitable for a first-person game. Works well together with the FirstPersonCD, FirstPersonCR and FirstPersonPlayerProperty.

#ifndef FIRST_PERSON_INTEGRATOR_H
#define FIRST_PERSON_INTEGRATOR_H

#include "Physics/Integrator.h"

class FirstPersonIntegrator : public Integrator
{
public:
	FirstPersonIntegrator();
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List< Entity* > & dynamicEntities, float timeInSeconds);
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List< Entity* > & kinematicEntities, float timeInSeconds);

	// Default true
	bool applyGravity;

private:
	void IntegrateVelocity(List< Entity* > & forEntities, float timeInSeconds);
	void IntegratePosition(List< Entity* > & forEntities, float timeInSeconds);
	void RecalcMatrices(List< Entity* > & entities);
};

#endif
