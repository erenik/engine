// Emil Hedemalm
// 2013-07-20
// Old structure, based on an octree which was used for view frustum culling

#ifndef VFC_OCTREE_H
#define VFC_OCTREE_H

// #define VFC_OCTREE 

#ifdef VFC_OCTREE

#include "PhysicsLib/Location.h"
#include "List/List.h"
#include "PhysicsLib/Shapes/Frustum.h"

class GraphicsState;
class Entity;

/** View Frustum Culling Octree
	This is an vfcOctree node class made to work with the objects from the Entity handler. 
	It assumes the objects have spherical bounding volumes.
*/
class VFCOctree {
public:
	/// Top-level constructor
	VFCOctree(float size, const Frustum & frustum);
	/// Constructor that sets the node's boundaries.
	VFCOctree(float leftBound, float rightBound, float topBound, float bottomBound, float nearBound, float farBound, int subdivision);
	/// Default destructor that deallocates all children
	~VFCOctree();

	/** Recursive subdivision function that subdivides the nodes down the tree specified amount of levels. 
		If MAX_SUBDIVISION is passed an exception will be thrown.
	*/
	void subdivide(int levels = 1);

	/** Removes all Entity pointers from the tree without deallocating any vfcOctree nodes.
		De-flags all entities from rendering too.
	*/
	void clearAll();

	/** Adds an entity node to this vfcOctree node unless max nodes has been reached. 
		If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs. 
	*/
	bool AddEntity(Entity* Entity);
	/** Polls the existence of target entity with in this node (or any of it's children). */
	bool Exists(Entity* entity);	
	/// Removes the Entity and re-inserts it to it's new location
	bool RepositionEntity(Entity* Entity);
	/// Removes an entity node from this vfcOctree. Searches recursively until.
	bool RemoveEntity(Entity* Entity);
	

	/// Returns a count of all registered entities within the vfcOctree
	int RegisteredEntities();

	/// Compares if this AABB ViewFrustumCulling node is inside the frustum, intersecting or outside.
	int inFrustum();

	/// Updates culling status for all objects in the tree.
	void updateCulling();

	/// Render the objects that are in the frustum, using checks against the frustum against the current VFCOctree ndoe.
	void RenderWithCulling(GraphicsState & graphicsState);
	/// Renders the children and all objects without any checks
	void Render(GraphicsState & graphicsState);

	/// Sets all children scenegraph nodes to the provided state
	void setInFrustum(bool inTrueOrOutFalse);

	/// Identifier if it's inside or out
	enum locationEnum{
		OUTSIDE = Loc::OUTSIDE, INTERSECT = Loc::INTERSECT, INSIDE = Loc::INSIDE
	};

	/// Sets the frustum to be used when culling entities.
	static void SetCullingFrustum(const Frustum & frustum) {cullingFrustum = frustum; };

	/// Number of nodes before a subdivision should occur.
	static const int MAX_INITIAL_NODES_BEFORE_SUBDIVISION = 15;
	/** Maximum amount of nodes in total for this node. 
		This is larger than the subdivision query level in order to store objects that are too large for the subdivided levels correctly.
	*/
	static const int MAX_INITIAL_NODES = 4;
	/// Maximum subdivision level
	static const int MAX_SUBDIVISION = 32;

private:
	/// The frustum to be checked against
	static Frustum cullingFrustum;
	
	/// Checks if the target node is inside this VFCOctree node, intersecting it or outside.
	int IsEntityInside(Entity* Entity);

	/// A center vector to avoid re-calculating it all the time.
	Vector3f center;
	/// Boundaries of the box.
	float left, right, top, bottom, nearBound, farBound;
	/// Radial bounding for ze box. I like spherical tests. :3
	float radius;

	/// Subdivision level of this node. Used to prevent extreme depths if possible.
	int subdivision;

	/// Entities in this vfcOctree node. This is a dynamically allocated array, length depending on the MAX_INITIAL_NODES variable.
	List< Entity* > entities;

	/// Enum over the child nodes.
	enum childNodeNames{
		HITHER_UPPER_LEFT = NULL, HITHER_UPPER_RIGHT, HITHER_LOWER_LEFT, HITHER_LOWER_RIGHT,
		FARTHER_UPPER_LEFT, FARTHER_UPPER_RIGHT, FARTHER_LOWER_LEFT, FARTHER_LOWER_RIGHT,
		// Just two different names for the same thing ^^
		MAX_CHILD_NODES, MAX_CHILDREN = MAX_CHILD_NODES
	};

	/// Chidren nodes
	VFCOctree * child[MAX_CHILD_NODES];
};

#endif // VFC_OCTREE

#endif