// Emil Hedemalm
// 2013-03-17
#include "PhysicsOctree.h"
#include "PhysicsProperty.h"
#include "PhysicsManager.h"

/// Define the static values

/// Basic constructor
PhysicsOctree::PhysicsOctree(){
	for (int i = 0; i < MAX_CHILD_NODES; ++i){
		child[i] = NULL;
	}

	// Default Entity pointer amounts.
	subdivision = 0;
	parent = NULL;
}

/// Internal constructor that sets the Entity's boundaries and subdivision level
PhysicsOctree::PhysicsOctree(float i_leftBound, float i_rightBound, float i_topBound, float i_bottomBound, float i_nearBound, float i_farBound, int i_subdivision){
	for (int i = 0; i < MAX_CHILD_NODES; ++i){
		child[i] = NULL;
	}
	// Set boundaries, calculate radius, etc.
	SetBoundaries(i_leftBound, i_rightBound, i_topBound, i_bottomBound, i_nearBound, i_farBound);
	subdivision = i_subdivision;
}

/// Default destructor that deallocates all children
PhysicsOctree::~PhysicsOctree(){
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			delete child[i];
			child[i] = NULL;
		}
	}
}

/// Sets boundaries of the top-level node.
void PhysicsOctree::SetBoundaries(float leftBound, float rightBound, float topBound, float bottomBound, float i_nearBound, float i_farBound){
	left = leftBound;
	right = rightBound;
	top = topBound;
	bottom = bottomBound;
	nearBound = i_nearBound;
	farBound = i_farBound;
	center = Vector3f(
		(right + left) / 2.0f,
		(top + bottom) / 2.0f,
		(nearBound + farBound) / 2.0f
	);
	/// Calculate radial radius straight away too.
	float width = (left - right) / 2.0f;
	float height = (top - bottom) / 2.0f;
	float depth = (nearBound - farBound) / 2.0f;

	radius = sqrt((width * width + height * height + depth * depth));
}


/** Recursive subdivision function that subdivides the nodes down the tree specified amount of levels.
	If MAX_SUBDIVISION is passed an exception will be thrown.
*/
void PhysicsOctree::subdivide(int levels){
	// Decrement levels
	levels--;
	// Check that the maximum subdivision level hasn't been reached.
	int childSubdivisionLvl = subdivision + 1;
	if (childSubdivisionLvl > MAX_SUBDIVISION)
		throw 4;
	// Check if the children aren't already allocated.
	if (!child[0]){
#define OctreeType	PhysicsOctree
		// Allocate if needed
		child[HITHER_LOWER_LEFT] = new PhysicsOctree(left, center.x, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_LOWER_RIGHT] = new PhysicsOctree(center.x, right, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UPPER_LEFT] = new PhysicsOctree(left, center.x, top, center.y, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UPPER_RIGHT] = new PhysicsOctree(center.x, right, top, center.y, nearBound, center.z, childSubdivisionLvl);
		child[FARTHER_LOWER_LEFT] = new PhysicsOctree(left, center.x, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_LOWER_RIGHT] = new PhysicsOctree(center.x, right, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UPPER_LEFT] = new PhysicsOctree(left, center.x, top, center.y, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UPPER_RIGHT] = new PhysicsOctree(center.x, right, top, center.y, center.z, farBound, childSubdivisionLvl);

		// UPPER_LEFT, UPPER_RIGHT, LOWER_LEFT, LOWER_RIGHT, FARTHER_DOWN, FARTHER_UP, HITHER_DOWN, HITHER_UP,
		child[UPPER_LEFT] = new OctreeType(left, center.x, top, center.y, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[UPPER_RIGHT] = new OctreeType(center.x, right, top, center.y, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[LOWER_LEFT] = new OctreeType(left, center.x, center.y, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[LOWER_RIGHT] = new OctreeType(center.x, right, center.y, bottom, (center.z + nearBound)*0.5f, (center.z + farBound)*0.5f, childSubdivisionLvl);
		child[FARTHER_DOWN] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UP] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, center.z, farBound, childSubdivisionLvl);
		child[HITHER_DOWN] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UP] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, nearBound, center.z, childSubdivisionLvl);

		// Center
		child[CENTER] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5, (center.z+farBound)*0.5, childSubdivisionLvl);

		// + Center-branches
		//HITHER, FARTHER, LEFTER, RIGHTER, UPPER, LOWER,
		child[HITHER] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl);
		child[FARTHER] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl);
		child[LEFTER] = new OctreeType(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);
		child[RIGHTER] = new OctreeType(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);
		child[UPPER] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, top, center.y, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);
		child[LOWER] = new OctreeType((center.x+left)*0.5f, (center.x+right)*0.5f, center.y, bottom, (center.z+nearBound)*0.5f, (center.z+farBound)*0.5f, childSubdivisionLvl);

		// + Huggers(apartments, whatever)
		// FATHER_LEFT, FARTHER_RIGHT, HITHER_LEFT, HITHER_RIGHT,
		child[FATHER_LEFT] = new OctreeType(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_RIGHT] = new OctreeType(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, center.z, farBound, childSubdivisionLvl);
		child[HITHER_LEFT] = new OctreeType(left, center.x, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_RIGHT] = new OctreeType(center.x, right, (center.y+top)*0.5f, (center.y+bottom)*0.5f, nearBound, center.z, childSubdivisionLvl);

	}
	for (int i = 0; i < MAX_CHILDREN; ++i)
		child[i]->parent = this;
	// Subdivide all children further if levels is still positive.
	if (levels > 0){
		for (int i = 0; i < MAX_CHILDREN; ++i)
			child[i]->subdivide(levels);
	}
}

void PhysicsOctree::clearAll(){
	entities.Clear();
	if (child[0])
		for (int i = 0; i < MAX_CHILDREN; ++i)
			child[i]->clearAll();
}


/** Adds a scenegraph Entity to this vfcOctree Entity unless max nodes has been reached.
	If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs.
*/
bool PhysicsOctree::AddEntity(Entity * targetEntity)
{
	// Check that it isn't already added!
	bool exists = entities.Exists(targetEntity);
	if (exists){
		std::cout<<"\nAdding Entity to PhysicsOctree while it already exists!";
		assert("Adding Entity to PhysicsOctree while it already exists!" && false);
		return false;
	}
	// If we have children, check if it fits in any of them.
	if (child[0])
	{
		int result = OUTSIDE;
		for (int i = 0; i < MAX_CHILDREN; ++i)
		{
			result = child[i]->IsEntityInside(targetEntity);
			switch(result)
			{		
				case OUTSIDE:
					// If the Entity is outside, just check next octree-section~
					continue; 
				case INTERSECT: 
					// If intersecting, don't do anything. Check all children and handle intersections after this for-loop.
					break;
				case INSIDE:
					// If it is inside, continue down the chain and then return from hierrr.
					return child[i]->AddEntity(targetEntity);
			}
		}
		// If we arrived here, it's either intersecting or something, so add it to our current children since it can't go further down the tree.
		entities.Add(targetEntity);

		// Set pointer for the entity too now
		targetEntity->physics->octreeNode = this;
		return true;
	} /// End of trying to enter it into any of our children

	// Okay, no spot in children, check if we should subdivide it (if the children aren't already allocated, that is!)
	if (entities.Size() > MAX_INITIAL_NODES_BEFORE_SUBDIVISION && child[0] == NULL && this->subdivision < MAX_SUBDIVISION){
		// Subdivide and then try push all our children down the tree further, so they don't get stuck here without reason.
		subdivide();
		List<Entity*> tempList(entities);
		entities.Clear();
		for (int j = 0; j < tempList.Size(); ++j){
			AddEntity(tempList[j]);
		}
		// Return if we subdivided, just make sure we try to add our new child to the subdivision too.
		return AddEntity(targetEntity);
	}
	// Alright, children are out, just add it now then.
	entities.Add(targetEntity);
	targetEntity->physics->octreeNode = this;
	return true;
}

/** Polls the existence of target entity with in this node (or any of it's children). */
bool PhysicsOctree::Exists(Entity * entity){
	if (entities.Exists(entity))
		return true;
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i)
			if (child[i]->Exists(entity))
				return true;
	}
	return false;
}

/// Removes the Entity and re-inserts it to it's new location
bool PhysicsOctree::RepositionEntity(Entity * entity)
{
	// Check for vfcOctree node in Entity.
	PhysicsOctree * node = entity->physics->octreeNode;
	if (node == NULL){
		return false;
	}
	assert(node);
	/// Why do we check this? This means that the entity will not move to a lesser vfcOctree node even if that is possibubul öAö
	int status = node->IsEntityInside(entity);
	switch(status)
	{
		case INSIDE:
			/// Still inside the node it was in on the start of the frame? Then leave it be.
			return true;
		case OUTSIDE: case INTERSECT:
			// Send it upwards until it fits
			while(true)
			{
				if (node->parent == NULL){
					std::cout<<"\nWARNING: Object outside root node! Unregistering from physics!";
					entity->physics->octreeNode = NULL;
			//		assert(false && "Entity at root-node yet outside/intersecting it!");
					/// Unregister th entity by default if it exceeds the boundaries!
					PhysicsManager::Instance()->UnregisterEntity(entity);
					return false;
				}
				node = node->parent;
				if (node->IsEntityInside(entity) == INSIDE){
					RemoveEntity(entity);
					assert(node->AddEntity(entity));
					return true;
				}
			}
			break;
	}
	return true;
}

// Removes a scenegraph Entity from this vfcOctree. Searches recursively until.
bool PhysicsOctree::RemoveEntity(Entity * targetEntity)
{
	if (entities.Remove(targetEntity))
	{
		targetEntity->physics->octreeNode = NULL;
		return true;
	}
	// Go through all children. If one of them finds the target Entity, return true without processing any more children.
	if (child[0])
		for (int i = 0; i < MAX_CHILDREN; ++i)
			if (child[i]->RemoveEntity(targetEntity))
				return true;
	if (this->subdivision == 0){
		if (this->Exists(targetEntity)){
			assert(!this->Exists(targetEntity));
			assert(false && "Unable to remove entity in PhysicsOctree::RemoveEntity!");
		};
	}
	return false;
}

/// Returns a count of all registered entities within the vfcOctree
int PhysicsOctree::RegisteredEntities(){
	int total = 0;
	total += entities.Size();
	if (child [0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			total += child[i]->RegisteredEntities();
		}
	}
	return total;
}

static int collissionsTested = 0;

/// Compare each dynamic entity with every other entity in it's current vfcOctree node,
/// all nodes below it as well the direct parents above it.
int PhysicsOctree::FindCollisions(Entity * targetEntity, List<Collision> & collissionList, int entrySubdivisionLevel){
	if (entrySubdivisionLevel == -1)
		entrySubdivisionLevel = this->subdivision;

	int collissionsTested = 0;
	/// First it's current node:
	for (int i = 0; i < entities.Size(); ++i){
		Entity * e = entities[i];
		assert(e && "Nullentity in PhysicsOctree::FindCollisions");
		Physics.TestCollision(targetEntity, e, collissionList);
		++collissionsTested;
	}

	/// If entry node, begin all recursion.
	if (this->subdivision == entrySubdivisionLevel){
		/// Then all children
		if (child[0] && this->subdivision >= entrySubdivisionLevel)
		for (int i = 0; i < MAX_CHILDREN; ++i){
			/// Do the actual culling with this continue-statement, yo!
			if (child[i]->IsEntityInside(targetEntity) == OUTSIDE)
				continue;
			collissionsTested += child[i]->FindCollisions(targetEntity, collissionList, entrySubdivisionLevel);
		}
		/// And at last all parents
		if (parent && this->subdivision <= entrySubdivisionLevel)
			collissionsTested += parent->FindCollisions(targetEntity, collissionList, entrySubdivisionLevel);
		return collissionsTested;
	}
	/// Process children if subdivision level is higher (further down the tree.)
	else if (this->subdivision > entrySubdivisionLevel){
		/// Then all children
		if (child[0]){
			for (int i = 0; i < MAX_CHILDREN; ++i){
				/// Do the actual culling with this continue-statement, yo!
				if (child[i]->IsEntityInside(targetEntity) == OUTSIDE)
					continue;
				collissionsTested += child[i]->FindCollisions(targetEntity, collissionList, entrySubdivisionLevel);
			}
		}
		return collissionsTested;
	}
	/// And process parents only if lesser subdivision
	else if (this->subdivision < entrySubdivisionLevel){
		/// Then go to next parent.
		if (parent && this->subdivision <= entrySubdivisionLevel)
			collissionsTested += parent->FindCollisions(targetEntity, collissionList, entrySubdivisionLevel);
		return collissionsTested;
	}
	assert(false && "PhysicsOctree::FindCollisions at a point where it shouldn't be!");
	return 0;
}

/// Checks if the target Entity is inside this PhysicsOctree Entity, intersecting it or outside.
int PhysicsOctree::IsEntityInside(Entity * entity){
	// Make box test insteeeead

	// Check if it's inside.
	if (entity->position.x + entity->physics->physicalRadius < right &&
		entity->position.x - entity->physics->physicalRadius > left &&
		entity->position.y + entity->physics->physicalRadius < top &&
		entity->position.y - entity->physics->physicalRadius > bottom &&
		entity->position.z + entity->physics->physicalRadius < nearBound &&
		entity->position.z - entity->physics->physicalRadius > farBound
	)
		return INSIDE;
	// Or intersecting, just compare with inverted radius
	else if (entity->position.x - entity->physics->physicalRadius < right &&
		entity->position.x + entity->physics->physicalRadius > left &&
		entity->position.y - entity->physics->physicalRadius < top &&
		entity->position.y + entity->physics->physicalRadius > bottom &&
		entity->position.z - entity->physics->physicalRadius < nearBound &&
		entity->position.z + entity->physics->physicalRadius > farBound
	)
		return INTERSECT;
	// It's outside if the previous were false, logical :P
	return OUTSIDE;
}
