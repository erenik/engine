#ifndef SELECTION_H
#define SELECTION_H

// #include "../Entity.h"
class Entity;
class Camera;
#include "List/List.h"
#include <MathLib.h>

#define MAX_SELECTED 1024

/** Class for handling selections/groups of entities. TODO: Consider entering sorting functions etc!
*/
class Selection : public List<Entity*> {
public:
	/// Default empty constructor
	Selection();
	virtual ~Selection();
	/// Copy constructor..!
	Selection(const Selection & otherSelection);
	Selection(const List<Entity*> & entityList);

	/// Removes all entities that are outside the frustum.
	Selection CullByCamera(Camera * camera) const;
	/// Sorts by distance to selected position.
	void SortByDistance(Vector3f position);

	/// Prints a simple list with entity names n stuff
	void ListEntities();
	/// Calls the MapManager to delete the entities.
	void DeleteEntities();

	/// Selects next entity using given one as reference for the previous one.
	Selection SelectNext(Entity * entity) const;
	/// Selects previous entity using given one as reference for the previous one.
	Selection SelectPrevious(Entity * entity) const;
private:
	/// Inherited variables from List<Entity*>
};

#endif
