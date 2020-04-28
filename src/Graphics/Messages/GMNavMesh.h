// Emil Hedemalm
// 2013-07-18

#ifndef GM_NAVMESH_H
#define GM_NAVMESH_H

#include "GraphicsMessage.h"
#include "Pathfinding/Waypoint.h"
#include "Pathfinding/Path.h"

/// All messages related to waypoints and navmesh-rendering.

class GMSetSelectedWaypoints : public GraphicsMessage {
public:
	GMSetSelectedWaypoints(List<Waypoint*> waypointsToRenderAsSelected);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	List<Waypoint*> wpList;
};

class GMSetPathToRender : public GraphicsMessage {
public:
	GMSetPathToRender(Path path);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	Path path;
};

#endif