#include "Path.h"
#include <cassert>
#include "NavMesh.h"
#include "WaypointManager.h"
#include <cstring>

/// Default constructor
Path::Path()
: List<Waypoint*>()
{
	circular = false;
}

Path::~Path()
{
}

/// Copy constructor
Path::Path(const Path &path)
{
	circular = path.circular;
	this->Add(path);
}

Waypoint * Path::GetClosest(const Vector3f & position) const 
{
	float lengthSq = 1000000000000.f,
		minLengthSq = 100000000000000.f;
	Waypoint * closest = NULL;
	for (int i = 0; i < this->currentItems; ++i){
		Waypoint * wp = arr[i];
		lengthSq = (wp->position - position).LengthSquared();
		if (lengthSq < minLengthSq){
			closest = wp;
			minLengthSq = lengthSq;
		}
	}
	return closest;
}

Waypoint * Path::GetNext(const Waypoint * previousWaypoint)
{
	for (int i = 0; i < currentItems; ++i)
	{
		Waypoint * wp = arr[i];
		if (wp == previousWaypoint)
		{
			return arr[(i+1) % currentItems];
		}
	}
	return NULL;
}


int Path::GetIndex(const Waypoint * wp) const
{
	for (int i = 0; i < currentItems; ++i){
		if (arr[i] == wp){
			return i;
		}
	}
	return -1;
}

/// Mirrors the path, this since most algorithms build it up in reverse...
void Path::Mirror()
{
	for (int i = 0; i < currentItems * 0.5f; ++i){
		Waypoint * tmp = arr[i];
		int otherI = currentItems - i-1;
		arr[i] = arr[otherI];
		arr[otherI] = tmp;
//		std::cout<<"\nSwapping waypoint "<<i<<" with "<<otherI<<": "<<arr[i]->position<<" <-> "<<arr[otherI]->position;
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

#include "List/ListUtil.h"

/// For reading/writing to stream.
bool Path::ReadFrom(std::fstream & file)
{
	std::cout<<"fix it";
	/*
	int version = PATH_VERSION;
	/// Version!
	file.read((char*)&version, sizeof(int));
	// Name!
	result = name.ReadFrom(file); // <- Use String::s built-in file-reader :)
	if (!result)
		return false;

	ReadListFrom<Waypoint*>(*this, file);

//	WriteListTo<Waypoint*>(*this, file);

	bool result;
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
	*/
	return true;
}
bool Path::WriteTo(std::fstream & file) const
{
	std::cout<<"fix it";
	/*
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
	}*/
	return true;
}

/// Adds addend to this vector.
void Path::operator += (const Path otherPath)
{
	this->Add(otherPath);
}


// Debug
void Path::Print()
{
	std::cout<<"\nPath with "<<currentItems<<" waypoints:";
	for (int i = 0; i < currentItems; ++i)
		std::cout<<"\n- "<<i<<": "<<arr[i]->position;
}