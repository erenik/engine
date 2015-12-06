/// Emil Hedemalm
/// 2013-03-07
/// Handles groups of entities.

#include "Entity/Entities.h"
#include <cassert>

#include "Maps/MapManager.h"
#include "../Entity/Entity.h"
#include "Graphics/Camera/Camera.h"
#include <cstring>

#include "Graphics/GraphicsProperty.h"

Entities::Entities()
: List<Entity*>()
{

};

Entities::~Entities()
{
	
};

/// Copy constructor..!
Entities::Entities(const Entities& otherEntities) 
: List<Entity*> ((List<Entity*>&)otherEntities) 
{
	// Already done in List<Entity*> constructor?
	/*
	currentItems = otherEntities.currentItems;
	arrLength = currentItems;
	arr = new Entity * [currentItems];
	for (int i = 0; i < currentItems; ++i){
		arr[i] = otherEntities.arr[i];
	}
	*/
}

/// Inherit-Copy constructor
Entities::Entities(const List<Entity*> &entityList) 
: List<Entity*> ((List<Entity*>&)entityList)
{
	// Already done in List<Entity*> constructor?
	/*
	currentItems = entityList.Size();
	arrLength  = currentItems;
	arr = new Entity * [currentItems];
	for (int i = 0; i < currentItems; ++i){
		arr[i] = entityList[i];
	}*/
}

// New list from single entity.
Entities::Entities(Entity * entity)
	: List<Entity*>(entity)
{
}


/// Removes all entities that are outside the frustum.
Entities Entities::CullByCamera(Camera * camera) const
{
	Entities culled;
	int found = 0;
	for (int i = 0; i < this->currentItems; ++i)
	{
		if (arr[i] == NULL)
			continue;
		if (camera->GetFrustum().SphereInFrustum(arr[i]->worldPosition, arr[i]->radius))
			culled.Add(arr[i]);
		++found;
		if (found >= currentItems)
			break;
	}
	return culled;
}

/// Sorts by distance to selected position.
void Entities::SortByDistance(ConstVec3fr position) 
{
	float distance[MAX_SELECTED];
	memset(distance, 0, sizeof(float) * MAX_SELECTED);
	for (int i = 0; i < currentItems; ++i)
	{
		/// Calculate distance
		distance[i] = (position - arr[i]->worldPosition).Length();
		/// Insertion sort for now, all previous entities have already gotten
		/// a distance and can thus be compared.
		for (int j = 0; j < i; ++j){
			if (distance[j] > distance[i]){
				// Insert it here, pushing down the remaining values.
				float tmpDistance = distance[i];
				Entity * tmpEntity = arr[i];
				/// First swap done, now push down the remaining entities that were already sorted.
				for (int k = i; k > j; --k){
					distance[k] = distance[k-1];
					arr[k] = arr[k-1];
				}
				distance[j] = tmpDistance;
				arr[j] = tmpEntity;
				/// Break once insert is done!
				break;
			}
		}
	}
}

/// Calculates based on Z-depth of the camera's near-plane.
void Entities::SortByDistanceToCamera(Camera * camera)
{
	// Find the camera near-plane.
	Frustum frustum = camera->GetFrustum();
	Plane plane(frustum.hitherBottomLeft, frustum.hitherBottomRight, frustum.hitherTopRight);

	Entity * entity, * entity2, * tmp;
	/// Use insertion sort, as we can assume that the entities will remain nearly sorted all the time?
	/// http://en.wikipedia.org/wiki/Insertion_sort
	for (int i = 0; i < currentItems; ++i)
	{
		// Calculate distances as we go.
		entity = arr[i];
		entity->graphics->zDepth = -plane.Distance(entity->worldPosition);
		tmp = entity;
		// Compare with previous items.
		for (int j = i - 1; j >= 0; --j)
		{
			entity2 = arr[j];
			// If zdepth is lower on tmp, move entity2 up one step.
			if (tmp->graphics->zDepth > entity2->graphics->zDepth)
			{
				arr[j + 1] = arr[j]; 
			}
			// Once we find another item with a lesser depth, place tmp in the previous spot.
			else {
				arr[j + 1] = tmp;
				tmp = 0;
				// Break inner loop.
				break;
			}
		}
		// Special case, placing the closest one.
		if (tmp)
			arr[0] = tmp;
	}
}


/// Prints a simple list with entity names n stuff
void Entities::ListEntities()
{
	std::cout<<"\nList selection: ";
	for (int i = 0; i < currentItems; ++i)
	{
		std::cout<<"\n"<<i<<". ";
		if (arr[i]->name)
			std::cout<<arr[i]->name;
		std::cout<<" Pos: "<<arr[i]->worldPosition;
	}
}

void Entities::DeleteEntities(){
	/// Reply is valid, proceed with deletion
	for (int i = 0; i < currentItems; ++i){
		// Mark the entity as deleted
		MapMan.DeleteEntity(arr[i]);
	}
	/// Clear the selection
	Clear();
}

/// Selects next entity using given one as reference for the previous one.
Entities Entities::SelectNext(Entity * referenceEntity) const {
	/// If we had one selected, get next one.
	Entity * newOne = NULL;
	if (currentItems >= 1 && referenceEntity){
		Entity * previousOne = referenceEntity;
		for (int i = 0; i < currentItems; ++i){
			if (arr[i] == previousOne){
				if (i == currentItems - 1)
					newOne = arr[0];
				else
					newOne = arr[i+1];
				break;
			}
		}
		// If no new one found, means the reference is outside camera, so just select 0
		if (newOne == NULL)
			newOne = arr[0];
	}
	/// If not, just get first one
	else if (currentItems)
		newOne = arr[0];
	else
		return Entities();
	assert(newOne);
	return Entities(newOne);
}

/// Selects next entity using given one as reference for the previous one.
Entities Entities::SelectPrevious(Entity * referenceEntity) const {
	Entity * newOne = NULL;
	/// If we had one selected, get next one.
	if (currentItems >= 1 && referenceEntity){
		Entity * previousOne = referenceEntity;
		for (int i = currentItems - 1; i >= 0; --i){
			if (arr[i] == previousOne){
				if (i == 0)
					newOne = arr[currentItems-1];
				else
					newOne = arr[i-1];
				break;
			}
		}
		if (newOne == NULL)
			newOne = arr[currentItems-1];
	}
	/// If not, just get last one
	else if (currentItems)
		newOne = arr[currentItems-1];
	else
		return Entities();
	assert(newOne);
	return Entities(newOne);
}
