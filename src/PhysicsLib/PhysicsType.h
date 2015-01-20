/// Emil Hedemalm
/// 2014-08-06
/// Not sure if I will regret dividing upp all of this into separate files..

#ifndef PHYSICS_TYPE_H
#define PHYSICS_TYPE_H

namespace PhysicsType {
enum PhysicsTypes{
	STATIC,			// Immovable
	DYNAMIC,		// Interacts with all objects.
	KINEMATIC,		// Move, but are not affected by other objects.
	NUM_TYPES
};};

#endif
