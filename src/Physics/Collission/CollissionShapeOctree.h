// An octree made specifically for grouping shapes of different kinds in a structure for eased testing.
// Emil Hedemalm
// 2013-03-17

#ifndef COLLISSION_SHAPE_OCTREE_H
#define COLLISSION_SHAPE_OCTREE_H

#include "Physics/Collission/Collissions.h"
#include "Physics/Collission/Collission.h"
#include "PhysicsLib.h"
#include "Entity/Entity.h"
#include <Util.h>
class Entity;

/** Physics Collission Detection Octree
	This is an octree node class made to work with the entity from the Entity handler.
	It assumes the entity have spherical bounding volumes.
*/
class CollissionShapeOctree {
	friend class PhysicsManager;
	friend class GraphicsManager; // For debug-rendering
public:
	/// Constructor that does.. nothing.
	CollissionShapeOctree();
private:
	/// Internal constructor for expanding the vfcOctree.
	CollissionShapeOctree(float leftBound, float rightBound, float topBound, float bottomBound, float nearBound, float farBound, int subdivision);
public:
	/// Default destructor that deallocates all children
	~CollissionShapeOctree();

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
	bool AddTriangle(Triangle * tri);
	/** Polls the existence of target entity with in this node (or any of it's children). */
	bool Exists(Triangle * tri);
	/// Removes the Entity and re-inserts it to it's new location
	bool RepositionTriangle(Triangle * tri);
	/// Removes an entity node from this vfcOctree. Searches recursively until.
	bool RemoveTriangle(Triangle * tri);

	/// Returns a count of all registered entities within the vfcOctree
	int RegisteredShapes();

	/// Number of nodes before a subdivision should occur.
	static const int MAX_INITIAL_NODES_BEFORE_SUBDIVISION = 7;
	/** Maximum amount of nodes in total for this node.
		This is larger than the subdivision query level in order to store entity that are too large for the subdivided levels correctly.
	*/
	static const int MAX_INITIAL_NODES = 4;
	/// Maximum subdivision level
	static const int MAX_SUBDIVISION = 32;

	/// Prints contents in each cubicle
	void PrintContents();

private:
	/// To be used via the GraphicsManager's RenderPhysics function ONLY.
	/// Matrices are assumed to be set up already upon entry
	bool Render(void * graphicsState);

	/** Searches for collissions with specified entity.
		If entry subdivision level is not specified the initial call will set it automatically (used for recursion limits)
		Returns amount of collissions tested.
		- localTransform is applied to all relevant triangle upon testing if provided.
	*/
	int FindCollissions(Entity * entity, List<Collission> & collissions, Matrix4f & localTransform, int entrySubdivisionLevel = -1);
	/** Searches for collissions with specified entity.
		If entry subdivision level is not specified the initial call will set it automatically (used for recursion limits)
		Returns amount of collissions tested.
	*/
	int FindCollissions(Entity * entity, List<Collission> & collissions, int entrySubdivisionLevel = -1);
	/// Checks if the target entity is inside this Octree node, intersecting it or outside.
	int IsEntityInside(Entity * Entity, Matrix4f & localTransform);
	/// Checks if the target tri is inside this Octree node, intersecting it or outside.
	int IsTriangleInside(Triangle * tri);

	/// A center vector to avoid re-calculating it all the time.
	Vector3f center;
	/// Boundaries of the box.
	float left, right, top, bottom, nearBound, farBound;
	/// Radial bounding for ze box. I like spherical tests. :3
	float radius;

	/// Subdivision level of this node. Used to prevent extreme depths if possible.
	int subdivision;

	/// Triangles in this octree node. This is a dynamically allocated array, length depending on the MAX_INITIAL_NODES variable.
	List<Triangle*> triangles;

	/// Locations
	enum {
		OUTSIDE = Loc::OUTSIDE,
		INSIDE = Loc::INSIDE,
		INTERSECT = Loc::INTERSECT,
	};

	/// Enum over the child nodes.
	enum childNodeNames{
		HITHER_UPPER_LEFT = NULL, HITHER_UPPER_RIGHT, HITHER_LOWER_LEFT, HITHER_LOWER_RIGHT,
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

	/// Parent node pointer. NULL only if root-node!
	CollissionShapeOctree * parent;
	/// Chidren nodes
	CollissionShapeOctree * child[MAX_CHILD_NODES];
};

#endif
