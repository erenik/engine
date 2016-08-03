/// Emil Hedemalm
/// 2013-03-01

#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include "Path.h"

class PathMessage;
class Thread;

#define PathMan		(*PathManager::Instance())

/// A manager for handling and calculating paths between various nodes provided by the waypoint-manager.
class PathManager 
{
	/// Private constructor for singleton pattern
	PathManager();
	static PathManager * pathManager;
public:
	~PathManager();
	/// Allocates the waypoint manager singleton
	static void Allocate();
	static void Deallocate();
	/// Get singleton instance
	static inline PathManager * Instance() { return pathManager; };

	/// In reality only accepts PathMessages, the rest are mostly ignored.
	void QueueMessage(PathMessage * pm);
	void StartThreadFromMessage(PathMessage * pm);

	/// For processing the path searches. If 1 thread, iterates a bit, if multi-threaded approach, will mostly keep track of which threads have finished or not.
	void Process(int timeInMs);

	/** Attempts to get control of the LastPath Mutex
		Make sure you call ReleaseLastPathMutex afterward!
		The maxWaitTime-parameter defines how long the function will wait before returning automatically.
	*/
	static bool GetLatsPathMutex(int maxWaitTime = 1000);
	/// Releases active NavMeshMutex.
	static bool ReleaseLastPathMutex();
	/// Returns a copy of the last calculated path.
	void GetLastPath(Path& path);

	/// Calculates a path between target waypoint nodes.
	Path GetPath(Waypoint * from, Waypoint * to);
	/// Calculates a path between target waypoint nodes, storing the finished path in the given variable.
	void GetPath(Waypoint * from, Waypoint * to, Path &path);

	/// Sets search algorithm by name (must match exact function name for now)
	void SetSearchAlgorithm(const char * name);

	/// Ew.
	int ThreadsActive();
	/// IF false, no.
	bool acceptRequests;
private:
	/// Last calculated path.
	Path lastPath;
	List<Thread*> pathfindingThreads;
	List<PathMessage*> messageQueue;
};


/// Calculates the given path using the A* algorithm
void AStar(Waypoint * from, Waypoint * to, Path& path);
/// Calculates the given path using the brute force breadth-first algorithm
void BreadthFirst(Waypoint * from, Waypoint * to, Path& path);
/// Calculates the given path using the brute force depth-first algorithm
void DepthFirst(Waypoint * from, Waypoint * to, Path& path);
/// A custom written algorithm for calculating a given path.
void CustomAlgorithm(Waypoint * from, Waypoint * to, Path& path);


#endif