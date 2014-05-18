/// Emil Hedemalm
/// 2013-03-07

#include "Selection.h"
#include <cassert>

#include "Maps/MapManager.h"
#include "../Entity/Entity.h"
#include "Graphics/Camera/Camera.h"
#include <cstring>

Selection::Selection()
: List<Entity*>()
{

};

Selection::~Selection()
{
	
};

/// Copy constructor..!
Selection::Selection(const Selection& otherSelection) : List<Entity*> ((List<Entity*>&)otherSelection) {
	currentItems = otherSelection.currentItems;
	arrLength = currentItems;
	arr = new Entity * [currentItems];
	for (int i = 0; i < currentItems; ++i){
		arr[i] = otherSelection.arr[i];
	}
}
/// Inherit-Copy constructor
Selection::Selection(const List<Entity*> &entityList) : List<Entity*> ((List<Entity*>&)entityList){
	currentItems = entityList.Size();
	arrLength  = currentItems;
	arr = new Entity * [currentItems];
	for (int i = 0; i < currentItems; ++i){
		arr[i] = entityList[i];
	}
}

/// Removes all entities that are outside the frustum.
Selection Selection::CullByCamera(Camera * camera) const{
	Selection culled;
	int found = 0;
	for (int i = 0; i < this->currentItems; ++i){
		if (arr[i] == NULL)
			continue;
		if (camera->GetFrustum().SphereInFrustum(arr[i]->position, arr[i]->radius))
			culled.Add(arr[i]);
		++found;
		if (found >= currentItems)
			break;
	}
	return culled;
}

/// Sorts by distance to selected position.
void Selection::SortByDistance(Vector3f position) {
	float distance[MAX_SELECTED];
	memset(distance, 0, sizeof(float) * MAX_SELECTED);
	for (int i = 0; i < currentItems; ++i){
		/// Calculate distance
		distance[i] = (position - arr[i]->position).Length();
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

/// Prints a simple list with entity names n stuff
void Selection::ListEntities(){
	std::cout<<"\nList selection: ";
	for (int i = 0; i < currentItems; ++i){
		std::cout<<"\n"<<i<<". ";
		if (arr[i]->name)
			std::cout<<arr[i]->name;
		std::cout<<" Pos: "<<arr[i]->position;
	}
}

void Selection::DeleteEntities(){
	/// Reply is valid, proceed with deletion
	for (int i = 0; i < currentItems; ++i){
		// Mark the entity as deleted
		MapMan.DeleteEntity(arr[i]);
	}
	/// Clear the selection
	Clear();
}

/// Selects next entity using given one as reference for the previous one.
Selection Selection::SelectNext(Entity * referenceEntity) const {
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
		return Selection();
	assert(newOne);
	return Selection(newOne);
}

/// Selects next entity using given one as reference for the previous one.
Selection Selection::SelectPrevious(Entity * referenceEntity) const {
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
		return Selection();
	assert(newOne);
	return Selection(newOne);
}
