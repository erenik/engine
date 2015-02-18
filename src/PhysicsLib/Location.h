/// Emil Hedemalm
/// 2014-08-06
/// Used for intersection and collision tests.

#ifndef PHYSICS_LOC_H
#define PHYSICS_LOC_H

namespace Loc {
enum Loc{
	OUTSIDE,
	INTERSECT,
	INSIDE
};};


/// Enum over the child nodes.
enum childNodeNames{
	HITHER_UPPER_LEFT = 0, HITHER_UPPER_RIGHT, HITHER_LOWER_LEFT, HITHER_LOWER_RIGHT,
	FARTHER_UPPER_LEFT, FARTHER_UPPER_RIGHT, FARTHER_LOWER_LEFT, FARTHER_LOWER_RIGHT,

	// 8 more added
	UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT, FARTHER_DOWN, FARTHER_UP, HITHER_DOWN, HITHER_UP,
	// + Center
	CENTER,
	// + Center-branches
	HITHER, FARTHER, LEFTER, RIGHTER, UPPER, LOWER,
	// + Huggers(apartments, whatever)
	FATHER_LEFT, FARTHER_RIGHT,
	HITHER_LEFT, HITHER_RIGHT,

	// Just two different names for the same thing ^^
	MAX_CHILD_NODES, MAX_CHILDREN = MAX_CHILD_NODES
};


#endif