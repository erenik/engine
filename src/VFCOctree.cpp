
#include "VFCOctree.h"
#include "Entity/Entity.h"

/// Define the static values
/// The frustum to be checked against
Frustum VFCOctree::cullingFrustum;

/// Top-level constructor
VFCOctree::VFCOctree(float size, Frustum i_frustum){
	
	for (int i = 0; i < MAX_CHILD_NODES; ++i){
		child[i] = NULL;
	}

	// Transpose the Octree slightly to avoid accumulation of crap objects along the x/y/z = 0 axises
	left = -size + size/10;
	right = size + size/10;
	top = size + size/10;
	bottom = -size + size/10;
	nearBound = size + size/10;
	farBound = -size + size/10;
	subdivision = 0;
	center = Vector3f(
		(right + left) / 2.0f, 
		(top + bottom) / 2.0f, 
		(nearBound + farBound) / 2.0f
	);
	// Calculate radial radius straight away too.
	radius = sqrt((right * right + top * top + nearBound * nearBound));
	cullingFrustum = i_frustum;
}

/// Constructor that sets the Entity's boundaries and subdivision level
VFCOctree::VFCOctree(float i_leftBound, float i_rightBound, float i_topBound, float i_bottomBound, float i_nearBound, float i_farBound, int i_subdivision){

	for (int i = 0; i < MAX_CHILD_NODES; ++i){
		child[i] = NULL;
	}

	left = i_leftBound;
	right = i_rightBound;
	top = i_topBound;
	bottom = i_bottomBound;
	nearBound = i_nearBound;
	farBound = i_farBound;
	subdivision = i_subdivision;
	center = Vector3f(
		(right + left) / 2.0f, 
		(top + bottom) / 2.0f, 
		(nearBound + farBound) / 2.0f
	);
	// Calculate radial radius straight away too.
	float width = (left - right) / 2.0f;
	float height = (top - bottom) / 2.0f;
	float depth = (nearBound - farBound) / 2.0f;

	radius = sqrt((width * width + height * height + depth * depth));
}

/// Default destructor that deallocates all children
VFCOctree::~VFCOctree(){
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			delete child[i];
			child[i] = NULL;
		}
	}
}

/** Recursive subdivision function that subdivides the nodes down the tree specified amount of levels. 
	If MAX_SUBDIVISION is passed an exception will be thrown.
*/
void VFCOctree::subdivide(int levels){
	// Decrement levels
	levels--;
	// Check that the maximum subdivision level hasn't been reached.
	int childSubdivisionLvl = subdivision + 1;
	if (childSubdivisionLvl > MAX_SUBDIVISION)
		throw 4;
	// Check if the children aren't already allocated.
	if (!child[0]){
		// Allocate if needed
		child[HITHER_LOWER_LEFT] = new VFCOctree(left, center.x, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_LOWER_RIGHT] = new VFCOctree(center.x, right, center.y, bottom, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UPPER_LEFT] = new VFCOctree(left, center.x, top, center.y, nearBound, center.z, childSubdivisionLvl);
		child[HITHER_UPPER_RIGHT] = new VFCOctree(center.x, right, top, center.y, nearBound, center.z, childSubdivisionLvl);
		child[FARTHER_LOWER_LEFT] = new VFCOctree(left, center.x, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_LOWER_RIGHT] = new VFCOctree(center.x, right, center.y, bottom, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UPPER_LEFT] = new VFCOctree(left, center.x, top, center.y, center.z, farBound, childSubdivisionLvl);
		child[FARTHER_UPPER_RIGHT] = new VFCOctree(center.x, right, top, center.y, center.z, farBound, childSubdivisionLvl);
	}
	// Subdivide all children further if levels is still positive.
	if (levels > 0){
		for (int i = 0; i < MAX_CHILDREN; ++i)
			child[i]->subdivide(levels);
	}
}

void VFCOctree::clearAll(){
	entities.Clear();
	if (child[0])
		for (int i = 0; i < MAX_CHILDREN; ++i)
			child[i]->clearAll();
}


/** Adds a scenegraph Entity to this vfcOctree Entity unless max nodes has been reached. 
	If MAX_INITIAL_NODES_BEFORE_SUBDIVISION is reached, subdivision hasn't yet been done and MAX_SUBDIVISION hasn't been reached, subdivision occurs. 
*/
bool VFCOctree::AddEntity(Entity * targetEntity){
	// Check that it isn't already added!
	bool exists = entities.Exists(targetEntity);
	if (exists){
		std::cout<<"\nAdding Entity to PhysicsOctree while it already exists!";
		assert("Adding Entity to PhysicsOctree while it already exists!" && false);
		return false;
	}
	// If we have children, check if it fits in any of them.
	if (child[0]){
		int result = OUTSIDE;
		for (int i = 0; i < MAX_CHILDREN; ++i){
			result = child[i]->IsEntityInside(targetEntity);
			switch(result){
					// If the Entity is outside, just check next Entity~
				case OUTSIDE: 
					continue; 	break;
					// If intersecting, don't do anything. Check all children and handle intersections after this for-loop.
				case INTERSECT: break;
					// If it is inside, continue down the chain and then return from hierrr.
				case INSIDE: 
					return child[i]->AddEntity(targetEntity);
			}
		}
		// If we arrived here, it's either intersecting or something, so add it to our current children since it can't go further down the tree.
		entities.Add(targetEntity);
		
		// Set pointer for the entity too now
	//	targetEntity->physics->octreeNode = this;
		return true;
	} /// End of trying to enter it into any of our children

	// Okay, no spot in children, check if we should subdivide it (if the children aren't already allocated, that is!)
	if (entities.Size() > MAX_INITIAL_NODES_BEFORE_SUBDIVISION && child[0] == NULL){
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
//	targetEntity->physics->octreeNode = this;
	return true;
}

/** Polls the existence of target entity with in this node (or any of it's children). */
bool VFCOctree::Exists(Entity * entity){
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
bool VFCOctree::RepositionEntity(Entity * entity){
	if (!RemoveEntity(entity)){
		throw 3;
		return false;
	}
	// We assume AddEntity works for now.
	AddEntity(entity);
	return true;
}

// Removes a scenegraph Entity from this vfcOctree. Searches recursively until.
bool VFCOctree::RemoveEntity(Entity * targetEntity){
	assert(targetEntity->registeredForRendering);
	if (entities.Remove(targetEntity)){
		return true;
	}
	// Go through all children. If one of them finds the target Entity, return true without processing any more children.
	if (child[0])
		for (int i = 0; i < MAX_CHILDREN; ++i)
			if (child[i]->RemoveEntity(targetEntity)){
				return true;
			}
	if (this->subdivision == 0){
		assert(this->Exists(targetEntity));
		assert(false && "Unable to remove entity in VFCOctree::RemoveEntity!");
	}
	return false;
}

/// Returns a count of all registered entities within the vfcOctree
int VFCOctree::RegisteredEntities(){
	int total = 0;
	total += entities.Size();
	if (child [0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			total += child[i]->RegisteredEntities();
		}
	}
	return total;
}

#include <cassert>
/// Just render daonw!
void VFCOctree::Render(){
	assert(this);
	// Render all objects in this node,
	int i;
	for (i = 0; i < entities.Size(); ++i){
		if (entities[i])
			entities[i]->Render();
		else
			break;
	}
	// Then traverse further down unless we have no children...
	if (!child[0])
		return;
	// ..and set everything there if we got any.
	for (i = 0; i < MAX_CHILDREN; ++i){
		child[i]->Render();
	}
}

/// Render the objects that are in the frustum!
void VFCOctree::RenderWithCulling(GraphicsState &state){
	// Set all active objects to be inside, all comparisons should just be with Octree objects to keep down the amount of comparisons
	for (int i = 0; i < entities.Size(); ++i){
		// Do individual check of all nodes here so we don't render in vain.
		if (entities[i]){
		//	if (cullingFrustum.SphereInFrustum(entities[i]->position, entities[i]->radius))
				entities[i]->Render();
		//	else
		//		;
		}
		// Break if we find an null-spot in the array. We shouldn't have any empty spots/holes in the arrays.
		else
			break;
	}
	// Check all children.
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			// Skip children with no scenegraph nodes or further children.
			if (child[i]->entities.Size() == 0 && child[i]->child[0] == NULL)
				continue;
			int childCullState = child[i]->cullingFrustum.SphereInFrustum(child[i]->center, child[i]->radius);
			// If inside, set all children to be inside the frustum
			if (childCullState == INSIDE)
				child[i]->Render();
			// And if outside, set all children to be outside the frustum
			else if (childCullState == OUTSIDE)
				continue; //	child[i]->setInFrustum(false);
			// If intersecting, continue comparisons
			else 
				child[i]->RenderWithCulling(state);
		}
	}
}

/// Render function, as with the scenegraph: begins the recursive culling and rendering.
void VFCOctree::updateCulling(){
	// Set all active objects to be inside, all comparisons should just be with Octree objects to keep down the amount of comparisons
	for (int i = 0; i < entities.Size(); ++i){
		// Do individual check of all nodes here so we don't render in vain.
		if (entities[i]){
			if (cullingFrustum.SphereInFrustum(entities[i]->position, entities[i]->radius))
				;
	//			objects[i]->inFrustum = true;
			else
			;
	//			objects[i]->inFrustum = false;
		}
		// Break if we find an null-spot in the array. We shouldn't have any empty spots/holes in the arrays.
		else
			break;
	}
	// Check all children.
	if (child[0]){
		for (int i = 0; i < MAX_CHILDREN; ++i){
			// Skip children with no scenegraph nodes or further children.
			if (child[i]->entities.Size() == 0 && child[i]->child[0] == NULL)
				continue;
			int childCullState = child[i]->cullingFrustum.SphereInFrustum(child[i]->center, child[i]->radius);
			// If inside, set all children to be inside the frustum
			if (childCullState == INSIDE)
				child[i]->setInFrustum(true);
			// And if outside, set all children to be outside the frustum
			else if (childCullState == OUTSIDE)
				child[i]->setInFrustum(false);
			// If intersecting, continue comparisons
			else 
				child[i]->updateCulling();
		}
	}
}

/// Sets all children scenegraph nodes to the provided state
void VFCOctree::setInFrustum(bool inTrueOrOutFalse){
/*	int i;
	for (i = 0; i < maxNodes; ++i){
		if (objects[i])
			objects[i]->inFrustum = inTrueOrOutFalse;
		else
			break;
	}
	// Break if we have no children...
	if (!child[0])
		return;
	// ..and set everything there if we got any.
	for (i = 0; i < MAX_CHILDREN; ++i){
		child[i]->setInFrustum(inTrueOrOutFalse);
	}
	*/
}


/// Checks if the target Entity is inside this VFCOctree Entity, intersecting it or outside.
int VFCOctree::IsEntityInside(Entity * entity){
	// Make box test insteeeead
	float radius = entity->radius * entity->scale.MaxPart();
	// Check if it's inside.
	if (entity->position.x + radius < right &&
		entity->position.x - radius > left &&
		entity->position.y + radius < top &&
		entity->position.y - radius > bottom &&
		entity->position.z + radius < nearBound &&
		entity->position.z - radius > farBound
	)
		return INSIDE;
	// Or intersecting, just compare with inverted radius
	else if (entity->position.x - radius < right &&
		entity->position.x + radius > left &&
		entity->position.y - radius < top &&
		entity->position.y + radius > bottom &&
		entity->position.z - radius < nearBound &&
		entity->position.z + radius > farBound
	)
		return INTERSECT;
	// It's outside if the previous were false, logical :P
	return OUTSIDE;
}
