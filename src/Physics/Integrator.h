/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#ifndef PHYSICS_INTEGRATOR_H
#define PHYSICS_INTEGRATOR_H

#include "Physics/PhysicsProperty.h"
#include "Entity/Entity.h"

void RecalculateMatrices(List<Entity*> entities);

class Integrator 
{
public:
	Integrator();

	void IsGood();
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List<Entity*> & dynamicEntities, float timeInSeconds) = 0;
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List<Entity*> & kinematicEntities, float timeInSeconds) = 0;
	
	/// -9.82 y by default
	Vector3f gravity;

	/// For 2D-based integrators, this holds the value where most entities will be placed or enforced to stay at, Z-wise.
	float constantZ;

	/// For performance calculation, should save time required for integration and recalculating entity matrices both.
	int integrationTimeMs;
	int entityMatrixRecalcMs;

protected:
	/// Provides default "scientific" rigid-body based simulation handling of forces, torques, etc.
	void CalculateForces(List<Entity*> & entities);
	void UpdateMomentum(List<Entity*> & entities, float timeInSeconds);
	void DeriveVelocity(List<Entity*> & entities);

private:
};

#endif


