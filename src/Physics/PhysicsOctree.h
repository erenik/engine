// An octree made specifically for grouping shapes of different kinds in a structure for eased testing.
// Emil Hedemalm
// 2013-03-17

#ifndef PHYSICS_OCTREE_H
#define PHYSICS_OCTREE_H

#include "Collision/Collisions.h"
#include "Collision/Collision.h"
//#include "../PhysicsLib.h"
#include "../Entity/Entity.h"
#include <Util.h>
#include "PhysicsLib/Location.h"
class Entity;

/** Physics Collision Detection Octree
	This is an octree node class made to work with the entity from the Entity handler.
	It assumes the entity have spherical bounding volumes.
*/
class PhysicsOctree {
	friend class PhysicsManager;
public:
	/// Constructor that does.. nothing.
	PhysicsOctree();
private:
	/// Internal constructor for expanding the vfcOctree.
	PhysicsOctree(float leftBound, float rightBound, float topBound, float bottomBound, float nearBound, float farBound, int subdivision);
public:
	/// Default destructor that deallocates all children
	~PhysicsOctree();

	/// Sets boundaries of the top-level node.
	void SetBoundaries(float leftBound, float rightBound, float topBound, float bottomBound, float nearBound, float farBound);

	/** Recursive subdivision function that subdivides the nodes down the tree specified amount of levels.
		If MAX_SUBDIVISION is passed an exception will be thrown.
	*/
	void subdivide(int levels = 1);

	/// Removes all Entity pointers from the tree without deallocating any vfcOctree nodes.
	void clearAll();

	/** Adds an entity node to this vfcOctree node unless max nodes has been reached.
		If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs.
	*/
	bool AddEntity(EntitySharedPtr Entity);
	/** Polls the existence of target entity with in this node (or any of it's children). */
	bool Exists(EntitySharedPtr entity);
	/// Removes the Entity and re-inserts it to it's new location
	bool RepositionEntity(EntitySharedPtr Entity);
	/// Removes an entity node from this vfcOctree. Searches recursively until.
	bool RemoveEntity(EntitySharedPtr Entity);

	/// Returns a count of all registered entities within the vfcOctree
	int RegisteredEntities();

	/// Number of nodes before a subdivision should occur.
	static const int MAX_INITIAL_NODES_BEFORE_SUBDIVISION = 7;
	/** Maximum amount of nodes in total for this node.
		This is larger than the subdivision query level in order to store entity that are too large for the subdivided levels correctly.
	*/
	static const int MAX_INITIAL_NODES = 4;
	/// Maximum subdivision level
	static const int MAX_SUBDIVISION = 32;

private:
	/** Searches for collissions with specified entity.
		If entry subdivision level is not specified the initial call will set it automatically (used for recursion limits)
		Returns amount of collissions tested.
	*/
	int FindCollisions(EntitySharedPtr entity, List<Collision> & collissions, int entrySubdivisionLevel = -1);
	/// Checks if the target entity is inside this PhysicsTree node, intersecting it or outside.
	int IsEntityInside(EntitySharedPtr Entity);

	/// A center vector to avoid re-calculating it all the time.
	Vector3f center;
	/// Boundaries of the box.
	float left, right, top, bottom, nearBound, farBound;
	/// Radial bounding for ze box. I like spherical tests. :3
	float radius;

	/// Subdivision level of this node. Used to prevent extreme depths if possible.
	int subdivision;

	/// Entities in this vfcOctree node. This is a dynamically allocated array, length depending on the MAX_INITIAL_NODES variable.
	List< std::shared_ptr<Entity> > entities;

	/// Locations
	enum {
		OUTSIDE = Loc::OUTSIDE,
		INSIDE = Loc::INSIDE,
		INTERSECT = Loc::INTERSECT,
	};

	/// Parent node pointer. NULL only if root-node!
	PhysicsOctree * parent;
	/// Chidren nodes
	PhysicsOctree * child[MAX_CHILD_NODES];
};

#endif
