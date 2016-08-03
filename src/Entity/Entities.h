/// Emil Hedemalm
/// 2014-08-31 (2013-03-07 in .cpp)
/// Handles groups of entities.

#ifndef SELECTION_H
#define SELECTION_H

// #include "../Entity.h"
#include "List/List.h"
#include <MathLib.h>

class Entity;
class Camera;

#define MAX_SELECTED 1024

/** Class for handling selections/groups of entities. TODO: Consider entering sorting functions etc!
*/
class Entities : public List<Entity*> 
{
public:
	/// Default empty constructor
	Entities();
	virtual ~Entities();
	/// Copy constructor..!
	Entities(const Entities & otherSelection);
	Entities(const List<Entity*> & entityList);
	// New list from single entity.
	Entities(Entity * entity);

	/// Removes all entities that are outside the frustum.
	Entities CullByCamera(Camera * camera) const;
	/// Sorts by distance to selected position.
	void SortByDistance(const Vector3f & position);
	/// Calculates based on Z-depth of the camera's near-plane.
	void SortByDistanceToCamera(Camera * camera);

	/// Prints a simple list with entity names n stuff
	void ListEntities();
	/// Calls the MapManager to delete the entities.
	void DeleteEntities();

	/// Selects next entity using given one as reference for the previous one.
	Entities SelectNext(Entity * entity) const;
	/// Selects previous entity using given one as reference for the previous one.
	Entities SelectPrevious(Entity * entity) const;
	/// If the list has a specific name?
///	String name;
private:
	/// Inherited variables from List<Entity*>
};

#endif
