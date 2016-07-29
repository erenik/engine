// Emil Hedemalm
// 2013-07-19

#ifndef PATH_H
#define PATH_H

#include "Waypoint.h"
#include <String/AEString.h>

/// A single path
class Path : public List<Waypoint*>
{
public:
	/// Default constructor
	Path();
	virtual ~Path();
	/// Copy constructors
	Path(const Path &path);
private:
	
public:
	/// Returns waypoint at specified index.
	Waypoint * GetClosest(const Vector3f & position) const;
	Waypoint * GetNext(const Waypoint * previousWaypoint);
	int GetIndex(const Waypoint * ofThisWaypoint) const;

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
	/// Defines if the last node should consider the first node as it's next waypoint.
	bool circular;
};

#endif