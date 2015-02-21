/// Emil Hedemalm
/// 2013-09-19
/// Class for handling broad-phase AABB collission detection

#ifndef AABBSWEEPER_H
#define AABBSWEEPER_H

#include "PhysicsLib/Shapes/AABB.h"
#include "List/List.h"

struct EntityPair 
{
	EntityPair();
	EntityPair(Entity * one, Entity * two);
    Entity * one;
    Entity * two;
    void PrintDetailed();
};

struct AABBSweepNode
{
    AABBSweepNode();
    enum types {
        NULL_TYPE,
        START,
        STOP,
    };
    AABB * aabb;
    Entity * entity;
	// Pointer to value.
	float * value;
    int type;
	bool sortedOnce;
};

class AABBSweeper {
public:
    AABBSweeper();
    virtual ~AABBSweeper();

    /// Enters or removes an entity from the lists to be sorted and evaluated.
    void RegisterEntity(Entity * entity);
    void UnregisterEntity(Entity * entity);

    void PrintSortedList();
    /// Returns the amount of nodes currently registered. Should always be registeredEntities * 2.
    int Nodes();

    /// Performs the sweep (including sort) and returns a list of all entity pairs whose AABBs are intersecting.
    /// Should be called once per physics frame if in use.
    List<EntityPair> Sweep();

	int AxesToWorkWith() {return axesToWorkWith;};
protected:

    enum axes {
        X_AXIS = 0,
        Y_AXIS = 1,
        Z_AXIS = 2,
        AXES = 3
    };

    /// Sorts the selected axis, using provided list and axis number
    void Sort(List<AABBSweepNode*> & listToSort, int axis);

    /// Determines which axis to sort. Default is 1, being only the X-axis and lazy comparison for the rest.
    int axesToWorkWith;
	int nodesToWorkWith; // Axes * 2
    int axesSorted;
    List<AABBSweepNode*> axisNodeList[AXES];
private:
};

#endif // AABBSWEEPER_H
