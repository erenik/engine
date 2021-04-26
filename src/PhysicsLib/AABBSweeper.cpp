/// Emil Hedemalm
/// 2013-09-19
/// Class for handling broad-phase AABB collission detection

#include "AABBSweeper.h"
#include "Entity/Entity.h"
#include "Physics/PhysicsProperty.h"
#include <iomanip>
#include "Timer/Timer.h"
#include "AABBSweeper/AABBSweepAxis.h"
#include "File/LogFile.h"

extern int debug;

#define TEST_TWO_AXES_FOCUS_SEARCH

EntityPair::EntityPair()
{
}
EntityPair::EntityPair(Entity* one, Entity* two)
: one(one), two(two)
{
}


void EntityPair::PrintDetailed()
{
    std::cout<<"\nComparing pair: "<<one<<" "<<one->worldPosition<<" & "<<two<<" "<<two->worldPosition;
    AABB * oneab = one->aabb, * twoab = two->aabb;
    std::cout<<"\nOne min/max: "<<oneab->min<<" "<<oneab->max;
    std::cout<<"\nTwo min/max: "<<twoab->min<<" "<<twoab->max;
}

AABBSweeper::AABBSweeper()
{
//	int subdivisionsZ = 8; // 8 used in the TIFS project. 1 for SpaceShooter
	divisions = 8;
	// Create a given amount of subdivisions in Z, that we later on sort in X. :)
	subdivisionLinesZ.AddItem(0.f);

	// Create 4 axes for the subdivision of XZ+, XZ-, X+Z-, and X-Z+
#define LARGE 1000000.f

	CreateAxes();	
}

AABBSweeper::~AABBSweeper()
{
	axes.ClearAndDelete();
	// Clear any remaining nodes.
	AABBSweepNode::FreeAll();
}

void AABBSweeper::CreateAxes()
{
	axes.ClearAndDelete();
	float minZ = -400, maxZ = 400;
	float range = maxZ - minZ;
	float rangePerSubdivision = range / divisions;
	for (int i = 0; i < divisions; ++i)
	{
		axes.AddItem(
			new AABBSweepAxis(X_AXIS, Vector3f(-LARGE,-LARGE, i == 0? -LARGE : minZ + rangePerSubdivision * i), Vector3f(LARGE,LARGE, i == (divisions - 1)? LARGE : minZ + rangePerSubdivision * (i + 1)))
		);
	}
}

void AABBSweeper::PrintSortedList()
{
	assert(false);
	/*
    std::cout<<"\nSorted list, by X:";
    List<AABBSweepNode*> & list = axisNodeList[0];
    for (int i = 0; i < list.Size(); ++i){
        AABBSweepNode * node = list[i];
        bool isStart = node->type == AABBSweepNode::START;
        std::cout<<"\n"<<std::setw(2)<<i<<": "<<node->entity->name<<" "<< (isStart? ("start") : ("stop"));
        std::cout<<" "<< (isStart? node->aabb->min[0] : node->aabb->max[0]);
    }*/
}

/// Enters or removes an entity from the lists to be sorted and evaluated.
void AABBSweeper::RegisterEntity(Entity* entity)
{
    assert(entity);
    assert(entity->physics);
	assert(entity->physics->collisionsEnabled);
	bool added = false;
	for (int i = 0; i < axes.Size(); ++i)
	{
		AABBSweepAxis * axis = axes[i];
		if (axis->ShouldBeHere(entity))
		{
			axis->AddEntity(entity);
			added = true;
		}
	}
	assert(added);
	if (entity->physics->type != PhysicsType::STATIC)
		movingEntities.AddItem(entity);
}

/// Clear registration, lets the list remain sorted..!
void AABBSweeper::UnregisterEntity(Entity* entity)
{
    assert(entity);
    assert(entity->physics);
	assert(entity->physics->collisionsEnabled);
	for (int i = 0; i < entity->physics->aabbSweepNodes.Size(); ++i)
	{
		AABBSweepNode * node = entity->physics->aabbSweepNodes[i];
		node->axis->RemoveEntity(entity);
		i -= 2;
		if (i < 0)
			i = 0;
	}	
	if (entity->physics->type != PhysicsType::STATIC)
		movingEntities.RemoveItemUnsorted(entity);

	// Check that all nodes with this entity have surely been discarded.
	for (int i = 0; i < axes.Size(); ++i)
	{
        AABBSweepAxis * axis = axes[i];
		for (int j = 0; j < axis->nodes.Size(); ++j)
		{
			AABBSweepNode * an = axis->nodes[j];
			if (an->entity == entity)
				assert(false);
		}
    }
}
/// Clear all internal nodes and references to entities.
void AABBSweeper::Clear()
{
	for (int i = 0; i < axes.Size(); ++i)
	{
		axes[i]->Clear();
	}
	movingEntities.Clear();
}

/// Returns the amount of nodes currently registered. Should always be registeredEntities * 2.
int AABBSweeper::Nodes()
{
    /// Only sort one axis to begin with?
	int numNodes = 0;
	for (int i = 0; i < axes.Size(); ++i)
	{
        AABBSweepAxis * axis = axes[i];
		numNodes += axis->nodes.Size();
    }
    return numNodes;
}

/// Performs the sweep (including sort) and returns a list of all entity pairs whose AABBs are intersecting.
/// Should be called once per physics frame if in use.
List<EntityPair> AABBSweeper::Sweep()
{
	// Primary sorting, X.
	Timer timer;
	timer.Start();
	/// Move them about as needed.
	for (int i = 0; i < movingEntities.Size(); ++i)
	{
		Entity* entity = movingEntities[i];
		/// check if close to any of our separation lines.
		for (int j = 0; j < axes.Size(); ++j)
		{
			AABBSweepAxis * axis = axes[j];
			bool shouldBeHere = true;
			float diff1 = axis->min.z - entity->aabb->max.z;
			/// If much outside, assume it has already been removed.
			if (diff1 > 10.f)
				continue;
			float diff2 = entity->aabb->min.z - axis->max.z;
			if (diff2 > 10.f)
				continue;
			/// Check if outside.
			if (diff1 > 0)
				shouldBeHere = false;
			else if (diff2 > 0)
				shouldBeHere = false;
			bool existsWithin = entity->physics->axes.Exists(axis);
			if (!shouldBeHere)
			{
				/// Remove if needed.
				if (existsWithin) {
					LogPhysics("Removing entity from axis within AABB Sweeper", INFO);
					axis->RemoveEntity(entity);
				}
				continue;
			}
			// If not, add it. Add iiiit o.o
			if (!existsWithin)
				axis->AddEntity(entity);
		}

		/*
		List<AABBSweepAxis*> currentAxes;
		List<AABBSweepAxis*> otherAxes = axes;
		// Check current placements?
		for (int k = 0; k < movingEntity->physics->aabbSweepNodes.Size(); ++k)
		{
			AABBSweepNode * node = movingEntity->physics->aabbSweepNodes[k];
			AABBSweepAxis * axis = node->axis;
			bool shouldBeHere = axis->ShouldBeHere(movingEntity);
			if (shouldBeHere)
			{
				currentAxes.Add(axis);
				otherAxes.RemoveItemUnsorted(axis);
			}
			else {
				axis->RemoveEntity(movingEntity);
				otherAxes.RemoveItemUnsorted(axis);
				k -= 1;
			}
		}

		// Check if we should enter any other axis.
		for (int j = 0; j < otherAxes.Size(); ++j)
		{
			AABBSweepAxis * axis = otherAxes[j];
			if (axis->ShouldBeHere(movingEntity))
				axis->AddEntity(movingEntity);
		}
		*/
	}
	timer.Stop();
	int movingAround = timer.GetMs();

	timer.Start();
	// Sort all axises.
	for (int i = 0; i < axes.Size(); ++i)
	{
		AABBSweepAxis * axis = axes[i];
		axis->Sort();
	}
	timer.Stop();
	int sorting = (int) timer.GetMs();

	// o.o
	timer.Start();
    /// Sweepty sweep. Static so re-allocation aren't performed in vain every physics-frame.
	static List< Entity* > activeEntities;
	activeEntities.Clear();
    static List<EntityPair> entityPairs;
	entityPairs.Clear();
	for (int i = 0; i < axes.Size(); ++i)
	{
		AABBSweepAxis * axis = axes[i];
		axis->GetPairs(entityPairs);		
	}

//    std::cout<<"\nPairs after sweep: "<<entityPairs.Size();
	timer.Stop();
	int filtering = timer.GetMs();
 //   std::cout<<"\nAA-AABB pairs after examining the remaining two axes: "<<entityPairs.Size();
    return entityPairs;
}

