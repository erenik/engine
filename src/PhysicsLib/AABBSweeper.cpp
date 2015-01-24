/// Emil Hedemalm
/// 2013-09-19
/// Class for handling broad-phase AABB collission detection

#include "AABBSweeper.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include <iomanip>

void EntityPair::PrintDetailed()
{
    std::cout<<"\nComparing pair: "<<one<<" "<<one->position<<" & "<<two<<" "<<two->position;
    AABB * oneab = one->aabb, * twoab = two->aabb;
    std::cout<<"\nOne min/max: "<<oneab->min<<" "<<oneab->max;
    std::cout<<"\nTwo min/max: "<<twoab->min<<" "<<twoab->max;
}

AABBSweepNode::AABBSweepNode(){
    aabb = NULL;
    entity = NULL;
    type = NULL_TYPE;
};

AABBSweeper::AABBSweeper()
{
    //ctor
    axesToWorkWith = 1;
    axesSorted = 0;
}

AABBSweeper::~AABBSweeper()
{
	axisNodeList[0].ClearAndDelete();
	axisNodeList[1].ClearAndDelete();
    //dtor
}

void AABBSweeper::PrintSortedList(){
    std::cout<<"\nSorted list, by X:";
    List<AABBSweepNode*> & list = axisNodeList[0];
    for (int i = 0; i < list.Size(); ++i){
        AABBSweepNode * node = list[i];
        bool isStart = node->type == AABBSweepNode::START;
        std::cout<<"\n"<<std::setw(2)<<i<<": "<<node->entity->name<<" "<< (isStart? ("start") : ("stop"));
        std::cout<<" "<< (isStart? node->aabb->min.x : node->aabb->max.x);
    }
}

/// Enters or removes an entity from the lists to be sorted and evaluated.
void AABBSweeper::RegisterEntity(Entity * entity)
{
    assert(entity);
    assert(entity->physics);
    AABB * aabb = entity->aabb;
    AABBSweepNode * nodes[2];
    nodes[0] = NULL;
    nodes[1] = NULL;
    for (int i = 0; i < 2; ++i)
	{
        nodes[i] = new AABBSweepNode();
        nodes[i]->entity = entity;
        nodes[i]->aabb = aabb;
        nodes[i]->type = (i%2 ? AABBSweepNode::START : AABBSweepNode::STOP);
//        std::cout<<"\nNode type created: "<<nodes[i]->type<<" "<< (nodes[i]->type == AABBSweepNode::START ? "Start" : "Stop") ;
    }

    for (int i = 0; i < axesToWorkWith; ++i)
	{
        axisNodeList[i].Add(nodes[0]);
        axisNodeList[i].Add(nodes[1]);
    }
    /// Save the nodes in the entity for future usage (deletion primarily).
    entity->physics->aabbSweepNodes[0] = nodes[0];
    entity->physics->aabbSweepNodes[1] = nodes[1];
}

/// Clear registration, lets the list remain sorted..!
void AABBSweeper::UnregisterEntity(Entity * entity)
{
  //  std::cout<<"\nUnregistering entity from AABBSweeper: "<<entity->name;
	int removed = 0;
    for (int i = 0; i < axesToWorkWith; ++i)
	{
        /// Needed reference & so that it didn't copy the fucking list!
        List<AABBSweepNode*> & axis = axisNodeList[i];
		for (int j = 0; j < axis.Size(); ++j)
		{
			// Remove all nodes that reference the entity.
			AABBSweepNode * node = axis[j];
			if (node->entity == entity)
			{
				axis.Remove(node, ListOption::RETAIN_ORDER);
				--j;
				delete node;
			}
		}
    }
	if (removed != 2 && removed != 0)
		std::cout<<"\nRemoved: "<<removed;
}

/// Returns the amount of nodes currently registered. Should always be registeredEntities * 2.
int AABBSweeper::Nodes(){
    assert(axesToWorkWith == 1 && "Sorry! Support only 1 active axis for the time being!");
    /// Only sort one axis to begin with?
    for (int i = 0; i < axesToWorkWith; ++i){
        List<AABBSweepNode*> axis = axisNodeList[i];
    //    std::cout<<"\nNodes to sort: "<<axis.Size();
        return axis.Size();
    }
    assert(false);
    return 0;
}

/// Performs the sweep (including sort) and returns a list of all entity pairs whose AABBs are intersecting.
/// Should be called once per physics frame if in use.
List<EntityPair> AABBSweeper::Sweep()
{
    assert(axesToWorkWith == 1 && "Sorry! Support only 1 active axis for the time being!");
    /// Only sort one axis to begin with?
    for (int i = 0; i < axesToWorkWith; ++i){
        /// Reference...!
        List<AABBSweepNode*> & axis = axisNodeList[i];
      //  std::cout<<"\nNodes to sort: "<<axis.Size();
        Sort(axis, i);
    }

    /// Sweepty sweep. Static so re-allocation aren't performed in vain every physics-frame.
	static List<Entity*> activeEntities;
    static List<EntityPair> entityPairs;
	entityPairs.Clear();
    for (int i = 0; i < axesToWorkWith; ++i)
	{
        /// Clear the active entities list.
        activeEntities.Clear();
        List<AABBSweepNode*> axis = axisNodeList[i];
        for (int j = 0; j < axis.Size(); ++j)
		{
            AABBSweepNode * node = axis[j];
            /// If it's a start node, add it to the active list.
            if (node->type == AABBSweepNode::START)
			{
        //        std::cout<<"\nActive entities: "<<activeEntities.Size()<<". Adding that many pairs (probably?)";
                /// Also create pairs from it with all other already active entities...!
                for (int k = 0; k < activeEntities.Size(); ++k)
				{
                    Entity * entity = activeEntities[k];
                    /// Skip self, but create a pair for the remaining.
                    if (entity == node->entity)
                        continue;
					if (
						(
							(entity->physics->collisionCategory & node->entity->physics->collisionFilter) && 
							(entity->physics->collisionFilter & node->entity->physics->collisionCategory)
							) == false)
					{
						continue;
					}
					EntityPair ep;
                    /// Sort them by address (hopefully it works)
                    if (entity < node->entity){
                        ep.one = entity;
                        ep.two = node->entity;
                    }
                    else {
                        ep.one = node->entity;
                        ep.two = entity;
                    }
          //          std::cout<<"\nAdding pair: "<<ep.one<<" "<<ep.one->position<<" & "<<ep.two<<" "<<ep.two->position;
                    entityPairs.Add(ep);
                }
                activeEntities.Add(node->entity);
            }
            /// And if it's a stop node, just remove it from the active entities list!
            else if (node->type == AABBSweepNode::STOP){
                activeEntities.Remove(node->entity);
            }
        }
    }
//    std::cout<<"\nPairs after sweep: "<<entityPairs.Size();

 //   std::cout<<"\nBegin examining Y/Z axes...";
//    std::cout<<"\nAABB-AABB Pairs after initial axis sweep: "<<entityPairs.Size();
    for (int i = 0; i < entityPairs.Size(); ++i){
        /// Assume we've got to examine the Y and Z axes only now!
        EntityPair ep = entityPairs[i];
        Entity * one = ep.one, * two = ep.two;
     //   ep.PrintDetailed();
        AABB * oneab = one->aabb, * twoab = two->aabb;
        if (oneab->max.z < twoab->min.z ||
            oneab->min.z > twoab->max.z ||
            oneab->max.y < twoab->min.y ||
            oneab->min.y > twoab->max.y)
        {
   //         std::cout<<"\nRemoving pair!";
            /// Removes it!
            entityPairs.RemoveIndex(i, ListOption::RETAIN_ORDER);
            /// Decrement i so we don't skip evaluating any of theze pairz!
            --i;
        }
        else
            ;//         std::cout<<"\nKeeping pair.";
    }
  //  std::cout<<"\nSecondary axises checked, pairs are now as follows:";
    for (int i = 0; i < entityPairs.Size(); ++i){
        EntityPair ep = entityPairs[i];
  //      std::cout<<"\nPair "<<i<<": "<<ep.one<<" "<<ep.one->position<<" & "<<ep.two<<" "<<ep.two->position;
    }
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
    /// Manual sort, commence!
    for (int i = 1; i < items; ++i)
	{
        /// Grab current node, starting from index 0 and moving towards the end.
        currentNode = listArray[i];
        /// Mark this index as empty for the time being.
        emptySlot = i;
		/// Extract current value depending on the node type (start, stop) and axis.
        if (currentNode->type == AABBSweepNode::START)
            currentValue = currentNode->aabb->min[axis];
        else if (currentNode->type == AABBSweepNode::STOP)
            currentValue = currentNode->aabb->max[axis];
        else {
            std::cout<<"\nBad node type: "<<currentNode->type;
            assert(false && "Bad AABBSweepNode type");
        }

        /// Assume all previous nodes have already been sorted, and compare only with them (comparing downwards)!
        for (int j = i-1; j >= 0; --j)
		{
            comparisonNode = listArray[j];

            /// Extract comparison value depending on the node type (start, stop) and axis.
            if (comparisonNode->type == AABBSweepNode::START)
                comparisonValue = comparisonNode->aabb->min[axis];
            else if (comparisonNode->type == AABBSweepNode::STOP)
                comparisonValue = comparisonNode->aabb->max[axis];
            else
                assert(false && "Bad AABBSweepNode type");

            /// If the current value is less, move up the comparer and move down the empty slot index.
            if (currentValue < comparisonValue){
                listArray[emptySlot] = listArray[j];
                --emptySlot;
            }
            /// A lesser value was encountered! Stop shuffling around and place the current node in the now empty slot.
            else {
                /// And break ze inner loopty loop
                break;
            }
        }
        listArray[emptySlot] = currentNode;
    }
}
