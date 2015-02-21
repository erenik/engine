/// Emil Hedemalm
/// 2013-09-19
/// Class for handling broad-phase AABB collission detection

#include "AABBSweeper.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include <iomanip>
#include "Timer/Timer.h"

extern int debug;

//#define TEST_THREE_AXES

EntityPair::EntityPair()
{
}
EntityPair::EntityPair(Entity * one, Entity * two)
: one(one), two(two)
{
}


void EntityPair::PrintDetailed()
{
    std::cout<<"\nComparing pair: "<<one<<" "<<one->position<<" & "<<two<<" "<<two->position;
    AABB * oneab = one->aabb, * twoab = two->aabb;
    std::cout<<"\nOne min/max: "<<oneab->min<<" "<<oneab->max;
    std::cout<<"\nTwo min/max: "<<twoab->min<<" "<<twoab->max;
}

AABBSweepNode::AABBSweepNode()
{
    aabb = NULL;
    entity = NULL;
    type = NULL_TYPE;
	sortedOnce = false;
	value = NULL;
};

AABBSweeper::AABBSweeper()
{
    //ctor
#ifdef TEST_THREE_AXES
	axesToWorkWith = 1;
#else
    axesToWorkWith = 1;
#endif
	nodesToWorkWith = axesToWorkWith * 2;
    axesSorted = 0;
}

AABBSweeper::~AABBSweeper()
{
	for (int i = 0; i < AXES; ++i)
	{
		axisNodeList[i].ClearAndDelete();
	}
    //dtor
}

void AABBSweeper::PrintSortedList(){
    std::cout<<"\nSorted list, by X:";
    List<AABBSweepNode*> & list = axisNodeList[0];
    for (int i = 0; i < list.Size(); ++i){
        AABBSweepNode * node = list[i];
        bool isStart = node->type == AABBSweepNode::START;
        std::cout<<"\n"<<std::setw(2)<<i<<": "<<node->entity->name<<" "<< (isStart? ("start") : ("stop"));
        std::cout<<" "<< (isStart? node->aabb->min[0] : node->aabb->max[0]);
    }
}

/// Enters or removes an entity from the lists to be sorted and evaluated.
void AABBSweeper::RegisterEntity(Entity * entity)
{
    assert(entity);
    assert(entity->physics);
	assert(entity->physics->collisionsEnabled);
    AABB * aabb = entity->aabb;
	AABBSweepNode * nodes[6];
	for (int i = 0; i < 6; ++i)
	{
		nodes[i] = NULL;
	}
	// Create the nodes.
	for (int i = 0; i < axesToWorkWith * 2; ++i)
	{
        nodes[i] = new AABBSweepNode();
        nodes[i]->entity = entity;
        nodes[i]->aabb = aabb;
        nodes[i]->type = (i%2 ? AABBSweepNode::START : AABBSweepNode::STOP);
		nodes[i]->value = &(i%2 ? aabb->min : aabb->max)[i/2];
//        std::cout<<"\nNode type created: "<<nodes[i]->type<<" "<< (nodes[i]->type == AABBSweepNode::START ? "Start" : "Stop") ;
    }


	for (int i = 0; i < axesToWorkWith * 2; ++i)
	{
		axisNodeList[i / 2].Add(nodes[i]);    
	   	/// Save the nodes in the entity for future usage (deletion primarily).
	    entity->physics->aabbSweepNodes[i] = nodes[i];
	}
}

/// Clear registration, lets the list remain sorted..!
void AABBSweeper::UnregisterEntity(Entity * entity)
{
  //  std::cout<<"\nUnregistering entity from AABBSweeper: "<<entity->name;
	int removed = 0;
    for (int i = 0; i < 3; ++i)
	{
        /// Needed reference & so that it didn't copy the fucking list!
        List<AABBSweepNode*> & axis = axisNodeList[i];
		for (int j = 0; j < axis.Size(); ++j)
		{
			// Remove all nodes that reference the entity.
			AABBSweepNode * node = axis[j];
			if (node->entity == entity)
			{
				axis.RemoveItem(node);
				--j;
				delete node;
			}
		}
    }
	if (removed != 2 && removed != 0)
		std::cout<<"\nRemoved: "<<removed;
}

/// Returns the amount of nodes currently registered. Should always be registeredEntities * 2.
int AABBSweeper::Nodes()
{
    /// Only sort one axis to begin with?
	int numNodes = 0;
    for (int i = 0; i < axesToWorkWith; ++i){
        List<AABBSweepNode*> axis = axisNodeList[i];
        numNodes += axis.Size();
    }
    return numNodes;
}

/// Performs the sweep (including sort) and returns a list of all entity pairs whose AABBs are intersecting.
/// Should be called once per physics frame if in use.
List<EntityPair> AABBSweeper::Sweep()
{
	Timer timer;
	timer.Start();
    /// Only sort one axis to begin with?
    for (int i = 0; i < axesToWorkWith; ++i){
        /// Reference...!
        List<AABBSweepNode*> & axis = axisNodeList[i];
      //  std::cout<<"\nNodes to sort: "<<axis.Size();
        Sort(axis, i);
    }
	timer.Stop();
	int sorting = timer.GetMs();

	timer.Start();

    /// Sweepty sweep. Static so re-allocation aren't performed in vain every physics-frame.
	static List<Entity*> activeEntities;
    static List<EntityPair> entityPairs;
	activeEntities.Clear();
	entityPairs.Clear();
	Timer timer2;
	timer2.Start();

#ifdef TEST_THREE_AXES
	/// For amount of nodes in each list.
	int num = axisNodeList[0].Size();
    List<AABBSweepNode*> & xAxis = axisNodeList[0];
    List<AABBSweepNode*> & yAxis = axisNodeList[1];
    List<AABBSweepNode*> & zAxis = axisNodeList[2];
	AABBSweepNode * xNode, * yNode, * zNode;
	List<Entity*> entering;
	static List<EntityPair> pairs;

	Entity * enterX, * enterY, * enterZ,
		* exitX, * exitY, * exitZ;
	for (int i = 0; i < num; ++i)
	{
		/// Clear pairs to add.
		pairs.Clear();
		/// Check node in each axes list.
		xNode = xAxis[i];
		yNode = yAxis[i];
		zNode = zAxis[i];

		/// Check which are entering
		enterX = xNode->type == AABBSweepNode::START? xNode->entity : NULL;
		enterY = yNode->type == AABBSweepNode::START? yNode->entity : NULL;
		enterZ = zNode->type == AABBSweepNode::START? zNode->entity : NULL;

		/// Remove duplicates pointers early on. Happens for large entities.
		if (enterX == enterY)
			enterY = NULL;
		if (enterY == enterZ)
			enterZ = NULL;
		if (enterX == enterZ)
			enterZ = NULL;
	
		/// Produce pairs based on the entering nodes, and the currently active nodes.
        for (int k = 0; k < activeEntities.Size(); ++k)
		{
            Entity * entity = activeEntities[k];
			/// Create pair with all of the nodes entering.
			
			// First check if non-NULL.
			// Check collision-filters.
			// Require at least 1 dynamic non-resting entity.
			// Dismiss matches with self lastly.
#define ADD_PAIR_IF_GOOD(e) \
		if (\
			e && \
			(\
				(entity->physics->collisionCategory & e->physics->collisionFilter) && \
				(entity->physics->collisionFilter & e->physics->collisionCategory)\
			) && \
			(\
				(e->physics->type == PhysicsType::DYNAMIC && !(e->physics->state & PhysicsState::IN_REST)) ||\
				(entity->physics->type == PhysicsType::DYNAMIC && !(entity->physics->state & PhysicsState::IN_REST)) \
			) && \
			e != entity \
		)\
			pairs.Add(EntityPair(entity, e));
			
			ADD_PAIR_IF_GOOD(enterX);
			ADD_PAIR_IF_GOOD(enterY);
			ADD_PAIR_IF_GOOD(enterZ);
        }
		// For-loop done, add the entity pairs.
        entityPairs.Add(pairs);

		// Add the new entities to the list of active entities too.
#define ADD_IF_NOT_EXIST(e) \
		if (e && !activeEntities.Exists(e))\
			activeEntities.AddItem(e);

		ADD_IF_NOT_EXIST(enterX);
		ADD_IF_NOT_EXIST(enterY);
		ADD_IF_NOT_EXIST(enterZ);

		/// Remove all exiting scope.
		exitX = xNode->type == AABBSweepNode::STOP? xNode->entity : NULL;
		exitY = yNode->type == AABBSweepNode::STOP? yNode->entity : NULL;
		exitZ = zNode->type == AABBSweepNode::STOP? zNode->entity : NULL;
		if (exitX)
			activeEntities.RemoveItemUnsorted(exitX);
		if (exitY)
			activeEntities.RemoveItemUnsorted(exitY);
		if (exitZ)
			activeEntities.RemoveItemUnsorted(exitZ);	
	}
#else
	/// Old loop.
	int dynamicEntitiesInside = 0;
    for (int i = 0; i < axesToWorkWith; ++i)
	{
        /// Clear the active entities list.
        activeEntities.Clear();
        List<AABBSweepNode*> & axis = axisNodeList[i];
        for (int j = 0; j < axis.Size(); ++j)
		{
            AABBSweepNode * node = axis[j];
			if (debug == 1)
				std::cout<<"\nNode "<<j<<": "<<node->entity->name<<" "<<(node->type == AABBSweepNode::START? "START" : "STOP");
            /// If it's a start node, add it to the active list.
            if (node->type == AABBSweepNode::START)
			{
        //        std::cout<<"\nActive entities: "<<activeEntities.Size()<<". Adding that many pairs (probably?)";
                /// Also create pairs from it with all other already active entities...!
				int numActiveEntities = activeEntities.Size();
				Entity ** activeEntityArray = activeEntities.GetArray();
				if (debug == 3 && node->entity->name == "Player")
					std::cout<<"\nActive entities: "<<numActiveEntities;
				/// Since "entity" will be any kind, demand that the node entering now is dynamic. Will filter a bit.
				if (node->entity->physics->type == PhysicsType::DYNAMIC &&
					!(node->entity->physics->state & PhysicsState::IN_REST))
					++dynamicEntitiesInside;
				if (!dynamicEntitiesInside)
				{
					// Just add and continue.
					activeEntities.AddItem(node->entity);
					continue;
				}
                for (int k = 0; k < numActiveEntities; ++k)
				{
                    Entity * entity = activeEntityArray[k];
					// Check collision-filters.
					if (
						(
							(entity->physics->collisionCategory & node->entity->physics->collisionFilter) && 
							(entity->physics->collisionFilter & node->entity->physics->collisionCategory)
							) == false)
					{
						continue;
					}
					/// Skip self, but create a pair for the remaining.
                    if (entity == node->entity)
                        continue;
					// Check other axes straight away.
					AABB * oneab = node->aabb, * twoab = entity->aabb;
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
          //          std::cout<<"\nAdding pair: "<<ep.one<<" "<<ep.one->position<<" & "<<ep.two<<" "<<ep.two->position;
                    entityPairs.AddItem(ep);
                }
				activeEntities.AddItem(node->entity);
            }
            /// And if it's a stop node, just remove it from the active entities list!
            else if (node->type == AABBSweepNode::STOP)
			{
                activeEntities.RemoveItemUnsorted(node->entity);
				if (node->entity->physics->type == PhysicsType::DYNAMIC &&
					!(node->entity->physics->state & PhysicsState::IN_REST))
					--dynamicEntitiesInside;
            }
        }
    }
#endif
	timer2.Stop();
	int initialFilter = timer2.GetMs();
//    std::cout<<"\nPairs after sweep: "<<entityPairs.Size();
	timer.Stop();
	int filtering = timer.GetMs();
 //   std::cout<<"\nAA-AABB pairs after examining the remaining two axes: "<<entityPairs.Size();
    return entityPairs;
}

/*
enum axes {
    NULL_AXIS,
    X,
    Y,
    Z
};
*/
/// Insertionsort reference: http://en.wikipedia.org/wiki/Insertion_sort
/// Sorts the selected axis, using provided list and axis number (X being 1, Y 2 and Z 3)
void AABBSweeper::Sort(List<AABBSweepNode*> & listToSort, int axis)
{
    /// Insertion sort the shit out of the list, yo!
    AABBSweepNode ** listArray = listToSort.GetArray();
    int items = listToSort.Size();

    AABBSweepNode * currentNode, * comparisonNode, * tempSwapNode;
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
                listArray[emptySlot] = listArray[j];
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
		// Mark the sortedOnce flag, so we only sort static entities once upon registration.
		currentNode->sortedOnce = true;
    }
	if (debug == 2)
	{
		std::cout<<"\nSorted nodes in AABBSweeper";
		for (int i = 0; i < listToSort.Size(); ++i)
		{
			AABBSweepNode * node = listToSort[i];
			std::cout<<"\n "<<i<<": "<<node->entity->name<<" "<<
				(	(node->type == AABBSweepNode::START)? 
					("START X: "+String(node->aabb->min.x)) : 
					("STOP X: "+String(node->aabb->max.x))
				);
		}
	}
}
