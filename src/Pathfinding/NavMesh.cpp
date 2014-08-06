// Author: Emil Hedemalm
// Date: 2013-02-27

#include "NavMesh.h"
#include <cstring>
#include <ctime>

#include "PhysicsLib/Shapes/Ray.h"

/// An organization of waypoints that are interconnected somehow, like a map.
NavMesh::NavMesh(){
	walkables = 0;
	original2DMap = NULL;
	rows = columns = 0;
	/// Default start and goal for testing the navmesh quickly.
	defaultStart = NULL;
	defaultGoal = NULL;
	waypointArraySize = 0;
	optimized = false;
	idCounter = 0;
}
NavMesh::~NavMesh(){
	CLEAR_AND_DELETE(waypoints);

	/// Deallocate map data array
	if (original2DMap){
		for (int x = 0; x < columns; ++x){
			delete[] original2DMap[x];
			original2DMap[x] = NULL;
		}
		delete[] original2DMap;
		original2DMap = NULL;
	}
}

/// Deletes all waypoints.
void NavMesh::Clear(){
	CLEAR_AND_DELETE(waypoints);
}

/// Loads from target navmesh file (specific to this system)
bool NavMesh::LoadFromFile(const char * filename){
	assert(waypoints.Size() == 0 && "Trying to load in an already filled navmesh, is this the intent?");
	if (waypoints.Size() != 0)
		return false;
	time_t startTime = clock();
	std::fstream file;
	file.open(filename, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open()){
		std::cout<<"\nUnable to open stream to file: "<<filename;
		return false;
	}
	int waypointsToLoad;
	/// First the amount of waypoints, obviously!
	file.read((char*)&waypointsToLoad, sizeof(int));
	bool result;
	std::cout<<"\nWaypoints specified in file: "<<waypointsToLoad;
	std::cout<<"\nBegin loading each waypoint...";
	for (int i = 0; i < waypointsToLoad; ++i){
		if (i%1000 == 0)
			std::cout<<"\n"<<i<<" out of "<<waypoints.Size()<<" loaded.";
		Waypoint * wp = new Waypoint();
		result = wp->ReadFrom(file);
		assert(result && "Unable to read waypoint!");
		AddWaypoint(wp);
	}
	file.close();

	assert(waypointsToLoad == waypoints.Size() && "Number of loaded waypoints does not equal the amount specified in the file!");
	/// Rebind each waypoint's neighbour after loading!
	std::cout<<"\nRe-binding waypoints to their respective neighbours.";
	for (int i = 0; i < waypoints.Size(); ++i){
		if (i%1000 == 0)
			std::cout<<"\n"<<i<<" out of "<<waypoints.Size()<<" processed.";
		Waypoint * wp = waypoints[i];
		for (int n = 0; n < wp->neighbours; ++n){
			assert(n < wp->maxNeighbours && "Neighbour array's not big enough, schtupid.");
			int soughtID = wp->neighbourIDs[n];
			assert(soughtID != -1 && "NeighbourID is -1 for some reason!");
			/// Find each neighbour!
			Waypoint * wp2 = GetWaypointById(soughtID);
			assert(wp2 && "Unable to find neighbour with soughtID!");
			wp->neighbour[n] = wp2;
			assert(wp->neighbour[n] && "Unable to find neighbour with specified id!");
		}
	}
	CalculateDefaults();
	clock_t stopTime = clock();
	std::cout<<"\nNavMesh loaded successfully into \""<<filename<<"\"  Time taken: "<<stopTime - startTime;
	source = filename;
	return true;
}
/// Saves to target navmesh file (specific to this system)
bool NavMesh::SaveToFile(const char * filename) const {
	assert(waypoints.Size() != 0 && "Trying to save empty navmesh, is this the intent?");
	std::fstream file;
	file.open(filename, std::ios_base::out | std::ios_base::binary);
	if (!file.is_open()){
		std::cout<<"\nUnable to open stream to file: "<<filename;
		return false;
	}
	/// First the amount of waypoints, obviously!
	int numWps = waypoints.Size();
	file.write((char*)&numWps, sizeof(int));
	bool result;
	/// Then save each waypoint by itself o-o
	for (int i = 0; i < waypoints.Size(); ++i){
		assert(i < waypoints.Size());
		for (int n = 0; n < waypoints[i]->neighbours; ++n){
			int soughtID = waypoints[i]->neighbour[n]->id;
			assert(GetWaypointById(soughtID) && "Waypoint has neighbours that are unaccessible!");
		}
		result = waypoints[i]->WriteTo(file);
		assert(result && "Unable to write waypoint to file!");
	}
	file.close();
	std::cout<<"\nNavMesh saved successfully into \""<<filename<<"\"";
	return true;
}

/// Maps the defualt start, goal, and other relevant nodes.
void NavMesh::CalculateDefaults(){
	for (int i = 0; i < waypoints.Size(); ++i){
		Waypoint * wp = waypoints[i];
		if (wp->data & DEFAULT_START){
			defaultStart = wp;
			std::cout<<"\nStart node found!";
		}
		if (wp->data & DEFAULT_GOAL){
			defaultGoal = wp;
			std::cout<<"\nGoal node found!";
		}
	}
}


/// Adds a waypoint to the NavMesh.
int NavMesh::AddWaypoint(Waypoint * wp)
{
	assert(wp);
	/// Assign it a new ID if it hasn't already been given one (id == -1)
	if (wp->id == -1){
		for (int i = 0; i < waypoints.Size(); ++i){
			if (waypoints[i]->id > idCounter)
				idCounter = waypoints[i]->id + 1;
		}
		wp->id = idCounter++;
	}
	waypoints.Add(wp);
	return 0;
}

/// Removes a waypoint from the NavMesh, but does not clean up any remaining bindings.
int NavMesh::RemoveWaypoint(Waypoint * wp){
	waypoints.Remove(wp);
	return 1;
}

/// Cleans up and removes all neighbour references to this waypoint.
int NavMesh::CleanupNeighbours(Waypoint * wp){
	assert(wp != NULL);
	int neighboursRemoved = 0;
	while(wp->neighbours > 0){
		wp->neighbour[0]->RemoveNeighbour(wp);
		wp->RemoveNeighbour(wp->neighbour[0]);
		++neighboursRemoved;
	}
	return neighboursRemoved;
}

/// Returns a pointer to specified Waypoint, or NULL if it does not exist.
Waypoint * NavMesh::GetWaypointById(int id) const {
	for (int i = 0; i < waypoints.Size(); ++i){
		if (waypoints[i]->id == id)
			return waypoints[i];
		else if (waypoints[i]->id == -1){
			assert(false && "Waypoint without valid ID found in NavMesh::GetWaypointByID!");
		}
	}
//	assert(false && "Waypoint not found! Algorithms should not get here.");
	return NULL;
}

/// Checks if this waypoint exists in this navMesh.
bool NavMesh::WaypointPartOf(Waypoint * wp) const {
	Waypoint * w;
	for (int i = 0; i < waypoints.Size(); ++i){
		w = waypoints[i];
		if (waypoints[i] == wp)
			return true;
	}
	return false;
}
/// Gets current index of the waypoint in the array.
int NavMesh::GetIndex(Waypoint * wp){
	for (int i = 0; i < waypoints.Size(); ++i){
		if (waypoints[i] == wp)
			return i;
	}
	return -1;
}

/** Re-load all waypoints from the original 2D reference map. */
int NavMesh::ReloadFromOriginal(){
	/// Notify the AIManager that all current paths will be more or less invalidated!
//	AI.PathsInvalidated();

	 /*
	if (waypoint){
		std::cout<<"\nNavMesh::ReloadFromOriginal: NavMesh allocated already. Deleting it first.";
		delete[] waypoint;
	}
	waypointArraySize = rows * columns;
	waypoint = new Waypoint * [waypointArraySize];
	walkables = 0;
	for (int y = 0; y < rows; ++y){
		for (int x = 0; x < columns; ++x){
			waypoints[y * columns + x] = new Waypoint();
			/// New waypoint
			Waypoint * nw = waypoints[y * columns + x];
			Waypoint * mapWaypoint = this->original2DMap[x][y];
			/// Transfer data to new waypoint from old one, not including neighbour-information (yet!)
			/// The ID is what is most important for connecting them later!
			nw->id = y * columns + x;
			nw->passable = mapWaypoint->passable;
			if (nw->passable)
				++walkables;
			nw->data = mapWaypoint->data;
			nw->position = mapWaypoint->position;
		}
	}
	waypoints = rows * columns;

	for (int y = 0; y < rows; ++y){
		for (int x = 0; x < columns; ++x){
			Waypoint * wp = waypoints[y * columns + x];
			/// Check for the default start and end nodes.
			if (wp->data & DEFAULT_START)
				this->defaultStart = wp;
			if (wp->data & DEFAULT_GOAL)
				this->defaultGoal = wp;

			/// Assign neighbours for all nodes.
			if (y > 0){
				wp->AddNeighbour(GetWaypointById(wp->id - columns));
				if (x > 0)
					wp->AddNeighbour(GetWaypointById(wp->id - columns - 1));
				if (x < columns - 1)
					wp->AddNeighbour(GetWaypointById(wp->id - columns + 1));
			}
			if (y < rows - 1){
				wp->AddNeighbour(GetWaypointById(wp->id + columns));
				if (x > 0)
					wp->AddNeighbour(GetWaypointById(wp->id + columns - 1));
				if (x < columns - 1)
					wp->AddNeighbour(GetWaypointById(wp->id + columns + 1));
			}
			if (x > 0)
				wp->AddNeighbour(GetWaypointById(wp->id - 1));
			if (x < columns - 1)
				wp->AddNeighbour(GetWaypointById(wp->id + 1));
		}
	}
	std::cout<<"\nNavMesh "<<this->source<<" reloaded.";
	optimized = false;

	*/
	return 0;
}

/** Optimizes the navmesh, using the original 2D-map as reference.
	Returns amount of merges that were performed.
*/
int NavMesh::Optimize(){
	/*
	/// Begin by re-loading from the original 2D-map
	this->ReloadFromOriginal();

	/// Sets to merge
	int MAX_MERGE_SETS = waypoints;
	Waypoint *** toMerge = new Waypoint ** [MAX_MERGE_SETS];
	for (int i = 0; i < waypoints.Size(); ++i){
		toMerge[i] = new Waypoint * [MAX_MERGE_SETS];
	}
	int * setSize = new int[MAX_MERGE_SETS];
	memset(setSize, 0, sizeof(int) * waypoints);
	int setsDone = 0;

	/// Then begin merging waypoints
	for (int i = 0; i < waypoints.Size(); ++i){
		/// Skip unpassables
		if (!waypoints[i]->passable)
			continue;
		/// Skip those already added to the previous arrays to decrease merge-attempts.
		bool skip = false;
		for (int j = 0; j < setsDone; ++j){
			for (int k = 0; k < setSize[j]; ++k){
				if (toMerge[j][k] == waypoints[i]){
					j = setsDone;
					skip = true;
					break;
				}
			}
		}
		if (skip)
			continue;

		bool ableToExpand = true;
		bool expandedLeft = true;
		bool expandedRight = true;
		bool expandedTop = true;
		bool expandedBottom = true;

// Macro for ID of a tile given it's coordinates
#define TileID(tileX, tileY) ((tileX) + (tileY) * columns)
#define TileX(tileID) (tileID % columns)
#define TileY(tileID) (tileID / columns)
		int x1, x2, y1, y2;
		int tileID = waypoints[i]->id;
		x1 = x2 = TileX(tileID);
		y1 = y2 = TileY(tileID);
		/// Begin by adding the initial node.
		int addedToSetSoFar = 0;
		toMerge[setsDone][addedToSetSoFar] = waypoints[i];
		++addedToSetSoFar;
		/// Then try expanding from it as base.
		while (ableToExpand){
			ableToExpand = false;
			/// Check left
			if (expandedLeft){
				if (x1 > 0){
					bool expandableLeft = true;
					for (int y = y1; y <= y2; ++y){
						/// Skip unpassables, as well as those already in the above arrays
						Waypoint * t = waypoints[TileID(x1 - 1, y)];
						if (!t->passable){
							expandableLeft = false;
							break;
						}
						/// Skip those already added to the previous arrays to decrease merge-attempts.
						bool skip = false;
						for (int j = 0; j < setsDone; ++j){
							for (int k = 0; k < setSize[j]; ++k){
								if (toMerge[j][k] == t){
									j = setsDone;
									skip = true;
									break;
								}
							}
						}
						if (skip)
							expandableLeft = false;
					}
					if (expandableLeft){
						for (int y = y1; y <= y2; ++y){
							toMerge[setsDone][addedToSetSoFar] = waypoints[TileID(x1 - 1, y)];
							++addedToSetSoFar;
						}
						x1 -= 1;
						expandedLeft = true;
					}
					else
						expandedLeft = false;
				}
				else
					expandedLeft = false;
			}
			/// Check right
			if (expandedRight){
				if (x2 < columns - 1){
					bool expandableRight = true;
					for (int y = y1; y <= y2; ++y){
						/// Skip unpassables, as well as those already in the above arrays
						Waypoint * t = waypoints[TileID(x2 + 1, y)];
						if (!t->passable){
							expandableRight = false;
							break;
						}
						/// Skip those already added to the previous arrays to decrease merge-attempts.
						bool skip = false;
						for (int j = 0; j < setsDone; ++j){
							for (int k = 0; k < setSize[j]; ++k){
								if (toMerge[j][k] == t){
									j = setsDone;
									skip = true;
									break;
								}
							}
						}
						if (skip)
							expandableRight = false;
					}
					if (expandableRight){
						for (int y = y1; y <= y2; ++y){
							Waypoint * t = waypoints[TileID(x2 + 1, y)];
							toMerge[setsDone][addedToSetSoFar] = t;
							++addedToSetSoFar;
						}
						x2 += 1;
						expandedRight = true;
					}
					else
						expandedRight = false;
				}
				else
					expandedRight = false;
			}
			/// Check top
			if (expandedTop){
				if (y1 > 0){
					bool expandableTop = true;
					for (int x = x1; x <= x2; ++x){
						/// Skip unpassables, as well as those already in the above arrays
						int id = TileID(x, y1 - 1);
						Waypoint * t = waypoints[id];
						if (!t->passable){
							expandableTop = false;
							break;
						}
						/// Skip those already added to the previous arrays to decrease merge-attempts.
						bool skip = false;
						for (int j = 0; j < setsDone; ++j){
							for (int k = 0; k < setSize[j]; ++k){
								if (toMerge[j][k] == t){
									j = setsDone;
									skip = true;
									break;
								}
							}
						}
						if (skip)
							expandableTop = false;
					}
					if (expandableTop){
						for (int x = x1; x <= x2; ++x){
							Waypoint * t = waypoints[TileID(x, y1 - 1)];
							toMerge[setsDone][addedToSetSoFar] = t;
							++addedToSetSoFar;
						}
						y1 -= 1;
						expandedTop = true;
					}
					else
						expandedTop = false;
				}
				else
					expandedTop = false;
			}
			/// Check bottom
			if (expandedBottom){
				if (y2 < rows - 1){
					bool expandableBottom = true;
					for (int x = x1; x <= x2; ++x){
						/// Skip unpassables, as well as those already in the above arrays
						int id = TileID(x, y2 + 1);
						Waypoint * t = waypoints[id];
						if (!t->passable){
							expandableBottom = false;
							break;
						}
						/// Skip those already added to the previous arrays to decrease merge-attempts.
						bool skip = false;
						for (int j = 0; j < setsDone; ++j){
							for (int k = 0; k < setSize[j]; ++k){
								if (toMerge[j][k] == t){
									j = setsDone;
									skip = true;
									break;
								}
							}
						}
						if (skip)
							expandableBottom = false;
					}
					if (expandableBottom){
						for (int x = x1; x <= x2; ++x){
							Waypoint * t = waypoints[TileID(x, y2 + 1)];
							toMerge[setsDone][addedToSetSoFar] = t;
							++addedToSetSoFar;
						}
						y2 += 1;
						expandedBottom = true;
					}
					else
						expandedBottom = false;
				}
				else
					expandedBottom = false;
			}

			/// Check if we managed to expand this iteration
			if (expandedLeft || expandedRight || expandedTop || expandedBottom)
				ableToExpand = true;
		} /// Expandation loop.
		/// Consider the set done only if we had more than 1 wp added to the set!
		if (addedToSetSoFar > 1){
			setSize[setsDone] = addedToSetSoFar;
			++setsDone;
		}
	}

	/// Register all merges
	int merges = 0;
	for (int i = 0; i < setsDone; ++i){
		if (MergeWaypoints(toMerge[i], setSize[i]))
			merges++;
	}

	/// Deallocate the allocated arrays
	for (int i = 0; i < MAX_MERGE_SETS; ++i){
		delete[] toMerge[i];
	}
	delete[] toMerge;
	delete[] setSize;

	if (merges > 0)
		optimized = true;

	std::cout<<"\nNavMesh "<<source<<" optimized with "<<merges<<" merges.";
	return merges;
	*/
	return 0;
}

/// Returns amount of discarded waypoints. Automatically re-routes/discards neighbours pointers.
int NavMesh::DiscardUnwalkables(){
	/*
	int removals = 0;
	List<Waypoint*> toRemove;
	for (int i = 0; i < waypoints.Size(); ++i){
		if(!waypoints[i]->passable){
			CleanupNeighbours(waypoints[i]);
			toRemove.Add(waypoints[i]);
			removals++;
		}
	}
	for (int i = 0; i < toRemove.Size(); ++i)
		RemoveWaypoint(toRemove[i]);
	std::cout<<"\nRemoved "<<removals<<" unwalkables.";
	return removals;
	*/
	return 0;
}
/// For scaling le navmesh (i.e.: all waypoints)
void NavMesh::Scale(float scale)
{
	for (int i = 0; i < waypoints.Size(); ++i){
		waypoints[i]->position *= scale;
		waypoints[i]->elevation *= scale;
	}
}

/// Attempts to merge all walkable waypoints depending on their distance to each other.
int NavMesh::MergeWaypointsByProximity(float maxDistance)
{
	int waypointsBeforeMerge = waypoints.Size();
	int walkablesBeforeMerge = walkables;
	std::cout<<"\nMerging Waypoints by Proximity, maxDistance: "<<maxDistance;
	/// Sets to merge
	int MAX_MERGE_SETS = waypoints.Size();
	Waypoint *** toMerge = new Waypoint ** [MAX_MERGE_SETS];
	for (int i = 0; i < waypoints.Size(); ++i){
		toMerge[i] = new Waypoint * [MAX_MERGE_SETS];
	}
	int * setSize = new int[MAX_MERGE_SETS];
	memset(setSize, 0, sizeof(int) * waypoints.Size());
	int setsDone = 0;
	std::cout<<"\nGathering data for merges...";
	/// Go through every waypoint
	for (int i = 0; i < waypoints.Size(); ++i){
		if (i%100 == 0)
			std::cout<<"\n"<<i<<" out of "<<waypoints.Size()<<" processed.";
		/// Skip unpassables
		if (!waypoints[i]->passable)
			continue;
		/// Begin by adding the initial node.
		int addedToSetSoFar = 0;
		toMerge[setsDone][addedToSetSoFar] = waypoints[i];
		++addedToSetSoFar;
		/// Don't have to go through all preceding waypoints...
		for (int j = i + 1; j < waypoints.Size(); ++j){
			/// check that they are passable, skip the unpassables, yes?
			if (!waypoints[j]->passable)
				continue;
			/// check distance
			if ((waypoints[i]->position - waypoints[j]->position).Length() < maxDistance){
				toMerge[setsDone][addedToSetSoFar] = waypoints[j];
				++addedToSetSoFar;
			}
		}
		/// Check that we added any nodes the last loop
		if (addedToSetSoFar > 1){
			setSize[setsDone] = addedToSetSoFar;
			++setsDone;
		}
	}
	std::cout<<"\nRegistering merges...";
	/// Register all merges
	int merges = 0;
	for (int i = 0; i < setsDone; ++i){
		if (i % 100 == 0)
			std::cout<<"\n"<<i<<" out of "<<setsDone<<" merges processed.";
		if (MergeWaypoints(toMerge[i], setSize[i]))
			merges++;
	}

	/// Deallocate the allocated arrays
	for (int i = 0; i < MAX_MERGE_SETS; ++i){
		delete[] toMerge[i];
	}
	delete[] toMerge;
	delete[] setSize;

	if (merges > 0)
		optimized = true;

	std::cout<<"\nNavMesh "<<source<<" optimized with "<<merges<<" merges.";
	std::cout<<"\nAmount of waypoints decreased from "<<waypointsBeforeMerge<<" to "<<waypoints.Size();
	std::cout<<"\nAmount of walkables decreased from "<<walkablesBeforeMerge<<" to "<<walkables;
	return merges;
}

/** Optimization function for merging a select number of waypoints, averaging the position,
	and re-directing all neighbours' pointers to the new merged waypoint.
*/
bool NavMesh::MergeWaypoints(Waypoint ** waypointList, int waypointsToMerge)
{
	if (waypointList == NULL || waypointsToMerge < 2)
		return false;

	int WAYPOINTS_BEFORE_MERGE = waypoints.Size();

	/// Check that none of the waypoints have already been merged.
	for (int i = 0; i < waypointsToMerge; ++i){
		if (waypointList[i]->child || waypointList[i]->merged || waypointList[i]->parent){
		//	std::cout<<"\nOne or more waypoints are already merged ones. Skipping this merge.";
			return false;
		}
	}

	/// Remove the merging waypoints from the list
	for (int j = 0; j < waypointsToMerge; ++j){
		RemoveWaypoint(waypointList[j]);
	}

	/// Create/Add a new merged waypoint
	Waypoint * wp = new Waypoint();
	Vector3f averagePosition;
	int data = 0;
	for (int i = 0; i < waypointsToMerge; ++i){
		averagePosition += waypointList[i]->position;
		data |= waypointList[i]->data;
		waypointList[i]->parent = wp;	// Set parent for all merged nodes!
		wp->AddChild(waypointList[i]);
	}
	averagePosition = averagePosition / waypointsToMerge;
	wp->position = averagePosition;
	wp->data = data;
	if (wp->data & DEFAULT_GOAL)
		this->defaultGoal = wp;
	if (wp->data & DEFAULT_START)
		this->defaultStart = wp;
	AddWaypoint(wp);

	Waypoint ** neighbour = new Waypoint * [WAYPOINTS_BEFORE_MERGE];
	memset(neighbour, 0, sizeof(Waypoint*) * WAYPOINTS_BEFORE_MERGE);
	int neighbours = 0;

	/// Find all references to the previous waypoints and replace them with this new one.
	for (int i = 0; i < waypoints.Size(); ++i){
		bool removedAny = false;
		for (int j = 0; j < waypointsToMerge; ++j){
			if (waypoints[i]->RemoveNeighbour(waypointList[j]))
				removedAny = true;
		}
		if (removedAny){
			waypoints[i]->AddNeighbour(wp);
			neighbour[neighbours] = waypoints[i];
			neighbours++;
		}
	}

	///  Keep track of all Waypoints that referenced it and add them as neighbours to this new merged waypoint
	for (int i = 0; i < neighbours; ++i)
		wp->AddNeighbour(neighbour[i]);

	/// Flag it as merged.
	wp->merged = true;
	walkables -= waypointsToMerge - 1;

	/// Check stuff
	for (int i = 0; i < wp->neighbours; ++i){
		int neighbourID = -1;
		if (neighbour)
			neighbourID = neighbour[i]->id;
		/// NOTE: NeighbourID and neighbour[i]->id are not fully synchronized!
		/// NeighbourID can be old ;___; needs fixin' (maybe, sometime XD)
		assert(neighbourID != wp->id && "Why the fuck would you add yourself as your neighbour? Paranoid waypoint...");
	}
	return true;
}

/// Selection..
Waypoint * NavMesh::GetClosestToRay(Ray & ray)
{
	float closestDot = 0.0f;
	Waypoint * closest = NULL;
	float dotProd = 0.0f;
	Vector3f dir = ray.direction.NormalizedCopy();
	for (int i = 0; i < waypoints.Size(); ++i){
		Waypoint * wp = waypoints[i];
		Vector3f rayStartToWp = wp->position - ray.start;
		rayStartToWp.Normalize();
		dotProd = rayStartToWp.DotProduct(dir);
		if (dotProd > closestDot){
			closest = wp;
			closestDot = dotProd;
		}
	}
	return closest;
}

/// If maxDistance is positive, it will limit the search within that vicinity-range. A negative number will set no limit.
Waypoint * NavMesh::GetClosestTo(Vector3f position, float maxDistance /* = -1.f */){
	float closestDist = 100000000000000.0f;
	Waypoint * closest = NULL;
	float sqMaxLen = maxDistance * maxDistance;
	for (int i = 0; i < waypoints.Size(); ++i){
		Waypoint * wp = waypoints[i];
		float dist = (wp->position - position).LengthSquared();
		if ((maxDistance < 0 && dist > sqMaxLen) ||
			(maxDistance > 0 && dist > sqMaxLen))
			continue;
		if (dist < closestDist){
			closest = wp;
			closestDist = dist;
		}
	}
	return closest;
}
/// Returns vacant waypoint closest to target position
Waypoint * NavMesh::GetClosestVacantWaypoint(Vector3f position)
{
	float closestDist = 100000000000000.0f;
	Waypoint * closest = NULL;
	for (int i = 0; i < waypoints.Size(); ++i){
		Waypoint * wp = waypoints[i];
		if (!wp->IsVacant())
			continue;
	//	std::cout<<"\n cppp";
		float dist = (wp->position - position).LengthSquared();
		if (dist < closestDist){
			closest = wp;
			closestDist = dist;
		}
	}
	return closest;	
}


/// Creates neighbour-connections automatically, using only a maximum distance. Returns number of connections made.
int NavMesh::ConnectWaypointsByProximity(float maxDistance){
	float squareDist = maxDistance * maxDistance;
	int connectionsMade = 0;
	Waypoint * wp1, * wp2;
	for (int i = 0; i < waypoints.Size(); ++i){
		wp1 = waypoints[i];
		for (int j = i+1; j < waypoints.Size(); ++j){
			wp2 = waypoints[j];
			if ((wp1->position - wp2->position).LengthSquared() < squareDist){
				wp1->AddNeighbour(wp2);
				wp2->AddNeighbour(wp1);
				connectionsMade++;
			}
		}
	}
	return connectionsMade;
}
/// Ensures that each waypoint has atLeast neighbours.
void NavMesh::EnsureNeighbours(int atLeast){
	std::cout<<"\nNavMesh::EnsureNeighbours "<<atLeast<<" called...";
	Waypoint * wp1, *wp2;
	for (int i = 0; i < waypoints.Size(); ++i){
		wp1 = waypoints[i];
		/// Only deal with those still lacking neighbours.
		while(wp1->neighbours < atLeast){
			float minDistance = 100000000000000000000.0f;
			float dist;
			Waypoint * closest = NULL;
			for (int j = 0; j < waypoints.Size(); ++j){
				wp2 = waypoints[j];
				if (wp2 == wp1)
					continue;
				if (wp1->HasNeighbour(wp2))
					continue;
				dist = (wp1->position - wp2->position).LengthSquared();
				if (dist < minDistance){
					minDistance = dist;
					closest = wp2;
				}
			}
			assert(closest);
			if (closest == NULL){
				std::cout<<"\nNavMesh::EnsureNeighbours: Too few waypoints in navmesh! Cannot fulfil minimum neighbour requirement.";
				break;
			}
			wp1->AddNeighbour(closest);
			closest->AddNeighbour(wp1);
		}
	}
}

/// Performs OnSelection functions, like nullifying pData.
void NavMesh::OnSelect(){
	for (int i = 0; i < waypoints.Size(); ++i){
		waypoints[i]->pData = NULL;
	}
}
