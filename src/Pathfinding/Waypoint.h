/// Emil Hedemalm
/// 2014-08-31
/// A waypoint for managing path-finding and entity locations in 2D/3D-space.

#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "List/List.h"
#include "../MathLib.h"
#include <fstream>

#define DEFAULT_GOAL	0x00000001
#define DEFAULT_START	0x00000002
#define CURRENT_GOAL	0x00000004
#define CURRENT_START	0x00000008
#define TYPE_AERIAL		0x00000010

class Entity;

/** A structure for handling navigational points throughout maps of various sorts.
*/
class Waypoint {
	friend class PathManager;
private:
	/// Copy constructor should not be allowed!
	Waypoint(const Waypoint &ref);
	Waypoint& operator = (const Waypoint &ref);
public:
	/// Default constructor
	Waypoint();
	~Waypoint();
	/// Center position of the waypoint
	Vector3f position;
	/// Distance from center of world. :)
	float elevation;
	/// Entity that's currently at this waypoint (mostly for debugging)
	List<Entity*> entities;
	/// Passability
	bool passable;
	/// Other data
	int data;
	/// Pointer to additional external data, depending on the demands of the waypoints.
	void * pData;
	
	/// Save/Load operations
	bool ReadFrom(std::fstream &f);
	bool WriteTo(std::fstream &f);
	/// Sets it as an aerial waypoint
	void SetAerial();
	/// Checks if it is an aerial waypoint.
	bool IsAerial();

	/// Returns true or false depending on statically set check-parameters, for example having the entity or pData pointers set can result in a false.
	bool IsVacant();

	/** Adds a neighbour to this waypoint. A check is done so that no waypoint is added twice.	
		Returns 0 if the it could not be added, 1 upon success and 2 if the neighbour was added earlier.
	*/
	int AddNeighbour(Waypoint * waypoint);
	/// Removes a waypoint from the neighbour array. Returns false if no such neighbour existed.
	bool RemoveNeighbour(Waypoint * waypoint);
	bool HasNeighbour(Waypoint* thisOne);
	/** Adds a child  to this waypoint. A check is done so that no waypoint is added twice.	
		Returns 0 if the it could not be added, 1 upon success and 2 if the child was added earlier.
	*/
	int AddChild(Waypoint * waypoint);
	/// Returns number of neighbours of this waypoint.
	int Neighbours(); 
	/// Returns neighbour at target index.
	Waypoint * Neighbour(int index);

	/** Flag if this node is a merge of several other waypoints. 
		Additional data may then be stored in the child-related variables. */
	bool merged;
	/// Parent node if this wp has taken part in a merge operation.
	Waypoint * parent;
	/// If this is an optimized waypoint, all children will be stored here!
	Waypoint ** child;
	int children;
	int maxChildren;
	static const int MAX_INITIAL_CHILDREN = 8;

	/// Shape of the waypoint, default is a Square
	void * shape;
	int physicsShape;
	/** The system assumes a 2d grid-layout until further notice, and thus maximum of 8 neighbours
		for any individual waypoint. However, optimization could be performed that adjust this further!
	*/
	static const int MAX_INITIAL_NEIGHBOURS = 8;
	/// Array of neighbour-pointers.
	Waypoint ** neighbour;
	/// IDs of neighbours, for when connecting them.
	int * neighbourIDs;
	/// Unique ID for this waypoint (in this navMesh).
	int id;
	/// Current amount of neighbours.
	int neighbours;
	/// Size of the neighbour arrays at the moment.
	int maxNeighbours;
};

#endif