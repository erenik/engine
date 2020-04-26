/// Emil Hedemalm
/// 2013-09-19
/// Class for handling broad-phase AABB collission detection

#ifndef AABBSWEEPER_H
#define AABBSWEEPER_H

#include "PhysicsLib/Shapes/AABB.h"
#include "List/List.h"

class AABBSweepNode;
class AABBSweepAxis;

class AABBSweeper {
public:
    AABBSweeper();
    virtual ~AABBSweeper();

    /// Enters or removes an entity from the lists to be sorted and evaluated.
    void RegisterEntity(EntitySharedPtr entity);
    void UnregisterEntity(EntitySharedPtr entity);

    void PrintSortedList();
    /// Returns the amount of nodes currently registered. Should always be registeredEntities * 2.
    int Nodes();
	/// Clear all internal nodes and references to entities.
	void Clear();

    /// Performs the sweep (including sort) and returns a list of all entity pairs whose AABBs are intersecting.
    /// Should be called once per physics frame if in use.
    List<EntityPair> Sweep();

	int AxesToWorkWith() {return axesToWorkWith;};

	/// Creates the sorting axes and delimiters.
	void CreateAxes();
	/// Default 8 for TIFS, 1 for SpaceShooter. Used to separate and have several axes to reduce sorting burden.
	int divisions; 

protected:

    /// Sorts the selected axis, using provided list and axis number
    void Sort(List<AABBSweepNode*> & listToSort, int axis);

    /// Determines which axis to sort. Default is 1, being only the X-axis and lazy comparison for the rest.
    int axesToWorkWith;
	int nodesToWorkWith; // Axes * 2
    int axesSorted;



	/// o.o
    List<AABBSweepAxis*> axes;
private:
	List<float> subdivisionLinesZ;
	/// For updating entities in the various subdivided axes.
	List< std::shared_ptr<Entity> > movingEntities;
};

#endif // AABBSWEEPER_H
