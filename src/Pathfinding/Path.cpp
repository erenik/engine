#include "Path.h"
#include <cassert>
#include "NavMesh.h"
#include "WaypointManager.h"
#include <cstring>

/// Default constructor
Path::Path(){
	waypoint = NULL;
	waypoints = 0;
	circular = false;
	arraySize = 0;
}
Path::~Path(){
	if (waypoint){
		if (waypoints > 0 && waypoint[0] == NULL){
			std::cout<<"\nWARNING: Waypoints in path are NULL for some reason.";
		}
		delete[] waypoint;
	}
	waypoint = NULL;
	waypoints = 0;
}
/// Copy constructor
Path::Path(const Path &path){
	waypoint = NULL;
	circular = path.circular;
	waypoints = path.waypoints;
	if (waypoints)
		waypoint = new Waypoint * [waypoints];
	for (int i = 0; i < waypoints; ++i){
		waypoint[i] = path.waypoint[i];
	}
	arraySize = waypoints;
}
Path::Path(const Path * path){
	waypoint = NULL;
	circular = false;
	waypoints = path->waypoints;
	if (waypoints)
		waypoint = new Waypoint * [waypoints];
	for (int i = 0; i < waypoints; ++i){
		waypoint[i] = path->waypoint[i];
	}
	arraySize = waypoints;
}

Path& Path::operator = (const Path &path){
	if (path.waypoints == 0){
		this->waypoints = 0;
		return *this;
	}
	if (arraySize > 0 && waypoint){
		delete[] waypoint;
		waypoint = NULL;
	}
	waypoints = path.waypoints;
	if (waypoints > 0)
		waypoint = new Waypoint * [waypoints];
	for (int i = 0; i < waypoints; ++i){
		waypoint[i] = path.waypoint[i];
	}
	arraySize = waypoints;
	return *this;
}

/// Returns waypoint at specified index.
Waypoint * Path::GetWaypoint(int i){
	if (arraySize == 0)
		return NULL;
//	if (circular){
		if (i >= arraySize)
			i -= arraySize;
		else if (i < 0)
			i += arraySize;
//	}
	assert(i >= 0 && i < arraySize && "Specified index not valid for given array in Path::Waypoint()");
	if (i < 0 || i >= arraySize)
		return NULL;
	return waypoint[i];
}

Waypoint * Path::GetClosest(Vector3f position) const {
	assert(waypoints);
	float lengthSq = 1000000000000.f,
		minLengthSq = 100000000000000.f;
	Waypoint * closest = NULL;
	for (int i = 0; i < waypoints; ++i){
		Waypoint * wp = waypoint[i];
		lengthSq = (wp->position - position).LengthSquared();
		if (lengthSq < minLengthSq){
			closest = wp;
			minLengthSq = lengthSq;
		}
	}
	return closest;
}
Waypoint * Path::GetNext(const Waypoint * previousWaypoint){
	for (int i = 0; i < waypoints; ++i){
		Waypoint * wp = waypoint[i];
		if (wp == previousWaypoint){
			if (i < waypoints -1)
				return waypoint[i+1];
			else
				return waypoint[0];
		}
	}
	return NULL;
}


int Path::GetIndex(const Waypoint * wp) const{
	for (int i = 0; i < waypoints; ++i){
		if (waypoint[i] == wp){
			return i;
		}
	}
	return -1;
}

/// Adds specified waypoint at the end of the path
void Path::AddWaypoint(Waypoint * newWaypoint){
	if (waypoints == arraySize){
		Resize(arraySize * 2);
	}
	waypoint[waypoints] = newWaypoint;
	++waypoints;
}

/// Resizes the array length.
void Path::Resize(int newSize){
	if (newSize == 0)
		newSize = 8;
	Waypoint ** newArray = new Waypoint * [newSize];
	for (int i = 0; i < waypoints; ++i){
		newArray[i] = waypoint[i];
	}
	if (waypoint){
		delete[] waypoint;
		waypoint = NULL;
	}
	waypoint = newArray;
	arraySize = newSize;
}

/// Clears all entries.
void Path::Clear(){
	for (int i = 0; i < arraySize; ++i){
		waypoint[i] = NULL;
	}
	waypoints = 0;
}

/// Mirrors the path, this since most algorithms build it up in reverse...
void Path::Mirror(){
	for (int i = 0; i < waypoints * 0.5f; ++i){
		Waypoint * tmp = waypoint[i];
		int otherI = waypoints - i-1;
		waypoint[i] = waypoint[otherI];
		waypoint[otherI] = tmp;
		std::cout<<"\nSwapping waypoint "<<i<<" with "<<otherI<<": "<<waypoint[i]->position<<" <-> "<<waypoint[otherI]->position;
	}
}

#define PATH_VERSION_1		0x00000001 // Path name, waypoints positions, and if the path is circular

#define PATH_VERSION PATH_VERSION_1

/// Wosh.
bool Path::Save(String toFile) const{
	std::fstream file;
	file.open(toFile.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<toFile<<" in Path::Save(toFile)";
		return false;
	}
	bool result = WriteTo(file);
	assert(result);
	file.close();
	return true;
}


/// Wosh.
bool Path::Load(String fromFile){
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<fromFile<<" in Path::Load(fromFile)";
		return false;
	}
	bool result = ReadFrom(file);
	assert(result);
	file.close();
	return true;
}

/// For reading/writing to stream.
bool Path::ReadFrom(std::fstream & file){
	bool result;
	int version = PATH_VERSION;
	/// Version!
	file.read((char*)&version, sizeof(int));
	// Name!
	result = name.ReadFrom(file); // <- Use String::s built-in file-reader :)
	if (!result)
		return false;
	// Circularity!
	file.read((char*)&circular, sizeof(bool)); // <- Should definitely make int and bit-wise flag for this shit...
	/// Waypoints !
	int newWaypoints;
	file.read((char*)&newWaypoints, sizeof(int));
	assert(newWaypoints < 50000 && newWaypoints >= 0);
	if (newWaypoints > 50000 || newWaypoints < 0){
		std::cout<<"\nERROR: Bad data... "<<newWaypoints<<" new waypoints.. seems wrong, yes?";
		return false;
	}
	Resize(newWaypoints);
	/// Read waypoint data!
	NavMesh * nm = WaypointMan.ActiveNavMesh();
	//	assert(nm);
	if (nm == NULL){
		nm = WaypointMan.CreateNavMesh("Temp path Navmesh");
	}
	if (nm == NULL){
		std::cout<<"\nERROR: No nav mesh selected and creation failed. Aborting. :/";
		return NULL;
	}
	for (int i = 0; i < newWaypoints; ++i){
		Vector3f position;
		file.read((char*)&position, sizeof(Vector3f));
		Waypoint * wp = nm->GetClosestTo(position, 10.0f);
		if (wp){
			waypoint[i] = wp;
		}
		else {
	//		std::cout<<"\nINFO: Unable to find waypoint in navmesh within given range, creating and adding a new waypoint.";
			waypoint[i] = new Waypoint();
			waypoint[i]->position = position;
			nm->AddWaypoint(waypoint[i]);
		}
	}
	waypoints = newWaypoints;
	return true;
}
bool Path::WriteTo(std::fstream & file) const{
	int version = PATH_VERSION;
	/// Version!
	file.write((char*)&version, sizeof(int));
	// Name!
	name.WriteTo(file); // <- Use String::s built-in file-reader :)
	// Circularity!
	file.write((char*)&circular, sizeof(bool)); // <- Should definitely make int and bit-wise flag for this shit...
	/// Waypoints !
	file.write((char*)&waypoints, sizeof(int));
	/// Write waypoint data.
	for (int i = 0; i < waypoints; ++i){
		file.write((char*)&waypoint[i]->position, sizeof(Vector3f));
	}
	return true;
}

/// Adds addend to this vector.
void Path::operator += (const Path otherPath){
	if (otherPath.arraySize == 0)
		return;
	Waypoint ** newArray = new Waypoint * [waypoints + otherPath.waypoints];
	int newNumWaypoints = 0;
	memset(newArray,0,sizeof(Waypoint**));
	for (int i = 0; i < waypoints; ++i){
		newArray[i] = waypoint[i];
		++newNumWaypoints;
	}
	if (waypoint)
		delete waypoint;
	waypoint = newArray;
	waypoints = newNumWaypoints;

	/// Check if the first waypoints in OtherPath occurs in this path!
	bool firstOneExistsInLastPath = false;
	for (int i = 0; i < waypoints; ++i){
		if (waypoint[i] == otherPath.waypoint[0])
			firstOneExistsInLastPath = true;
	}
	/// If so, skip it and add le others.
	for (int i = 0; i < otherPath.waypoints; ++i){
		if (i == 0 && firstOneExistsInLastPath)
			continue;
		waypoint[waypoints] = otherPath.waypoint[i];
		++waypoints;
	}
}


// Debug
void Path::Print(){
	std::cout<<"\nPath with "<<waypoints<<" waypoints:";
	for (int i = 0; i < waypoints; ++i)
		std::cout<<"\n- "<<i<<": "<<GetWaypoint(i)->position;
}