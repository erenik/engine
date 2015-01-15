/// Emil Hedemalm
/// 2013-03-01
#include "WaypointManager.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <cstring>
#include "Maps/Map.h"
#include "Entity/Entity.h"
#include "Mesh/Mesh.h"
#include "Model/ModelManager.h"

float WaypointManager::minimumWaypointProximity = 10.0f;
int WaypointManager::minimumNeighbours = 4;
float WaypointManager::minimumInclination = 0.8f;

/// Private constructor for singleton pattern
WaypointManager::WaypointManager(){
	activeNavMesh = NULL;
	activeNavMeshMutex.Create("ActiveNavMeshMutex");
}
WaypointManager * WaypointManager::waypointManager = NULL;

WaypointManager::~WaypointManager(){
	CLEAR_AND_DELETE(navMeshList);
	activeNavMeshMutex.Destroy();
}

/// Performs initial calculations (like loading default waypoint-maps for visualization/testing purposes..)
void WaypointManager::Initialize(){
}

/// Allocates the waypoint manager singleton
void WaypointManager::Allocate(){
	assert(!waypointManager);
	waypointManager = new WaypointManager();
}
void WaypointManager::Deallocate(){
	assert(waypointManager);
	delete waypointManager;
	waypointManager = NULL;
}

/// Create a new navmesh!
NavMesh * WaypointManager::CreateNavMesh(String name){
	for (int i = 0; i < navMeshList.Size(); ++i){
		NavMesh * n = navMeshList[i];
		if (n->name == name){
			assert( false && "Navmesh already exists! select it instead yaow");
			std::cout<<"\nERROR: Navmesh by name "<<name<<" already exists. Select it instead!";
			return NULL;
		}
	}
	NavMesh * nm = new NavMesh();
	nm->name = name;
	navMeshList.Add(nm);
	activeNavMesh = nm;
	return nm;
}

/// Fetch by name
NavMesh * WaypointManager::GetNavMeshByName(String name){
	for (int i = 0; i < navMeshList.Size(); ++i){
		NavMesh * n = navMeshList[i];
		if (n->name == name)
			return n;
	}
	return NULL;
}

/// For when auto-generating,
void WaypointManager::SetMinimumWaypointProximity(float minDist){
	minimumWaypointProximity = minDist;
}
void WaypointManager::SetMinimumNeighbours(int neighbours){
	minimumNeighbours = neighbours;
}
void WaypointManager::SetMinimumInclination(float inclination){
	minimumInclination = inclination;
}

/// Loads navmesh from file into a free NavMesh slot
NavMesh * WaypointManager::LoadNavMesh(const char * filename){
	GetActiveNavMeshMutex();
	NavMesh * existsAlready = GetNavMesh(filename);
	if (existsAlready){
		activeNavMesh = existsAlready;
		ReleaseActiveNavMeshMutex();
		return activeNavMesh;
	}
	activeNavMesh = CreateNavMesh(filename);
	String path(filename);
	if (!(path.Contains("navmesh/") || path.Contains("nm/"))){
		path = "navmesh/" + path;
	}
	bool result = activeNavMesh->LoadFromFile(path);
	ReleaseActiveNavMeshMutex();
	if (result)
		return activeNavMesh;
	return NULL;
}
/// Saves active navmesh to file
bool WaypointManager::SaveNavMesh(const char * filename){
	char path[MAX_PATH];
	strcpy(path, "navmesh/");
	strcat(path, filename);
	return activeNavMesh->SaveToFile(path);
}


/// Returns a reference to it :)
NavMesh * WaypointManager::GenerateNavMesh(Map * fromMap){
	GetActiveNavMeshMutex();
	// Do the generation for the active map.
	assert(activeNavMesh);

	List<Entity*> entities = fromMap->GetEntities();
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];		
		GenerateWaypointsFromEntity(entity);
	}
	/// Merge 'em
	activeNavMesh->MergeWaypointsByProximity(minimumWaypointProximity);
	/// Bind 'em.
//	activeNavMesh->ConnectWaypointsByProximity(50.0f);
	activeNavMesh->EnsureNeighbours(minimumNeighbours);
	ReleaseActiveNavMeshMutex();
	return activeNavMesh;
}

NavMesh * WaypointManager::GenerateNavMesh(List<Entity*> fromEntities)
{
	GetActiveNavMeshMutex();
	// Do the generation for the active map.
	assert(activeNavMesh);

	List<Entity*> entities = fromEntities;
	for (int i = 0; i < entities.Size(); ++i){
		Entity * entity = entities[i];		
		GenerateWaypointsFromEntity(entity);
	}
	/// Merge 'em
	activeNavMesh->MergeWaypointsByProximity(minimumWaypointProximity);
	/// Bind 'em.
//	activeNavMesh->ConnectWaypointsByProximity(50.0f);
	activeNavMesh->EnsureNeighbours(minimumNeighbours);
	ReleaseActiveNavMeshMutex();
	return activeNavMesh;	
}

/// generates and stores new waypoints for this entity into the active navmesh.
void WaypointManager::GenerateWaypointsFromEntity(Entity * entity)
{
	/*
	Model * model = entity->model;
	if (model == NULL)
		return;
	// For every face, create a waypoint
	Mesh * mesh = model->mesh;
	if (mesh == NULL)
		return;
	for (int j = 0; j < mesh->faces.Size(); ++j)
	{
		/// Get average position of all vertices in face.
		MeshFace * face = &mesh->faces[j];
		Vector3f position;
		for (int v = 0; v < face->numVertices; ++v){
			position += mesh->vertices[face->vertices[v]];
		}
		position /= face->numVertices;
		position = entity->transformationMatrix.Product(position);
		Vector3f normals = mesh->normals[face->normals[0]];
		normals = entity->rotationMatrix.Product(normals);

		if (normals.DotProduct(Vector3f(0,1,0)) > minimumInclination){
			Waypoint * newWp = new Waypoint();
			newWp->position = position;
			newWp->pData = (void*)&mesh->face[j];
			activeNavMesh->AddWaypoint(newWp);
		}
	}
	*/
}

/// Attempts to load a spherical world from target entity's model file.
void WaypointManager::GenerateNavMeshFromWorld(Entity * worldEntity)
{

	/*
	assert(worldEntity && "NullEntity provided in WaypointManager::GenerateNavMeshFromWorld");
	// Get source file
	assert(worldEntity->model && worldEntity->model->mesh->source && "Lacking data to make navMesh from probably");
	// Open it again...
	Model * model = ModelMan.LoadObj(worldEntity->model->mesh->source);

	/// Get mutex now..
	this->GetActiveNavMeshMutex();

	activeNavMesh = CreateNavMesh(worldEntity->name + " navmesh");

	// For every face, create a waypoint
	Mesh * mesh = model->mesh;
	for (int i = 0; i < mesh->faces; ++i){
		Waypoint * newWp = new Waypoint();
		/// Get average position of all vertices in face.
		Vector3f position;
		for (int v = 0; v < mesh->face[i].numVertices; ++v){
			position += mesh->vertices[mesh->face[i].vertices[v]];
		}
		position /= mesh->face[i].numVertices;
		newWp->position = position;
		newWp->pData = (void*)&mesh->face[i];
		activeNavMesh->AddWaypoint(newWp);
	}

	/// For every waypoint..
	std::cout<<"\nBegin binding neighbours...";
	for (int i = 0; i < activeNavMesh->waypoints.Size(); ++i){
		Waypoint * wp = activeNavMesh->waypoints[i];
		MeshFace * face = (MeshFace *) wp->pData;

		if (i % 100 == 0){
			std::cout<<"\n"<<i<<" out of "<<activeNavMesh->waypoints<<" processed.";
		}
		/// Check with every other waypoint.
		for (int j = i+1; j < activeNavMesh->waypoints.Size(); ++j){

			/// And check every vertices with each other in here.
			for (int k = 0; k < face->numVertices; ++k){

				Waypoint * wp2 = activeNavMesh->waypoints[j];
				MeshFace * f2 = (MeshFace *) wp2->pData;
				///.. No I meant check with every other waypoint's VERTICES!
				for (int l = 0; l < f2->numVertices; ++l){
					Vector3f * v, *v2;
					v = &mesh->vertices[face->vertices[k]];
					v2 = &mesh->vertices[f2->vertices[l]];
					if (v == v2){
						/// THIS IS NEIGHBOUR :D
						wp->AddNeighbour(wp2);
						wp2->AddNeighbour(wp);
						break;
					}
				}
			}
		}
	}

	activeNavMesh->walkables = activeNavMesh->waypoints;
	// Mark unpassables by comparing their normals with their position (since it should be relative to 0,0,0)
	for (int i = 0; i < activeNavMesh->waypoints; ++i){
		Waypoint * wp = activeNavMesh->waypoints[i];
		MeshFace * f = (MeshFace *) wp->pData;
		/// Just compare one normals for now..
		float normalDotPosition = mesh->normals[f->normals[0]].DotProduct(wp->position.NormalizedCopy());
		if (normalDotPosition < 0.5f){
			wp->passable = false;
			--activeNavMesh->walkables;
		}
		/// Calculate their elevation too o-o;
		wp->elevation = wp->position.Length();
		if (wp->elevation > 4250.0f){
		//	std::cout<<"\nElevation: "<<wp->elevation;
			wp->passable = false;
			--activeNavMesh->walkables;
		}
	}

	float distanceProximity = 150.0f;
//	activeNavMesh->MergeWaypointsByProximity(distanceProximity);

	// Scale it up slightly vor visualization...
	for (int i = 0; i < activeNavMesh->waypoints; ++i)
		activeNavMesh->waypoints[i]->position *= 1.01f;

	/// Get max point now, scale it up and create waypoints for aerial entities (le birdie nam-nam o-o)
	float maxElevation = activeNavMesh->waypoints[0]->elevation;
	for (int i = 0; i < activeNavMesh->waypoints; ++i){
		if (activeNavMesh->waypoints[i]->elevation > maxElevation)
			maxElevation = activeNavMesh->waypoints[i]->elevation;
	}
	maxElevation *= 1.07f;

	/// Generate sphere-waypoints for our new elevation!
	model = ModelMan.GetModel("sphere.obj");
	Mesh * sphere = model->mesh;
	for (int i = 0; i < sphere->faces; ++i){
		Waypoint * newWp = new Waypoint();
		newWp->SetAerial();
		/// Get average position of all vertices in face.
		Vector3f position;
		for (int v = 0; v < sphere->face[i].numVertices; ++v){
			position += sphere->vertices[sphere->face[i].vertices[v]];
		}
		position /= sphere->face[i].numVertices;
		newWp->position = position;
		newWp->pData = (void*)&sphere->face[i];
		activeNavMesh->AddWaypoint(newWp);
	}

	/// Bind neighbours for close-by fl
	/// For every waypoint..
	std::cout<<"\nBegin binding neighbours for aerial sphere...";
	for (int i = 0; i < activeNavMesh->waypoints; ++i){
		Waypoint * wp = activeNavMesh->waypoints[i];
		if (!wp->IsAerial())
			continue;
		MeshFace * face = (MeshFace *) wp->pData;

		if (i % 100 == 0){
			std::cout<<"\n"<<i<<" out of "<<activeNavMesh->waypoints<<" processed.";
		}
		/// Check with every other waypoint.
		for (int j = i+1; j < activeNavMesh->waypoints; ++j){
			Waypoint * wp2 = activeNavMesh->waypoints[j];
			if (!wp2->IsAerial())
				continue;
			/// And check every vertices with each other in here.
			for (int k = 0; k < face->numVertices; ++k){

				MeshFace * f2 = (MeshFace *) wp2->pData;
				///.. No I meant check with every other waypoint's VERTICES!
				for (int l = 0; l < f2->numVertices; ++l){
					Vector3f * v, *v2;
					v = &mesh->vertices[face->vertices[k]];
					v2 = &mesh->vertices[f2->vertices[l]];
					if (v == v2){
						/// THIS IS NEIGHBOUR :D
						wp->AddNeighbour(wp2);
						wp2->AddNeighbour(wp);
						break;
					}
				}
			}
		}
	}

	/// Move out all aerial waypoints now to their appropriate elevation
	for (int i = 0; i < activeNavMesh->waypoints; ++i){
		if (!activeNavMesh->waypoints[i]->IsAerial())
			continue;
		activeNavMesh->waypoints[i]->position.Normalize();
		activeNavMesh->waypoints[i]->position *= maxElevation;
	}

	// Release mutex..
	this->ReleaseActiveNavMeshMutex();
}

/// Nullifies the pData varible of every active waypoint.
int WaypointManager::CleansePData(){
	int nullified = 0;
	for (int i = 0; i < activeNavMesh->waypoints; ++i){
		if (activeNavMesh->waypoints[i]->pData)
			++nullified;
		activeNavMesh->waypoints[i]->pData = NULL;
	}
	return nullified;
	*/
}

/** Attempts to toggle walkability on the waypoint closest to the provided position both in the current array
	as well as the arrays from which the NavMesh is based upon.
*/
bool WaypointManager::ToggleWaypointWalkability(Vector3f position){
   /* Mutex mutex;
    mutex.Open("ActiveNavMeshMutex");
    */
	assert(this->activeNavMesh->original2DMap && "WaypointManager::ToggleWaypointWalkability");
	Waypoint * closestWaypoint = this->activeNavMesh->original2DMap[0][0];
	float distance = (closestWaypoint->position - position).Length();
	float newDist;
	for (int y = 0; y < this->activeNavMesh->rows; ++y){
		for (int x = 0; x < this->activeNavMesh->columns; ++x){
			newDist = (activeNavMesh->original2DMap[x][y]->position - position).Length();
			if (newDist < distance){
				distance = newDist;
				closestWaypoint = activeNavMesh->original2DMap[x][y];
			}
		}
	}
	std::cout<<"\nClosest waypoint: "<<closestWaypoint->position;
	/// Do stuff
	closestWaypoint->passable = !closestWaypoint->passable;
	/// Check the new mesh too!
	closestWaypoint = this->activeNavMesh->waypoints[0];
	distance = (closestWaypoint->position - position).Length();
	for (int i = 0; i < activeNavMesh->waypoints.Size(); ++i)
	{
		newDist = (activeNavMesh->waypoints[i]->position - position).Length();
		if (newDist < distance){
			distance = newDist;
			closestWaypoint = activeNavMesh->waypoints[i];
		}
	}
	/// Do stuff
	std::cout<<"\nClosest waypoint: "<<closestWaypoint->position;
	closestWaypoint->passable = !closestWaypoint->passable;

	if (activeNavMesh->optimized){
		activeNavMesh->Optimize();
	}
	else
		; ///activeNavMesh->ReloadFromOriginal();
//    mutex.Release();
	return true;
}

/// Returns a waypoint that has no active data bound to it's pData member.
Waypoint * WaypointManager::GetFreeWaypoint(){
	Waypoint * wp = NULL;
	while(true){
		wp = activeNavMesh->waypoints[rand()%activeNavMesh->waypoints.Size()];
		if (wp->passable && wp->pData == NULL)
			return wp;
	}
	return NULL;
}
/// Returns the closest waypoint to target position that is passable/walkable/valid.
Waypoint * WaypointManager::GetClosestValidWaypoint(Vector3f position){
	int waypoints = activeNavMesh->waypoints.Size();
	if (!waypoints)
		return NULL;
	assert(waypoints > 0);
	/// Get a first waypoint
	Waypoint * closest = NULL;
	for (int i = 0; i < waypoints; ++i){
		if (activeNavMesh->waypoints[i]->passable){
			closest = activeNavMesh->waypoints[i];
			break;
		}
	}
	float closestDist = (closest->position - position).Length();
	for (int i = 0; i < waypoints; ++i){
		if (!activeNavMesh->waypoints[i]->passable)
			continue;
		float thisDist = (activeNavMesh->waypoints[i]->position - position).Length();
		if (thisDist < closestDist){
			closest = activeNavMesh->waypoints[i];
			closestDist = thisDist;
		}
	}
	return closest;
}

/// Gets the closest valid free waypoint (equivalent to GetFreeWaypoint combined with GetClosestValidWaypoint)
Waypoint * WaypointManager::GetClosestValidFreeWaypoint(Vector3f position)
{
	assert(activeNavMesh->waypoints.Size() > 0);
	/// Get an initial one..
	Waypoint * closest = NULL;
	for (int i = 0; i < activeNavMesh->waypoints.Size(); ++i){
		if (!activeNavMesh->waypoints[i]->passable)
			continue;
		if (activeNavMesh->waypoints[i]->pData)
			continue;
		closest = activeNavMesh->waypoints[i];
		break;
	}
	float closestDist = (closest->position - position).Length();
	for (int i = 0; i < activeNavMesh->waypoints.Size(); ++i)
	{
		if (!activeNavMesh->waypoints[i]->passable)
			continue;
		if (activeNavMesh->waypoints[i]->pData)
			continue;
		float thisDist = (activeNavMesh->waypoints[i]->position - position).Length();
		if (thisDist < closestDist){
			closest = activeNavMesh->waypoints[i];
			closestDist = thisDist;
		}
	}
	assert(closest->passable && closest->pData == NULL);
	return closest;
}

Waypoint * WaypointManager::GetClosestVacantWaypoint(Vector3f position){
	assert(activeNavMesh);
	Waypoint * wp = activeNavMesh->GetClosestVacantWaypoint(position);
	return wp;
}

/** Loads waypoint map from target file.
*/
bool WaypointManager::Load2DWaypointMap(const char * filename, bool optimize){
	assert(false);
	/*
	/// Get a valid navMesh index before we start.
	int index = -1;
	for (int i = 0; i < MAX_NAVMESHES; ++i){
		if (navMesh[i] == NULL){
			index = i;
			break;
		}
	}
	if (index < 0)
		return false;

	/// File data
	char * data = NULL;
	/// File data size
	int size;
	// Try read file
	try {
		std::fstream fileStream;
		fileStream.open(filename, std::ios_base::in);
		if (fileStream == NULL)
			return false;

		int start  = (int) fileStream.tellg();

		// Get size by seeking to end of file
		fileStream.seekg( 0, std::ios::end );
		size = (int) fileStream.tellg();

		// Allocate data array for length
		data = new char [size];
		memset(data, 0, size);

		// Go to beginning of file and read the data
		fileStream.seekg( 0, std::ios::beg);
		fileStream.read((char*) data, size);

		// Then close the stream
		fileStream.close();
	} catch(...){
		return false;
	}

	std::cout<<"\nWaypointManager: File read successfully, begun parsing.";

	int rows = 0, columns = -1;
	std::stringstream ss(data);
	char line[2048];
	while (ss.good()){
		ss.getline(line, 2048);
		int lineLength = strlen(line);
		if (columns != lineLength && columns != -1){
			std::cout<<"\nIrregular amount of columns in file waypoint file! "<<columns<<" vs. "<<lineLength;
		}
		columns = lineLength;
		++rows;
	}
	if (columns <= 0 && rows <= 0){
		std::cout<<"\nNo valid data in waypoint file, aborting.";
		return false;
	}

	/// Allocate map data array
	Waypoint *** map = new Waypoint ** [columns];
	for (int x = 0; x < columns; ++x){
		map[x] = new Waypoint *[rows];
		for (int y = 0; y < rows; ++y)
			map[x][y] = new Waypoint();
	}

	// Parse data
	int column = 0, row = 0;
	Waypoint * start = NULL, * goal = NULL;
	for (int i = 0; i < size; ++i){
		Waypoint * wp;
		if (row < rows && column < columns){
			wp = map[column][row];
			/// Set waypoint position
			float x = (column - columns/2.0f) * 10.0f;
			float y = (row - rows/2.0f) * 10.0f;
			wp->position = Vector3f(x, 0, y);
		}
		switch(data[i]){
			/// Goal
			case 'g': case 'G':
				/// Add as a regular passable node, but set it as default goal node in the NavMesh
				wp->passable = true;
				wp->data |= DEFAULT_GOAL;
				break;
			/// PAssable terrain
			case '0': case 'o': case 'O':
				/// Add as a regular passable node
				wp->passable = true;
				break;
			/// Start
			case 's': case 'S':
				/// Add as a regular passable node, but set it as default starting node in the NavMesh
				wp->passable = true;
				wp->data |= DEFAULT_START;
				break;
			/// Impassable terrain
			case 'x': case 'X':
				/// Unnecessary to add to map...!
				wp->passable = false;
				break;
			/// Next row
			case '\n':
				++row;
				column = 0;
				continue;	/// Continue to avoid the column iteration below!
			/// End of file
			case '\0':
				std::cout<<"\nEnd of file at character "<<(int)i;
				i = size;
				break;
			default:
				std::cout<<"\nDefault case label encountered in WaypointManager::Load2DWaypointMap for character \'"<<data[i]<<"\'";
		}

		++column;
		if (column > columns && row > rows)
			break;
	};


	/// Register each created waypoint with the new NavMesh
	navMesh[index] = new NavMesh();
	NavMesh * nm = navMesh[index];

	/// Save the loaded 2D map into the NavMesh for future optimization operations.
	nm->original2DMap = map;
	nm->rows = rows;
	nm->columns = columns;

	/// Load data from the 2D map to build the current map for usage.
	nm->ReloadFromOriginal();

/*
	/// Deallocate map data array
	for (int i = 0; i < rows; ++i)
		delete[] map[i];
	delete[] map;
*//*
	/// Save map source
	nm->SetSource(filename);
	/// Make the first loaded file the default one~
	if (!activeNavMesh)
		activeNavMesh = nm;
	return true;
	*/
	return false;
}

/// Returns the loaded navMesh by source
NavMesh * WaypointManager::GetNavMesh(String source){
	if (source.Length() == 0)
		return NULL;
	for (int i = 0; i < navMeshList.Size(); ++i){
		NavMesh * nm = navMeshList[i];
		String nmSource = nm->source;
		if (source == navMeshList[i]->source)
			return navMeshList[i];
	}
	return NULL;
}

/// Makes it active for manipulation
void WaypointManager::MakeActive(NavMesh * nm){
	assert(navMeshList.Exists(nm));
	activeNavMesh = nm;
}
/// Clears all waypoints from the navmesh!
void WaypointManager::Clear(){
	assert(activeNavMesh);
	activeNavMesh->Clear();
}

/** Attempts to get control of the Active NavMesh Mutex
	Make sure you call ReleaseActiveNavMeshMutex afterward!
*/
bool WaypointManager::GetActiveNavMeshMutex(int maxWaitTime){
	WaypointMan.activeNavMeshMutex.Claim(-1);
	return true;
}
/// Releases active NavMeshMutex.
bool WaypointManager::ReleaseActiveNavMeshMutex(){
	WaypointMan.activeNavMeshMutex.Release();
	return true;
}

/// Returns the active nav mesh
NavMesh * WaypointManager::ActiveNavMesh(){
	return waypointManager->activeNavMesh;
}

/// Optimizes the currently selected navMesh
int WaypointManager::Optimize(){
	GetActiveNavMeshMutex();
	/// Do the actual stuff.
	int optimizationResult = activeNavMesh->Optimize();
	ReleaseActiveNavMeshMutex();
	return optimizationResult;
}
