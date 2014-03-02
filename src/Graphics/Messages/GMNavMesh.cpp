// Emil Hedemalm
// 2013-07-18

#include "GMNavMesh.h"
#include "GraphicsMessages.h"
#include "../GraphicsManager.h"
#include "../RenderSettings.h"
#include "Pathfinding/Waypoint.h"

GMSetSelectedWaypoints::GMSetSelectedWaypoints(List<Waypoint*> waypointsToRenderAsSelected)
: GraphicsMessage(GM_NAVMESH)
{
	wpList = waypointsToRenderAsSelected;	
};

void GMSetSelectedWaypoints::Process(){
	Graphics.renderSettings->selectedWaypoints = wpList;
};

GMSetPathToRender::GMSetPathToRender(Path path)
: GraphicsMessage(GM_NAVMESH), path(path){	
}
void GMSetPathToRender::Process(){
	Graphics.renderSettings->pathToRender = path;
}