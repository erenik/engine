#include "Waypoint.h"
#include "../PhysicsLib.h"
#include <cassert>

Waypoint::Waypoint(){
	merged = false;
	neighbour = NULL;
	neighbourIDs = NULL;
	maxNeighbours = 0;
	neighbours = 0;
	passable = true;
	physicsShape = ShapeType::PLANE;
	shape = new Plane();
	
	parent = NULL;
	child = NULL;
	children = 0;
	maxChildren = 0;
	data = 0;
	pData = NULL;
	id = -1;

	entity = NULL;
};

Waypoint::~Waypoint(){
	if (shape)
		delete (Plane*)shape;
	if (child)
		delete[] child;
	delete[] neighbour;
}

/// Save/Load operations
bool Waypoint::ReadFrom(std::fstream &f){
	/// Unique ID for this waypoint (in this navMesh).
	f.read((char*)&id, sizeof(int));
	f.read((char*)&neighbours, sizeof(int));
	if (neighbours > 10)
		;//std::cout<<"\nNeighbours "<<neighbours;
	/// Allocate neighbour array
	assert(maxNeighbours == 0);
	neighbour = new Waypoint * [neighbours];
	neighbourIDs = new int [neighbours];
	maxNeighbours = neighbours;
	for (int i = 0; i < maxNeighbours; ++i){
		neighbour[i] = NULL;
		neighbourIDs[i] = -1;
	}

	for (int i = 0; i < neighbours; ++i){
		f.read((char*)&neighbourIDs[i], sizeof(int));
		assert(neighbourIDs[i] != id && "Why the fuck would you add yourself as your neighbour? Paranoid waypoint...");
	//	std::cout<<" "<<neighbourIDs[i];
		assert(neighbourIDs[i] >= 0 && "Neighbour ID negative! Is this the intent?");
		assert(neighbourIDs[i] != -1 && "Neighbour ID is negative! Extracting ID from neighbour pointer if possible");
	}
	int floatsize = sizeof(float);
	int vectorSize = sizeof(Vector3f);
	f.read((char*)&position, sizeof(Vector3f));
	f.read((char*)&elevation, sizeof(float));
	f.read((char*)&passable, sizeof(bool));
	f.read((char*)&data, sizeof(int));

	/*
	int id;
	int neighbours;
	/// IDs of neighbours, for when connecting them.
	int * neighbourIDs;
	/// Center position of the waypoint
	Vector3f position;
	float elevation;
	bool passable;
	int data;
	void * pData;

/* /// Ignored for now!
	bool merged;
	Waypoint * parent;
	Waypoint ** child;
	int children;
	int maxChildren;
	static const int MAX_INITIAL_CHILDREN = 8;
	/// Shape of the waypoint, default is a Square
	void * shape;
	int physicsShape;

*/
	return true;
}
bool Waypoint::WriteTo(std::fstream &f){
	assert(id != -1 && "ID is -1, will be unable to re-bind neighbours later!");
	f.write((char*)&id, sizeof(int));
	if (id == -1)
		return false;
	f.write((char*)&neighbours, sizeof(int));
	if (neighbours > 10)
		std::cout<<"\nNeighbours "<<neighbours;
	for (int i = 0; i < neighbours; ++i){
		int neighbourID = -1;
		if (neighbour)
			neighbourID = neighbour[i]->id;
		/// NOTE: NeighbourID and neighbour[i]->id are not fully synchronized! 
		/// NeighbourID can be old ;___; needs fixin' (maybe, sometime XD)
		assert(neighbourID != id && "Why the fuck would you add yourself as your neighbour? Paranoid waypoint...");
		assert(neighbourID == neighbour[i]->id);
		assert(neighbourID >= 0 && "Neighbour ID negative! Is this the intent?");
		assert(neighbourID != -1 && "Neighbour ID is negative! Extracting ID from neighbour pointer if possible");
		f.write((char*)&neighbourID, sizeof(int));
	}
	f.write((char*)&position, sizeof(Vector3f));
	f.write((char*)&elevation, sizeof(float));
	f.write((char*)&passable, sizeof(bool));
	f.write((char*)&data, sizeof(int));
	return true;
}

/// Sets it as an aerial waypoint
void Waypoint::SetAerial(){
	data |= TYPE_AERIAL;
}
/// Checks if it is an aerial waypoint.
bool Waypoint::IsAerial(){
	return data & TYPE_AERIAL;
}

/// Returns true or false depending on statically set check-parameters, for example having the entity or pData pointers set can result in a false.
bool Waypoint::IsVacant(){
//	std::cout<<"\nWaypoint passable: "<<passable<<" enitity:" <<entity<<" pData:"<<pData;
	if (!passable)
		return false;
	if (entity)
		return false;
//	if (pData)
//		return false;
	return true;
}

/** Adds a neighbour to this waypoint. A check is done so that no waypoint is added twice.	
	Returns 0 if the it could not be added, 1 upon success and 2 if the neighbour was added earlier.
*/
int Waypoint::AddNeighbour(Waypoint * newWaypoint){
	assert(newWaypoint != this && "Trying to add self as neighbour...");
	if (newWaypoint == this)
		return -1;
	for (int i = 0; i < maxNeighbours; ++i){
		if (neighbour[i] == newWaypoint)
			return 2;
	}
	int index = -1;
	for (int i = 0; i < maxNeighbours; ++i){
		if (neighbour[i] == NULL){
			index = i;
			break;
		}
	}
	/// Reize neighbour-array !
	if (index == -1){
		int newSize = maxNeighbours * 2;
		if (newSize <= 0)
			newSize = 4;
		Waypoint ** newList = new Waypoint * [newSize];
		for (int i = 0; i < newSize; ++i){
			if (i < maxNeighbours)
				newList[i] = neighbour[i];
			else
				newList[i] = NULL;
		}
		index = maxNeighbours;
		maxNeighbours = newSize;
		delete[] neighbour;
		neighbour = newList;
	}
	neighbour[index] = newWaypoint;
	++neighbours;
	return 1;
}
/// Removes a waypoint from the neighbour array. Returns false if no such neighbour existed.
bool Waypoint::RemoveNeighbour(Waypoint * wp){
	for (int i = 0; i < maxNeighbours; ++i){
		if (neighbour[i] == wp){
			--neighbours;
			neighbour[i] = neighbour[neighbours];
			neighbour[neighbours] = NULL;
			return true;
		}
	}
	return false;
}
bool Waypoint::HasNeighbour(Waypoint* thisOne){
	for (int i = 0; i < maxNeighbours; ++i){
		if (neighbour[i] == thisOne){
			return true;
		}
	}
	return false;
}


/** Adds a child  to this waypoint. A check is done so that no waypoint is added twice.	
	Returns 0 if the it could not be added, 1 upon success and 2 if the child was added earlier.
*/
int Waypoint::AddChild(Waypoint * newWaypoint){
	int index = -1;
	if (child){
		for (int i = 0; i < maxChildren; ++i){
			if (child[i] == newWaypoint)
				return 2;
		}
		for (int i = 0; i < maxChildren; ++i){
			if (child[i] == NULL){
				index = i;
				break;
			}
		}
	}
	/// Reize neighbour-array !
	if (index == -1){
		int newSize = maxChildren * 2;
		if (newSize <= 0)
			newSize = MAX_INITIAL_CHILDREN;
		Waypoint ** newList = new Waypoint * [newSize];
		for (int i = 0; i < newSize; ++i){
			if (i < maxChildren)
				newList[i] = child[i];
			else
				newList[i] = NULL;
		}
		index = maxChildren;
		maxChildren = newSize;
		if (child)
			delete[] child;
		child = newList;
	}
	child[index] = newWaypoint;
	++children;
	return 1;
}

/// Returns number of neighbours of this waypoint.
int Waypoint::Neighbours(){
	return neighbours;
}
/// Returns neighbour at target index.
Waypoint * Waypoint::Neighbour(int index){
	assert(index >= 0 && index < neighbours);
	return neighbour[index];
}
