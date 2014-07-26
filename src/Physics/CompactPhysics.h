// Emil Hedemalm
// 2013-09-04

#ifndef COMPACT_PHYSICS_H
#define COMPACT_PHYSICS_H

#include "../MathLib.h"
#include "Physics/PhysicsSettings.h"
#include <fstream>

struct PhysicsProperty;	// Forward declaration

/** Compact Entity struct that is a storage class for entities when saving/loading
	as well as when they are not active in an unloaded map to decrease memory usage but still enabling
	dynamic level loading.
*/
typedef struct CompactPhysics {
	CompactPhysics();
	~CompactPhysics();
	CompactPhysics(PhysicsProperty * physicsProperty);

	int type;
	int physicsShape;
	/// Name of the physics mesh, if the entity uses a separate such
	char * physicsMesh;
	int state;
	float physicalRadius;
	Vector3f velocity;
	Vector3f acceleration;
#ifdef USE_MASS
	float mass;
	float inverseMass;
	float volume;
	float density;
#endif
	float restitution;
	float friction;

	/// See PhysicsProperty.h
	int collissionCallback;
	float collissionCallbackRequirementValue;
	bool collissionsEnabled;
	bool noCollisionResolutions;

	/// Reads data from file stream
	bool ReadFrom(std::fstream& file);
	/// Write data to file stream
	bool WriteTo(std::fstream& file);
	/// Calculates size the entity will take when saving, including any eventual sub-properties (if we place them here?)
	int Size();

private:
	/// Nullifies all relevant pointers and numbers
	void Nullify();

} CPHYSICS, *PCPHYSICS;

#endif
