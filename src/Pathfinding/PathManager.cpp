/// Emil Hedemalm
/// 2013-03-01

#include "PathManager.h"
#include <cassert>
#include <map>
#include <cstring>
#include "WaypointManager.h"
#include <ctime>
#include "PathMessage.h"
#include "Message/MessageManager.h"
#include "Thread/Thread.h"

/// A manager for handling and calculating paths between various nodes provided by the waypoint-manager.
// class PathManager{

/// Function pointer for the actively bound search-function.
void (*searchFunction)(Waypoint * from, Waypoint * to, Path& path) = NULL;

/// Private constructor for singleton pattern
PathManager::PathManager(){
	searchFunction = &AStar;

	// Create lastPath Mutex
//	Mutex mutex;
//	mutex.Create("LastPathMutex");
}
PathManager * PathManager::pathManager = NULL;

//=============================================================================================//
// public:
//========================================================================================//
PathManager::~PathManager(){
}
/// Allocates the waypoint manager singleton
void PathManager::Allocate(){
	pathManager = new PathManager();
}
void PathManager::Deallocate(){
	assert(pathManager);
	delete pathManager;
	pathManager = NULL;
}

AE_THREAD_START(PathFinderThread)
	/// Check arguments.
	Waypoint * from = (Waypoint*) args[1], * to = (Waypoint*) args[2];
	Entity * entity = (Entity*) args[3];
	PathMessage * reply = new PathMessage(entity);
	AStar(from, to, reply->path);
	reply->path.Mirror();
	/// Queue reply via Message manager once we are done, since we are currently in another thread.
	MesMan.QueueMessage(reply);
AE_THREAD_END

/// In reality only accepts PathMessages, the rest are mostly ignored.
void PathManager::QueueMessage(PathMessage * pm)
{
//	messages.AddItem(pm);
	assert(pm->from);
	assert(pm->to);
	assert(pm->entity);

	if (pathfindingThreads.Size() > 20)
	{
		messageQueue.AddItem(pm);
		return;
	}

	/// Make a new thread.
	StartThreadFromMessage(pm);
	delete pm;
}

void PathManager::StartThreadFromMessage(PathMessage * pm)
{
	Thread * thread = new Thread(PathFinderThread);
	thread->AddArgument((Argument*)pm->from);
	thread->AddArgument((Argument*)pm->to);
	thread->AddArgument((Argument*)pm->entity);
	thread->Run();
	pathfindingThreads.AddItem(thread);
}

/// For processing the path searches. If 1 thread, iterates a bit, if multi-threaded approach, will mostly keep track of which threads have finished or not.
void PathManager::Process(int timeInMs)
{
	static int accum = 0;
	accum += timeInMs;
	if (accum > 1000)
		accum = accum % 1000;
	for (int i = 0; i < pathfindingThreads.Size(); ++i)
	{
		Thread * t = pathfindingThreads[i];
		if (t->HasEnded())
		{
			pathfindingThreads.RemoveItem(t);
			--i;
			if (messageQueue.Size())
			{
				PathMessage * pm = messageQueue[0];
				StartThreadFromMessage(pm);
				messageQueue.RemoveItem(pm);
				delete pm;
			}
		}
	}
}


/** Attempts to get control of the LastPath Mutex
	Make sure you call ReleaseLastPathMutex afterward!
	The maxWaitTime-parameter defines how long the function will wait before returning automatically.
*/
bool PathManager::GetLatsPathMutex(int maxWaitTime){
    /*
    Mutex mutex;
    mutex.Open("LastPathMutex");
    mutex.Claim(maxWaitTime);
    */
	return true;
}
/// Releases active NavMeshMutex.
bool PathManager::ReleaseLastPathMutex(){
    /*
	Mutex mutex;
    mutex.Open("LastPathMutex");
    mutex.Release();
    */
	return true;
}

/// Returns a copy of the last calculated path.
void PathManager::GetLastPath(Path& path){
	path = lastPath;
	return;
}

/// Calculates a path between target waypoint nodes.
Path PathManager::GetPath(Waypoint * from, Waypoint * to)
{
	GetLatsPathMutex();
	/// Get mutex for the active navmesh too
	WaypointMan.GetActiveNavMeshMutex();
	searchFunction(from, to, lastPath);
	Path returnPath = lastPath;
	returnPath.Mirror();
	WaypointMan.ReleaseActiveNavMeshMutex();
	ReleaseLastPathMutex();
	return returnPath;
}
/// Calculates a path between target waypoint nodes, storing the finished path in the given variable.
void PathManager::GetPath(Waypoint * from, Waypoint * to, Path &path)
{
	GetLatsPathMutex();
	WaypointMan.GetActiveNavMeshMutex();
	searchFunction(from, to, path);
	lastPath = path;
	path.Mirror();
	WaypointMan.ReleaseActiveNavMeshMutex();
	ReleaseLastPathMutex();
}

/// Sets search algorithm by name (must match exact function name for now)
void PathManager::SetSearchAlgorithm(const char * name)
{
	if (!name)
		return;
	if (strcmp(name, "AStar") == 0){
		searchFunction = &AStar;
	}
	else if (strcmp(name, "BreadthFirst") == 0){
		searchFunction = &BreadthFirst;
	}
	else if (strcmp(name, "DepthFirst") == 0){
		searchFunction = &DepthFirst;
	}
	else if (strcmp(name, "CustomAlgorithm") == 0){
		searchFunction = &CustomAlgorithm;
	}
}

/// Ew.
int PathManager::ThreadsActive()
{
	for (int i = 0; i < pathfindingThreads.Size(); ++i)
	{
		Thread * t = pathfindingThreads[i];
		if (t->HasEnded()){
			pathfindingThreads.RemoveItem(t);
			--i;
		}
	}
	return pathfindingThreads.Size();
}


//=============================================================================================//
// Search algorithms
//=============================================================================================//

float ManhattanDistance(Waypoint * from, Waypoint * to){
	return AbsoluteValue(from->position[0] - to->position[0]) +
		AbsoluteValue(from->position[1] - to->position[1]) +
		AbsoluteValue(from->position[2] - to->position[2]);
};

/** Calculates the given path using the A* algorithm, derived with guidance from Wikipedia
	http://en.wikipedia.org/wiki/A*_search_algorithm
*/
void AStar(Waypoint * from, Waypoint * to, Path& path)
{
	/// TODO: Add custom functions for seeking game-specific tiles. Or manipulate the navmesh in run-time somehow.
//	std::cout<<"\nBeginning A* path search...";
	Timer timer;
	timer.Start();
	path.Clear();

	/// Get active navmesh, as it should be what we're working with.
	NavMesh * nm = WaypointMan.ActiveNavMesh();
	assert(nm->WaypointPartOf(from));
	if (!nm->WaypointPartOf(to)){
		std::cout<<"\nWaypoint not part of NavMesh. No path can be generated!";
		return;
	}

	// The set of nodes already evaluated.
	const int SET_SIZE = 65536;
	Waypoint * closedSet[SET_SIZE];
	memset(closedSet, 0, SET_SIZE * sizeof(Waypoint*));
	/** The set of tentative nodes to be evaluated, initially containing the start node
		The open set is assumed to always be sorted by f_score.
	*/
	Waypoint * openSet[SET_SIZE];
	memset(openSet, 0, SET_SIZE * sizeof(Waypoint*));
	int itemsInOpenSet = 1;
	int itemsInClosedSet = 0;
	// Add start node to the open set
	openSet[0] = from;

	/// Map of nodes we came from, cost from start along best known path and estimated cost to goal
	assert(nm->waypoints.Size() < SET_SIZE);
	List<Waypoint*> & waypointList = nm->waypoints;
	Waypoint ** cameFrom = new Waypoint * [nm->waypoints.Size()];
	memset(cameFrom, 0, sizeof(Waypoint *) * nm->waypoints.Size());
	float * g_score = new float[nm->waypoints.Size()];
	float * f_score = new float[nm->waypoints.Size()];

	/// Set start node scores
	int startNodeIndex = nm->GetIndex(from);
	assert(startNodeIndex >= 0);
	g_score[startNodeIndex] = 0;
	f_score[startNodeIndex] = g_score[startNodeIndex] + ManhattanDistance(from, to);

	/// While we still have items in the open set to investigate...
	while (itemsInOpenSet > 0){
		/// Get the node in the open set with the lowest estimated total distance score.
		Waypoint * current = openSet[0];
		assert(current);
		float lowestFScore = f_score[nm->GetIndex(openSet[0])];
		/// Just grab the node with lowest f_score by force ..
		for (int i = 1; i < itemsInOpenSet; ++i){
			float fScore = f_score[nm->GetIndex(openSet[i])];
			if (fScore < lowestFScore){
				lowestFScore = fScore;
				current = openSet[i];
			}
		}
		int currentIndex = nm->GetIndex(current);
		/// If the current node is our goal, reconstruct our path.
		if (current == to) 
		{
		//	std::cout<<"\nGoal has been reached! Yays";
			path.AddItem(current);
			while (current != from){
				current = cameFrom[nm->GetIndex(current)];
				path.AddItem(current);
			}
			timer.Stop();
//			std::cout<<"\nA* Time taken: "<<timer.GetMicros()<<" microseconds, or "<<timer.GetMs()<<" ms.";
			return;
		}
		/// Remove the current from the open set
		for (int i = 0; i < itemsInOpenSet; ++i){
			/// First find the current one..
			if (openSet[i] != current)
				continue;
			/// Then move up the rest!
			for (int j = i; j < itemsInOpenSet; ++j)
				openSet[j] = openSet[j+1];
			break;
		}
		--itemsInOpenSet;
		/// Add the current node to the closedSet
		closedSet[itemsInClosedSet] = current;
		++itemsInClosedSet;

		/// Add all neighbours of the current node to the openSet
		for (int i = 0; i < current->neighbours; ++i){
			Waypoint * neighbour = current->neighbour[i];
			/// Check that it's passable, lol..
			if (!neighbour->passable)
				continue;
			bool skip = false;
			///...unless they've already been examined (are in the closed set)
			for (int j = 0; j < itemsInClosedSet; ++j){
				if (neighbour == closedSet[j]){
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			/// Get the neighbour's index in our map
			int index = nm->GetIndex(neighbour);
			float tentative_g_score = g_score[currentIndex] +
				(waypointList[index]->position - waypointList[currentIndex]->position).Length();

			/// Check if it already exists in the open set.
			bool existsInOpenSet = false;
			for (int j = 0; j < itemsInOpenSet; ++j){
				if (neighbour == openSet[j]){
					existsInOpenSet = true;
					break;
				}
			}
			// If it doesn't exist i nthe open set, just add it.
			if (!existsInOpenSet){
				/// Set where we came from, g_score and f_score
				cameFrom[index] = current;
				g_score[index] = g_score[currentIndex] +
					(waypointList[index]->position - waypointList[currentIndex]->position).Length();
				f_score[index] = g_score[index] + ManhattanDistance(neighbour, to);
				/// Add the waypoint to the openSet
				openSet[itemsInOpenSet] = neighbour;
				++itemsInOpenSet;
			}
			/// If it does exist, compare the new g_scores using the given paths.
			else if (tentative_g_score < g_score[index]){
				/// Set where we came from, g_score and f_score
				cameFrom[index] = current;
				g_score[index] = g_score[currentIndex] +
					(waypointList[index]->position - waypointList[currentIndex]->position).Length();
				f_score[index] = g_score[index] + ManhattanDistance(neighbour, to);
			}
		}
	}
	std::cout<<"\nA* Unable to find a suitable path :<";
	delete[] cameFrom;
	delete[] g_score;
	delete[] f_score;

	/// Return an empty path if we failed...
    return;
}

/// Calculates the given path using the brute force breadth-first algorithm
void BreadthFirst(Waypoint * from, Waypoint * to, Path& path){
	std::cout<<"\nBeginning Breadth First path search...";
	/*
	clock_t start = clock();
	path.Clear();

	/// Get active navmesh, as it should be what we're working with.
	NavMesh * nm = WaypointMan.ActiveNavMesh();
	assert(nm->WaypointPartOf(from));
	if (!nm->WaypointPartOf(to)){
		std::cout<<"\nWaypoint not part of NavMesh. No path can be generated!";
		return;
	}

	// The set of nodes already evaluated.
	const int SET_SIZE = 2048;
	Waypoint * closedSet[SET_SIZE];
	memset(closedSet, 0, SET_SIZE * sizeof(Waypoint*));
	// The set of tentative nodes to be evaluated, initially containing the start node
	//	The open set is assumed to always be sorted by f_score.

	Queue<Waypoint*> openSet;
	int itemsInOpenSet = 1;
	int itemsInClosedSet = 0;
	// Add start node to the open set
	openSet.Push(from);

	/// Map of nodes we came from, cost from start along best known path and estimated cost to goal
	assert(nm->waypoints < 2048);
	Waypoint ** waypointList = nm->waypoint;
	Waypoint ** cameFrom = new Waypoint * [nm->waypoints];
	memset(cameFrom, 0, sizeof(Waypoint *) * nm->waypoints);

	/// Set start node scores
	int startNodeIndex = nm->GetIndex(from);
	assert(startNodeIndex >= 0);

	/// While we still have items in the open set to investigate...
	while (openSet.Length() > 0){
		Waypoint * current = openSet.Pop();
		assert(current);
		int currentIndex = nm->GetIndex(current);
		/// If the current node is our goal, reconstruct our path.
		if (current == to) {
			std::cout<<"\nGoal has been reached! Yays";
			path.AddWaypoint(current);
			while (current != from){
				current = cameFrom[nm->GetIndex(current)];
				path.AddWaypoint(current);
			}
			clock_t stop = clock();
			clock_t duration = stop - start;
			std::cout<<"\nBreadthFirst Time taken: "<<duration<<" clock() ticks, or "<<duration / CLOCKS_PER_SEC<<" seconds.";
			return;
		}
		/// Add the current node to the closedSet
		closedSet[itemsInClosedSet] = current;
		++itemsInClosedSet;

		/// Add all neighbours of the current node to the openSet
		for (int i = 0; i < current->neighbours; ++i){
			Waypoint * neighbour = current->neighbour[i];
			/// Check that it's passable, lol..
			if (!neighbour->passable)
				continue;
			bool skip = false;
			///...unless they've already been examined (are in the closed set)
			for (int j = 0; j < itemsInClosedSet; ++j){
				if (neighbour == closedSet[j]){
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			/// Get the neighbour's index in our map
			int index = nm->GetIndex(neighbour);

			/// Check if it already exists in the open set.
			bool existsInOpenSet = openSet.Exists(neighbour);

			// If it doesn't exist i nthe open set, just add it.
			if (!existsInOpenSet){
				/// Set where we came from, g_score and f_score
				cameFrom[index] = current;
				/// Add the waypoint to the openSet
				openSet.Push(neighbour);
			}
		}
	}
	std::cout<<"\nBreadthFirst Unable to find a suitable path :<";
	/// Return an empty path if we failed...
	*/
    return;
}
/// Calculates the given path using the brute force depth-first algorithm
void DepthFirst(Waypoint * from, Waypoint * to, Path& path){
	std::cout<<"\nBeginning DepthFirst path search...";
	/*
	clock_t start = clock();
	path.Clear();

	/// Get active navmesh, as it should be what we're working with.
	NavMesh * nm = WaypointMan.ActiveNavMesh();
	assert(nm->WaypointPartOf(from));
	assert(nm->WaypointPartOf(to));

	// The set of nodes already evaluated.
	const int SET_SIZE = 2048;
	Waypoint * closedSet[SET_SIZE];
	memset(closedSet, 0, SET_SIZE * sizeof(Waypoint*));
	// The set of tentative nodes to be evaluated, initially containing the start node
	//	The open set is assumed to always be sorted by f_score.

	Queue<Waypoint*> openSet;
	int itemsInOpenSet = 1;
	int itemsInClosedSet = 0;
	// Add start node to the open set
	openSet.Push(from);

	/// Map of nodes we came from, cost from start along best known path and estimated cost to goal
	assert(nm->waypoints < 2048);
	Waypoint ** waypointList = nm->waypoint;
	Waypoint ** cameFrom = new Waypoint * [nm->waypoints];
	memset(cameFrom, 0, sizeof(Waypoint *) * nm->waypoints);

	/// Set start node scores
	int startNodeIndex = nm->GetIndex(from);
	assert(startNodeIndex >= 0);

	/// While we still have items in the open set to investigate...
	while (openSet.Length() > 0){
		Waypoint * current = openSet.PopLast();
		assert(current);
		int currentIndex = nm->GetIndex(current);
		/// If the current node is our goal, reconstruct our path.
		if (current == to) {
			std::cout<<"\nGoal has been reached! Yays";
			path.AddWaypoint(current);
			while (current != from){
				current = cameFrom[nm->GetIndex(current)];
				path.AddWaypoint(current);
			}
			clock_t stop = clock();
			clock_t duration = stop - start;
			std::cout<<"\nDepthFirst Time taken: "<<duration<<" clock() ticks, or "<<duration / CLOCKS_PER_SEC<<" seconds.";
			return;
		}
		/// Add the current node to the closedSet
		closedSet[itemsInClosedSet] = current;
		++itemsInClosedSet;

		/// Add all neighbours of the current node to the openSet
		for (int i = 0; i < current->neighbours; ++i){
			Waypoint * neighbour = current->neighbour[i];
			/// Check that it's passable, lol..
			if (!neighbour->passable)
				continue;
			bool skip = false;
			///...unless they've already been examined (are in the closed set)
			for (int j = 0; j < itemsInClosedSet; ++j){
				if (neighbour == closedSet[j]){
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			/// Get the neighbour's index in our map
			int index = nm->GetIndex(neighbour);

			/// Check if it already exists in the open set.
			bool existsInOpenSet = openSet.Exists(neighbour);

			// If it doesn't exist i nthe open set, just add it.
			if (!existsInOpenSet){
				/// Set where we came from, g_score and f_score
				cameFrom[index] = current;
				/// Add the waypoint to the openSet
				openSet.Push(neighbour);
			}
		}
	}
	std::cout<<"\nDepthFirst Unable to find a suitable path :<";
	/// Return an empty path if we failed...
	*/
    return;
}
/// A custom written algorithm for calculating a given path.
void CustomAlgorithm(Waypoint * from, Waypoint * to, Path& path){

	/// Tis algorithm tries to find the closest way to the destination, assuming no previous knowledge of the map is available.
	/// It will simulate a confused NPC or player ^^
	std::cout<<"\nBeginning CustomAlgorithm path search...";
	/*
	clock_t start = clock();
	path.Clear();

	/// Get active navmesh, as it should be what we're working with.
	NavMesh * nm = WaypointMan.ActiveNavMesh();
	assert(nm->WaypointPartOf(from));
	assert(nm->WaypointPartOf(to));

	// The set of nodes already evaluated.
	const int SET_SIZE = 2048;
	Waypoint * closedSet[SET_SIZE];
	memset(closedSet, 0, SET_SIZE * sizeof(Waypoint*));
	// The set of tentative nodes to be evaluated, initially containing the start node
	//	The open set is assumed to always be sorted by f_score.

	Waypoint * openSet[SET_SIZE];
	memset(openSet, 0, SET_SIZE * sizeof(Waypoint*));
	int itemsInOpenSet = 1;
	int itemsInClosedSet = 0;
	// Add start node to the open set
	openSet[0] = from;

	/// Map of nodes we came from, cost from start along best known path and estimated cost to goal
	assert(nm->waypoints < 2048);
	Waypoint ** waypointList = nm->waypoint;
	Waypoint ** cameFrom = new Waypoint * [nm->waypoints];
	memset(cameFrom, 0, sizeof(Waypoint *) * nm->waypoints);
	/// Total cost so far
	float * g_score = new float[nm->waypoints];
	/// Estimated total cost to goal, including total cost so far
	float * f_score = new float[nm->waypoints];

	/// Set start node scores
	int startNodeIndex = nm->GetIndex(from);
	assert(startNodeIndex >= 0);
	g_score[startNodeIndex] = 0;
	f_score[startNodeIndex] = g_score[startNodeIndex] + ManhattanDistance(from, to);

	/// While we still have items in the open set to investigate...
	while (itemsInOpenSet > 0){
		/// Get the node in the open set with the lowest estimated total distance score.
		Waypoint * current = openSet[0];
		assert(current);
		float lowestFScore = f_score[nm->GetIndex(openSet[0])];
		/// Just grab the node with lowest f_score by force ..
		for (int i = 1; i < itemsInOpenSet; ++i){
			float fScore = f_score[nm->GetIndex(openSet[i])];
			if (fScore > lowestFScore){
				lowestFScore = fScore;
				current = openSet[i];
			}
		}
		int currentIndex = nm->GetIndex(current);
		/// If the current node is our goal, reconstruct our path.
		if (current == to) {
			std::cout<<"\nGoal has been reached! Yays";
			path.AddWaypoint(current);
			while (current != from){
				current = cameFrom[nm->GetIndex(current)];
				path.AddWaypoint(current);
			}
			clock_t stop = clock();
			clock_t duration = stop - start;
			std::cout<<"\nCustom Time taken: "<<duration<<" clock() ticks, or "<<duration / CLOCKS_PER_SEC<<" seconds.";
			return;
		}
		/// Remove the current from the open set
		for (int i = 0; i < itemsInOpenSet; ++i){
			/// First find the current one..
			if (openSet[i] != current)
				continue;
			/// Then move up the rest!
			for (int j = i; j < itemsInOpenSet; ++j)
				openSet[j] = openSet[j+1];
			break;
		}
		--itemsInOpenSet;
		/// Add the current node to the closedSet
		closedSet[itemsInClosedSet] = current;
		++itemsInClosedSet;

		/// Add all neighbours of the current node to the openSet
		for (int i = 0; i < current->neighbours; ++i){
			Waypoint * neighbour = current->neighbour[i];
			/// Check that it's passable, lol..
			if (!neighbour->passable)
				continue;
			bool skip = false;
			///...unless they've already been examined (are in the closed set)
			for (int j = 0; j < itemsInClosedSet; ++j){
				if (neighbour == closedSet[j]){
					skip = true;
					break;
				}
			}
			if (skip)
				continue;

			/// Get the neighbour's index in our map
			int index = nm->GetIndex(neighbour);
			float tentative_g_score = g_score[currentIndex] +
				(waypointList[index]->position - waypointList[currentIndex]->position).Length();

			/// Check if it already exists in the open set.
			bool existsInOpenSet = false;
			for (int j = 0; j < itemsInOpenSet; ++j){
				if (neighbour == openSet[j]){
					existsInOpenSet = true;
					break;
				}
			}
			// If it doesn't exist i nthe open set, just add it.
			if (!existsInOpenSet){
				/// Set where we came from, g_score and f_score
				cameFrom[index] = current;
				g_score[index] = g_score[currentIndex] +
					(waypointList[index]->position - waypointList[currentIndex]->position).Length();
				f_score[index] = g_score[index] + ManhattanDistance(neighbour, to);
				/// Add the waypoint to the openSet
				openSet[itemsInOpenSet] = neighbour;
				++itemsInOpenSet;
			}
			/// If it does exist, compare the new g_scores using the given paths.
			else if (tentative_g_score < g_score[index]){
				/// Set where we came from, g_score and f_score
				cameFrom[index] = current;
				g_score[index] = g_score[currentIndex] +
					(waypointList[index]->position - waypointList[currentIndex]->position).Length();
				f_score[index] = g_score[index] + ManhattanDistance(neighbour, to);
			}
		}
	}
	std::cout<<"\nCustom Algorithm Unable to find a suitable path :<";
	/// Return an empty path if we failed...
	*/
    return;
}



