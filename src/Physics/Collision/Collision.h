// Emil Hedemalm
// 2013-03-19

#ifndef COLLISSION_STRUCT_H
#define COLLISSION_STRUCT_H

#define NORMAL_ONLY						0x00000000
#define DISTANCE_INTO					0x00000001
#define COLLISSION_POINT				0x00000002
#define COLLISSION_VELOCITY				0x00000004
#define STATIC_DYNAMIC					0x00000008
#define PRELIMINARY_COLLISSION_NORMAL	0x00000010
#define RESOLVED						0x00000020

/// Trigger events, for setting up the entity's physics properties accordingly.
enum collisionCallbackRequirements{
	DISABLED,
	NO_REQUIREMENT, // Will spam every single collission as a message!
	IMPACT_VELOCITY, // Requires certain velocity into impact normal
#ifdef USE_MASS
	IMPACT_ENERGY,	// Requires certain impact energy to report collissions
#endif
	COLLISSION_CALLBACK_TYPES
};

//#include "../PhysicsLib.h"
#include "PhysicsLib/Shapes/Triangle.h"
#include "../MathLib.h"
#include "Entity/Entity.h"

struct CollisionResolution {
	Vector3f deltaLinearMomentum[2];
	Vector3f deltaAngularMomentum[2];
	/// Matrices before resolution, for comparison's sake.
	Matrix4f onePreResolution;
	Matrix4f twoPreResolution;
};

// A collission result struct for eased collission resolving
struct Collision 
{
	Collision();

	// The only guaranteed result from a collission. This is always normalized
	// The normal should go from entity 'two' to entity 'one', so that it may be used straight away if e.g. two is a static floor and one is the dynamic entity.
	Vector3f collisionNormal;
	/// Flags for what results are relevant.
	int results;
	Vector3f preliminaryCollisionNormal;
	// False by defualt. Should be set to true in resolver.
	bool resolved;
	/// Depth the entities are into each other's geometry upon detection.
	float distanceIntoEachOther;

	// Entities in the collission.
	Entity* one;
	Entity* two;

	/// Fills the lists of involved entities.
	void ExtractData();

	/// Always at least one, so if it is empty, check the base entities involved.
	List< Entity* > dynamicEntities;
	List< Entity* > kinematicEntities;
	List< Entity* > staticEntities;

	/// If a collission between a static and dynamic entity, a flag will be raised and these pointers will be set.
	Entity* dynamicEntity;
	Entity* staticEntity;
	///
	Vector3f collissionPoint;
	/// The total force at the moment of collission (sum of velocities)
	float collissionVelocity;
	/// List of active triangles in the collission
	List<Triangle> activeTriangles;

	/// Collision-points.
	List<Vector3f> pointsOne, pointsTwo;
	List<Vector3f> separatingAxes;
	/// Current velocity in the active points at the time of the collission.
	Vector3f collissionPointVelocity[2];

	/// Data about the resolution of the collission, including changes to linear and angular momentum.
	CollisionResolution cr;

private:
	// Called by ExtractData for each entity.
	void ExtractEntityData(Entity* entity);
};

/** Updates the entity's collission state (colliding, in rest, on plane)
	using it's current velocities and the collission normal.
	Defined in UpdateCollisionState.cpp
*/
void UpdateCollisionState(Entity* entity, Vector3f & collisionNormal);


#endif
