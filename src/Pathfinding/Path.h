// Emil Hedemalm
// 2013-07-19

#ifndef PATH_H
#define PATH_H

#include "Waypoint.h"
#include <String/AEString.h>

/// A single path
class Path {
public:
	/// Default constructor
	Path();
	~Path();
	/// Copy constructors
	Path(const Path &path);
	/// Assignment operator
	Path& operator = (const Path &path);
private:
	Path(const Path * path);
	
public:
	
	/// Returns number of active waypoints in the path
	int Waypoints() { return waypoints; };
	/// Returns waypoint at specified index.
	Waypoint * GetWaypoint(int i);
	Waypoint * GetClosest(const Vector3f & position) const;
	Waypoint * GetNext(const Waypoint * previousWaypoint);
	int GetIndex(const Waypoint * ofThisWaypoint) const;
	/// Adds specified waypoint at the end of the path
	void AddWaypoint(Waypoint * waypoint);
	/// Clears all entries.
	void Clear();

	/// Mirrors the path, this since most algorithms build it up in reverse... 
	void Mirror();
	/// Adds addend to this vector.
	void operator += (const Path otherPath);

	/// Public save/load functions for individual files.
	bool Save(String toFile) const;
	bool Load(String fromFile);

	/// For reading/writing to stream.
	bool ReadFrom(std::fstream & file);
	bool WriteTo(std::fstream & file) const;

	// Debug
	void Print();

	/// Identifier for this path.
	String name;
private:
	/// Resizes the array length.
	void Resize(int newSize);


	

	/// Size of the waypoint-array.
	int arraySize;
	/// The array of waypoints that the Path consists of.
	Waypoint ** waypoint;
	/// Number of waypoints in the path
	int waypoints;
	/// Defines if the last node should consider the first node as it's next waypoint.
	bool circular;
};

#endif