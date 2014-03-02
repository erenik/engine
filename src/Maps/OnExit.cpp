// Emil Hedemalm
// 2013-07-24

#include "Map.h"
#include "MapManager.h"

struct Map::OnExitAttributes{
	const char * musicAtZone;
};

void Map::OnExit(){
	MapMan.ClearEventSpawnedEntities();
	MapMan.ClearPlayerEntities();
};