/// Emil Hedemalm
/// 2015-02-21
/// AABBSweeper 
/// For division nof AABB-sweep grid.

#include "AABBSweepAxis.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include "Globals.h"

List<AABBSweepNode*> AABBSweepNode::allTheNodes, AABBSweepNode::freeNodes;

AABBSweepNode::AABBSweepNode()
{
    aabb = NULL;
    entity = NULL;
    type = NULL_TYPE;
	sortedOnce = false;
	value = NULL;
	index = -1;
	allTheNodes.AddItem(this);
};

AABBSweepNode::~AABBSweepNode()
{
	allTheNodes.RemoveItemUnsorted(this);
}

AABBSweepNode * AABBSweepNode::New()
{
	if (freeNodes.Size())
	{
		AABBSweepNode * first = freeNodes[0];
		freeNodes.RemoveIndex(0);
		return first;
	}
	return new AABBSweepNode();
}
void AABBSweepNode::FreeForUse(AABBSweepNode * node)
{
	freeNodes.AddItem(node);
	assert(freeNodes.Size() < 10000);
}
void AABBSweepNode::FreeAllForUse() // Just moves to the usable-array.
{
	freeNodes = allTheNodes;
}

void AABBSweepNode::FreeAll()
{
	while(allTheNodes.Size())
		delete allTheNodes[0];
	allTheNodes.Clear();
}


AABBSweepAxis::AABBSweepAxis(int axis, ConstVec3fr min, ConstVec3fr max)
: axis(axis), min(min), max(max)
{
	
}
AABBSweepAxis::~AABBSweepAxis()
{
	nodes.ClearAndDelete();
}

void AABBSweepAxis::AddEntity(Entity* entity)
{
	// check if it fits here?
	AABB * aabb = entity->aabb;
	AABBSweepNode * newNodes[2];
	entity->physics->axes.AddItem(this);
	
	// Create the nodes.
	for (int i = 0; i < 2; ++i)
	{
		AABBSweepNode * newNode = newNodes[i] = AABBSweepNode::New();
        newNode->entity = entity;
        newNode->aabb = aabb;
        newNode->type = (i%2 == 0 ? AABBSweepNode::START : AABBSweepNode::STOP);
		newNode->value = &(i%2 == 0 ? aabb->min : aabb->max)[axis];
		newNode->name = entity->name + (newNode->type == AABBSweepNode::START? "Start" : "Stop");
//        std::cout<<"\nNode type created: "<<nodes[i]->type<<" "<< (nodes[i]->type == AABBSweepNode::START ? "Start" : "Stop") ;
		newNode->index = nodes.Size() - 1; // Since adding adds at the end, set its initial index accordingly.
		newNode->axis = this;
		bool inserted = false;
		/// Place the 2 entering nodes where fitting!
		for (int j = 0; j < nodes.Size(); ++j)
		{
			AABBSweepNode * other = nodes[j];
			/// Moving entity's are re-added per-frame, maybe. But if so, and their nodes are already here, skip them!
			if (other == newNode)
			{
				inserted = true;
				break;
			}
			/// Search until a node is larger than us, then place us once step before it.
			if (*other->value > *newNode->value)
			{
				nodes.Insert(newNode, j);
				inserted = true;
				break;
			}
		}
		if (!inserted)
			nodes.AddItem(newNode);
		
		entity->physics->aabbSweepNodes.AddItem(newNode);
    }
	if (entity->physics->type == PhysicsType::DYNAMIC)
		dynamicEntities.AddItem(entity);

}

void AABBSweepAxis::RemoveEntity(Entity* entity)
{
	entity->physics->axes.RemoveItemUnsorted(this);
	int deleted = 0;
	for (int i = 0; i < entity->physics->aabbSweepNodes.Size(); ++i)
	{
		AABBSweepNode * node = entity->physics->aabbSweepNodes[i];
		if (node->axis == this)
		{
			// Remove item, retain order.
			nodes.RemoveItem(node);
			entity->physics->aabbSweepNodes.RemoveItemUnsorted(node);
			--i;
			++deleted;
			AABBSweepNode::FreeForUse(node);
		}
	}
	assert(deleted == 2);
	/// Force-resort of the whole list, since the indices will now have been changed?
	/// .. nah, will be needed if try another approach tho

	if (entity->physics->type == PhysicsType::DYNAMIC)
		dynamicEntities.RemoveItemUnsorted(entity);
}

void AABBSweepAxis::Clear() // Clears all references.
{
	AABBSweepNode::FreeAllForUse();
	nodes.Clear();
	dynamicEntities.Clear();
}

bool AABBSweepAxis::ShouldBeHere(Entity* entity)
{
	AABB * aabb = entity->aabb;
	if (aabb->min.x > max.x)
		return false;
	if (aabb->min.y > max.y)
		return false;
	if (aabb->min.z > max.z)
		return false;
	if (aabb->max.x < min.x)
		return false;
	if (aabb->max.y < min.y)
		return false;
	if (aabb->max.z < min.z)
		return false;
	return true;
}

void AABBSweepAxis::Sort()
{
    /// Insertion sort the shit out of the list, yo!
    AABBSweepNode ** listArray = nodes.GetArray();
    int items = nodes.Size();

    AABBSweepNode * currentNode, * comparisonNode;
    float currentValue, comparisonValue;
    int emptySlot;
	/// Since we are sorting only one way, optimizations require looking out for movements in the other direction.
	bool sortedLastIteration = false;
	bool skip = false;
	int i, j;
    /// Manual sort, commence!
    for (i = 1; i < items; ++i)
	{
        /// Grab current node, starting from index 0 and moving towards the end.
        currentNode = listArray[i];
		skip = false;
		/// If static, Check if initially sorted. No need to look at this more then.
		if (currentNode->entity->physics->type == PhysicsType::STATIC && currentNode->sortedOnce)
			skip = true;
		if (sortedLastIteration)
			skip = false;
		if (skip)
			continue;
		// Set default.
		sortedLastIteration = false;
		// But in the case of dynamic entities, make sure those after check for sorting with it.
		if (currentNode->entity->physics->type != PhysicsType::STATIC)
			sortedLastIteration = true;

		/// Mark this index as empty for the time being.
        emptySlot = i;
		/// Extract current value depending on the node type (start, stop) and axis.
		currentValue = *currentNode->value;

        /// Assume all previous nodes have already been sorted, and compare only with them (comparing downwards)!
        for (j = i-1; j >= 0; --j)
		{
            comparisonNode = listArray[j];
            /// Extract comparison value depending on the node type (start, stop) and axis.
            comparisonValue = *comparisonNode->value;
            /// If the current value is less, move up the comparer and move down the empty slot index.
            if (currentValue < comparisonValue)
			{
                listArray[emptySlot] = comparisonNode;
				comparisonNode->index = emptySlot; // update index.
                --emptySlot;
				sortedLastIteration = true;
            }
            /// A lesser value was encountered! Stop shuffling around and place the current node in the now empty slot.
            else {
                /// And break ze inner loopty loop
                break;
            }
        }
        listArray[emptySlot] = currentNode;
		currentNode->index = emptySlot; // update index.
		// Mark the sortedOnce flag, so we only sort static entities once upon registration.
		currentNode->sortedOnce = true;
    }
	if (debug == 2)
	{
		std::cout<<"\nSorted nodes in AABBSweeper";
		for (int i = 0; i < nodes.Size(); ++i)
		{
			AABBSweepNode * node = nodes[i];
			std::cout<<"\n "<<i<<": "<<node->entity->name<<" "<<
				(	(node->type == AABBSweepNode::START)? 
					("START X: "+String(node->aabb->min.x)) : 
					("STOP X: "+String(node->aabb->max.x))
				);
			if (debug != 2)
				break;
		}
	}
}

bool AABBSweepAxis::GetPairs(List<EntityPair> & pairList)
{
	FindPairsLinearPlusSearch(pairList);
	return true;
}

void AABBSweepAxis::SortDynamicFocus()
{
	
}

/// X+
bool AABBSweepAxis::FindPairsLinearPlusSearch(List<EntityPair> & entityPairs)
{
	if (nodes.Size() == 0)
		return true;
	/// Old loop.
	int dynamicEntitiesInside = 0;
	int numActiveEntities;
	AABBSweepNode * node;
	int i, j;
	Entity** activeEntityArray;
	Entity* entity;
	AABB * oneab, * twoab;

	static List< Entity* > activeEntities;
	/// Clear the active entities list.
    activeEntities.Clear();
    for (j = 0; j < nodes.Size(); ++j)
	{
        node = nodes[j];
		if (debug == 1)
			std::cout<<"\nNode "<<j<<": "<<node->entity->name<<" "<<(node->type == AABBSweepNode::START? "START" : "STOP");
        /// If it's a start node, add it to the active list.
        if (node->type == AABBSweepNode::START)
		{
    //        std::cout<<"\nActive entities: "<<activeEntities.Size()<<". Adding that many pairs (probably?)";
            /// Also create pairs from it with all other already active entities...!
			numActiveEntities = activeEntities.Size();
			activeEntityArray = activeEntities.GetArray();
			if (debug == 3 && node->entity->name == "Player")
				std::cout<<"\nActive entities: "<<numActiveEntities;
			/// Since "entity" will be any kind, demand that the node entering now is dynamic. Will filter a bit.
			if (node->entity->physics->type == PhysicsType::DYNAMIC &&
				!(node->entity->physics->state & CollisionState::IN_REST))
				++dynamicEntitiesInside;
			if (!dynamicEntitiesInside)
			{
				// Just add and continue.
				activeEntities.AddItem(node->entity);
				continue;
			}
            for (int k = 0; k < numActiveEntities; ++k)
			{
                entity = activeEntityArray[k];
				// Check collision-filters.
				if (
					(
						(entity->physics->collisionCategory & node->entity->physics->collisionFilter) && 
						(entity->physics->collisionFilter & node->entity->physics->collisionCategory)
						) == false)
				{
					continue;
				}
				if (node->entity->physics->type != PhysicsType::DYNAMIC &&
					entity->physics->type != PhysicsType::DYNAMIC)
					continue; // Irrelevant.
				// Check other axes straight away.
				oneab = node->aabb;
				twoab = entity->aabb;
				if (oneab->max.z < twoab->min.z ||
					oneab->min.z > twoab->max.z ||
					oneab->max.y < twoab->min.y ||
					oneab->min.y > twoab->max.y)
				{
					continue;
				}

				EntityPair ep;
                /// Sort them by address (hopefully it works)
                ep.one = node->entity;
                ep.two = entity;
				/// check for duplicates?
				/// TODO: Check for duplicates in the entityPairs list?, since this functions is called iteratively.
				if (debug == 10)
					std::cout<<"\nAdding pair "<<entityPairs.Size()<<": "<<ep.one->name<<" "<<ep.one->worldPosition<<" & "<<ep.two->name<<" "<<ep.two->worldPosition;
				if (debug == 12)
					std::cout<<"\nAdding pair "<<entityPairs.Size()<<": "<<ep.one->name<<"  &  "<<ep.two->name;
                entityPairs.AddItem(ep);
            }
			activeEntities.AddItem(node->entity);
        }
        /// And if it's a stop node, just remove it from the active entities list!
        else if (node->type == AABBSweepNode::STOP)
		{
            activeEntities.RemoveItemUnsorted(node->entity);
			if (node->entity->physics->type == PhysicsType::DYNAMIC &&
				!(node->entity->physics->state & CollisionState::IN_REST))
				--dynamicEntitiesInside;
        }
    }
	return true;
}


//#define TEST_DYNAMIC_FOCUS_SORT
#ifdef TEST_DYNAMIC_FOCUS_SORT
	static List< Entity* > relevantEntities;
	relevantEntities = dynamicEntities;
	// Remove sleeping ones.
	Entity* entity;
	int i, j, k;
	static List<EntityPair> toAdd;
	List<AABBSweepNode*> & xAxis = axisNodeList[0];
	AABBSweepNode * start, * stop, * node;
	AABB * oneab, * twoab;
	static List< Entity* > processedDynamicEntities; // Used to filter for duplicates of collisions.
	processedDynamicEntities.Clear();
	for (i = 0; i < relevantEntities.Size(); ++i)
	{
		entity = relevantEntities[i];
		if (entity->physics->type != PhysicsType::DYNAMIC || entity->physics->state & CollisionState::IN_REST)
			continue;
		toAdd.Clear();
		processedDynamicEntities.Add(entity);
		/// Check all nodes between the start- and end-node of this entity.
		start = entity->physics->aabbSweepMin,
		stop = entity->physics->aabbSweepMax;
		/// Declare some filtering macros used in both the left and right searches.
#define SKIP_IF_COLLISION_FILTER_MISMATCH \
			if ((entity->physics->collisionCategory & node->entity->physics->collisionFilter && entity->physics->collisionFilter & node->entity->physics->collisionCategory) == false)\
				continue;
#define SKIP_IF_OUTSIDE_ZY \
			if (oneab->max.z < twoab->min.z || \
				oneab->min.z > twoab->max.z || \
				oneab->max.y < twoab->min.y || \
				oneab->min.y > twoab->max.y) \
				continue;
// #define SKIP_DYNAMIC_DUPLICATE if (node->entity->physics->type == PhysicsType::DYNAMIC && processedDynamicEntities.Exists(node->entity)) continue;
#define SKIP_DUPLICATES for (k = 0; k < toAdd.Size(); ++k) \
		{\
			EntityPair & pair = toAdd[k]; \
			if (pair.one == entity && pair.two == node->entity || \
				pair.one == node->entity && pair.two == entity) \
				continue; \
		}
#define MAIN_FILTERS \
	SKIP_IF_OUTSIDE_ZY; \
	SKIP_DUPLICATES;
		/// Check for pairs to the right, including those intersecting with us.
		for (j = start->index + 1; j < xAxis.Size(); ++j)
		{
			node = xAxis[j];
			// Check filters quickly.
			SKIP_IF_COLLISION_FILTER_MISMATCH;
			/// Compare AABBs for the other two axes.
			oneab = node->entity->aabb;
			twoab = entity->aabb;
			MAIN_FILTERS;
			// Pair up
			toAdd.Add(EntityPair(entity, node->entity));
		}
		/// Do the same search, but to the left, and this time excluding those inside us.
		for (j = start->index - 1; j >= 0; --j)
		{
			node = xAxis[j];
			SKIP_IF_COLLISION_FILTER_MISMATCH;
			/// Compare AABBs for the other two axes.
			oneab = node->entity->aabb;
			twoab = entity->aabb;
			MAIN_FILTERS;
			toAdd.Add(EntityPair(entity, node->entity));			
		}
		entityPairs.Add(toAdd);
	}
	timer2.Stop();
	int initialFilter = (int)timer2.GetMs();
	timer2.Stop();
#else
#endif