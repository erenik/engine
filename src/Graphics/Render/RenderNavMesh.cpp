/// Emil Hedemalm
/// 2013-03-01

#include "Graphics/GraphicsManager.h"
#include "../RenderSettings.h"
#include "GraphicsState.h"
#include "Pathfinding/WaypointManager.h"
#include "Pathfinding/PathManager.h"

#include "Model/ModelManager.h"
#include "Model/Model.h"


#define SELECTED_HARD_COLOR {glColor4f(1.8f, 1.6f, 0.5f, 0.6f);}
#define SELECTED_COLOR 	{glColor4f(1.0f, 0.9f, 0.6f, 0.6f);}
#define REGULAR_COLOR {glColor4f(0.5f, 0.7f, 0.9f, 0.4f);}
#define HAS_ENTITY_COLOR {glColor4f(2.0f,0,0,0.7f);}

/// Sets a color fitting for the waypoint.
void SetColorForWaypoint(Waypoint * wp)
{
	Vector4f color;
	if (wp->passable){
		color = Vector4f(0.5f, 0.7f, 0.9f, 0.4f);
	}
	if (!wp->passable) {
		color = Vector4f(1.0f,0,0,0.5f);
	}

	// If the waypoint currently has an entity, adjust color.
	if (wp->entities.Size()){
		color += Vector4f(2.0f,0,0,0.7f);
	}
	/// If in selection, highlight
//	if (graphicsState->selectedWaypoints.Exists(wp)){
	//	color += Vector4f(0.2, 0.2f, 0, 0.2f);
//	}
	glColor4f(color[0],color[1],color[2],color[3]);
}

/// Renders target Navigation-meshes used for pathfinding
void GraphicsManager::RenderNavMesh()
{
	bool success = WaypointMan.GetActiveNavMeshMutex();
	if (!success)
		return;
	/// Assume projection and view has already been set up for us!
	NavMesh * nm = WaypointMan.ActiveNavMesh();
	if (!nm){
		WaypointMan.ReleaseActiveNavMeshMutex();
		return;
	}
	/// Rendering connections?
	glDisable(GL_DEPTH_TEST);
		
	/// Draw a line-strip too, using default renderer!
	int waypointRendered = 0;
	ShadeMan.SetActiveShader(NULL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->viewMatrixF.getPointer());

	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState->currentTexture = NULL;
	glLineWidth(2.0f);
	glPointSize(3.0f);

	glBegin(GL_POINTS);	
	

	float passablesRendered = 0;
	/// Draw points at each waypoint first?
	for (int i = 0; i < nm->waypoints.Size(); ++i)
	{
		Waypoint * wp = nm->waypoints[i];
		SetColorForWaypoint(wp);
		// Render if we got a model ^^
		glVertex3f(wp->position[0], wp->position[1], wp->position[2]);
	}
	glEnd();

	if (true /*renderNavMeshNeighbours*/)
	{
		for (int i = 0; i < nm->waypoints.Size(); ++i)
		{
			Waypoint * wp = nm->waypoints[i];
			for (int j = 0; j < wp->neighbours; ++j)
			{
				Waypoint * nb = wp->neighbour[j];
				glBegin(GL_LINES);
					SetColorForWaypoint(wp);
					glVertex3f(wp->position[0], wp->position[1], wp->position[2]);
					/// Always draw the other vertex as alpha'd to get a better grip, yaow?
					SetColorForWaypoint(nb);
					glColor4f(0,0,0,0);
					glVertex3f(nb->position[0], nb->position[1], nb->position[2]); 
				glEnd();
			}
		}
	}

	/// Renders paths
	if (true)
	{

		glDisable(GL_DEPTH_TEST);
		Path & path = graphicsState->pathToRender;
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < path.Waypoints(); ++i){
			Waypoint * wp = path.GetWaypoint(i);
			Vector3f pos = wp->position;
			glColor4f(1.0f - 0.001f * i, 0.2f + 0.005f * i, 0.2f + 0.005f * i, 0.7f);
			glVertex3f(pos[0], pos[1], pos[2]);
		}
		glEnd();
		glEnable(GL_DEPTH_TEST);
	}

	glPointSize(1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/// Release mutex as needed.
	WaypointMan.ReleaseActiveNavMeshMutex();
	return;
}

/// Renders target paths
void GraphicsManager::RenderPath()
{
	if (!PathMan.GetLatsPathMutex(10))
		return;
	
	PathMan.ReleaseLastPathMutex();
		
	//  Old shit.
	return;

	Shader * shader = ActiveShader();
	/// Get last calculated path
	Path path;
	PathMan.GetLastPath(path);
	if (!path.Waypoints())
		return;

	// Get model to render
	Model * model = ModelMan.GetModel("plane.obj");
	assert(model);

	glDisable(GL_DEPTH_TEST);
	
	/// Set shader prugrum!
	ShadeMan.SetActiveShader("Wireframe");

	int waypoints = path.Waypoints();
	float waypointRendered = 0;
	for (int i = 0; i < path.Waypoints(); ++i){
		Waypoint * wp = path.GetWaypoint(i);
		if (wp == NULL){
			std::cout<<"\nWARNING: Waypoint is NULL for some reason o.O";
		}
		/// Passable
		glUniform4f(shader->uniformPrimaryColorVec4, 
			0.4f + waypointRendered / waypoints, 
			1.0f + waypointRendered / waypoints, 
			0.4f + waypointRendered / waypoints, 
			1.0f);
		++waypointRendered;
		/// Translate to waypoint position.
		Matrix4f transform;
		transform = Matrix4f::InitTranslationMatrix(wp->position + Vector3f(0,1,0));
		transform.Scale(4);
		/// Set uniform matrix in shader to point to the AppState modelView matrix.
 		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, transform.getPointer());
		// Render if we got a model ^^
		model->Render(*graphicsState);
	}
	/// Draw a line-strip too, using default renderer!
	waypointRendered = 0;
	ShadeMan.SetActiveShader(NULL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->viewMatrixF.getPointer());

	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState->currentTexture = NULL;
	glLineWidth(2.0f);

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < path.Waypoints(); ++i){
		Waypoint * wp = path.GetWaypoint(i);
		/// Passable
		glColor4f(
			0.4f + waypointRendered / waypoints, 
			1.0f + waypointRendered / waypoints, 
			0.4f + waypointRendered / waypoints, 
			1.0f);
		++waypointRendered;
		/// Set uniform matrix in shader to point to the AppState modelView matrix.
 		glVertex3f(wp->position[0], wp->position[1] + 1.0f, wp->position[2]);
	}
	glEnd();
	return;
}