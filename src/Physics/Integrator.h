/// Emil Hedemalm
/// 2014-07-16
/// Physics integration class. Sublcass for custom behaviour.

#ifndef PHYSICS_INTEGRATOR_H
#define PHYSICS_INTEGRATOR_H

#include "Physics/PhysicsProperty.h"
#include "Entity/Entity.h"

void RecalculateMatrices(List< std::shared_ptr<Entity> > entities);

class Integrator 
{
public:
	Integrator();

	void IsGood();
	/** All entities sent here should be fully dynamic! 
		Kinematic ones may or may not work (consider adding own integration function).
	*/
	virtual void IntegrateDynamicEntities(List< std::shared_ptr<Entity> > & dynamicEntities, float timeInSeconds) = 0;
	/** All entities sent here should be fully kinematic! 
		If not subclassed, the standard IntegrateEntities is called.
	*/
	virtual void IntegrateKinematicEntities(List< std::shared_ptr<Entity> > & kinematicEntities, float timeInSeconds) = 0;
	
	/// -9.82 y by default
	Vector3f gravity;

	/// For 2D-based integrators, this holds the value where most entities will be placed or enforced to stay at, Z-wise.
	float constantZ;

	/// For performance calculation, should save time required for integration and recalculating entity matrices both.
	int integrationTimeMs;
	int entityMatrixRecalcMs;

	/// Called once from the PhysicsManager after integration completes. By default entities with parents are skipped, as the parent should trigger the recursive recalc.
	void RecalculateMatrices(List< std::shared_ptr<Entity> > & entities);
protected:
	/// Provides default "scientific" rigid-body based simulation handling of forces, torques, etc.
	void CalculateForces(List< std::shared_ptr<Entity> > & entities);
	void UpdateMomentum(List< std::shared_ptr<Entity> > & entities, float timeInSeconds);
	void DeriveVelocity(List< std::shared_ptr<Entity> > & entities);
private:
};

class NoIntegrator : public Integrator {
	virtual void IntegrateDynamicEntities(List< std::shared_ptr<Entity> >& dynamicEntities, float timeInSeconds);
	virtual void IntegrateKinematicEntities(List< std::shared_ptr<Entity> >& kinematicEntities, float timeInSeconds);
};


#endif


