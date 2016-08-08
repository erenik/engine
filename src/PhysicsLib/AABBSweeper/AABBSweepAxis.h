/// Emil Hedemalm
/// 2015-02-21
/// AABBSweeper 
/// For division nof AABB-sweep grid.

#include "PhysicsLib/Shapes/AABB.h"
#include "List/List.h"
#include "String/AEString.h"

enum axes {
    X_AXIS = 0,
    Y_AXIS = 1,
    Z_AXIS = 2,
    AXES = 3
};

class AABBSweepAxis;

class AABBSweepNode
{
    AABBSweepNode();
public:
	~AABBSweepNode();

	static AABBSweepNode * New();
	static void FreeForUse(AABBSweepNode * node); // Moves to the usable-array.
	static void FreeAllForUse(); // Just moves to the usable-array.
	static void FreeAll(); // Actually deletes.
    enum types {
        NULL_TYPE,
        START,
        STOP,
    };
	String name;
    AABB * aabb;
	AABBSweepAxis * axis;
    Entity * entity;
	// Pointer to value.
	float * value;
    int type;
	bool sortedOnce;
	// index in the sorted list.
	int index;
private:
	static List<AABBSweepNode*> allTheNodes,
		freeNodes;
};

class AABBSweepAxis 
{
	friend class AABBSweeper;
public:
	/// Bounds
	AABBSweepAxis(int axis, ConstVec3fr min, ConstVec3fr max);
	~AABBSweepAxis();

	void AddEntity(Entity * entity);
	void RemoveEntity(Entity * entity);
	void Clear(); // Clears all references.

	bool ShouldBeHere(Entity * entity);

	void Sort();
	bool GetPairs(List<EntityPair> & pairList);

	List<AABBSweepNode*> nodes;
private:
	int axis;
	Vector3f min, max;

	void SortDynamicFocus();
	bool FindPairsLinearPlusSearch(List<EntityPair> & list);

	/// Stored internally for optimized pairing of entities.
	List<Entity*> dynamicEntities;
};



