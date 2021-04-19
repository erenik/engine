#ifndef STATE_H
#define STATE_H

/*
#include "Uniform.h"
#include "Texture.h"
#include "ShaderManager.h"
#include "Lighting.h"
*/

// Might as well include OpenGL here 
#include "Graphics/OpenGL.h"

#include "PhysicsLib/Shapes/Frustum.h"
#include "MathLib/Rect.h"
#include "Util/List/List.h"
#include "Entity/Entities.h"

#include "Pathfinding/Path.h"
#include "Lighting.h"

class RenderPipeline;
class Waypoint;
class AppWindow;
class Viewport;
class Shader;
class Texture;
class Mesh;
struct GraphicEffect;
class ParticleSystem;
class TextFont;
class Camera;
class Light;
class GraphicsState;
class EntityGroup;
class RenderInstancingGroup;

#define GraphicsStatePtr (GraphicsState*)

class Entity;
#define EntitySharedPtr std::shared_ptr<Entity>

#define checkGLError() {int error = glGetError();if (error != GL_NO_ERROR)throw 1;}

#define RENDER_LIGHT_POSITION			0x00000001
#define VIEW_FRUSTUM_CULLING			0x00000002
#define USE_VFC_OCTREE					0x00000004
#define USE_LEGACY_GL					0x00000008
#define RENDER_SORTED_ENTITIES			0x00000010	// For when toggling when to render the entities that require sorting and not!
#define ENABLE_SPECIFIC_ENTITY_OPTIONS	0x00000020  // Enables specific render options on a per-entity basis. Disable this for rendering in custom manner.
#define SCISSOR_DISABLED                0x00000040  // For toggling scissor-functionality which is used by UILists et al.

// Macro while rendering
#define ActiveViewport (graphicsState.activeViewport)

/// Frame ID/number/enumeration.
extern int graphicsFrameNumber;

/** A structure containing information about the current rendering settings and temporary variables, including:
	- the current model matrix.
	- a pointer to relevant mesh.
	- an index of the currently bound texture, to avoid re-binding the same texture.
	- an integer with flags for debugging purposes.
*/
class GraphicsState {
public:
	GraphicsState();
	~GraphicsState();
public:
	/// Adds an entity to the graphics state, which includes sorting it into proper instancing groups, if flagged for it.
	void AddEntity(EntitySharedPtr entity);
	void RemoveEntity(EntitySharedPtr entity);
	void RemoveAllEntities();
	void UpdateRenderInstancingGroupBuffers();
	/// Calls glScissor, and updates locally tracked scissor. Appends current viewport x0/y0 co-ordinates automatically to the GL call.
	void SetGLScissor(const Rect & scissor);
	void SetCamera(Camera * camera);
	/// What pipeline is currently being used.
	RenderPipeline * renderPipe;
	/// AppWindow we are currently rendering to.
	AppWindow * activeWindow; 
	Viewport * activeViewport;
	/// Current width and height of the active AppWindow.
	int windowWidth, windowHeight;
	
	/// Desired resolution during rendering. Updated automatically when window-sizes changes. Lock it using GM_SET_RESOLUTION messages. Applicable ONLY to the main window and main pipeline rendering.
	Vector2i renderResolution;
	Vector2f relativeResolution;
	bool resolutionLocked;
	bool antialiasing; // 0 off, 1 on?
	/// Distance. If 0 or negative, will use default values from camera.
	int farPlane;

	/// All entities to render?
	List< std::shared_ptr<Entity> > entities;
	/// Those requiring depth-sorting and usually rendered in a separate alpha-pass?
	Entities alphaEntities;
	Entities solidEntities,
		solidEntitiesNotInstanced; // Non-alphas...
	// Shadows..
	Entities shadowCastingEntities,
		shadowCastingEntitiesNotInstanced;

	List<EntityGroup*> entityGroups;
	EntityGroup * defaultEntityGroup;

	/// Sublist.
	List<RenderInstancingGroup*> shadowCastingEntityGroups, solidEntityGroups;
	/// Contains all groups
	List<RenderInstancingGroup*> entityInstancingGroups;

	/** Used to smooth out positions and quaternions of entitys to solve issue of temporal aliasing  (http://gafferongames.com/game-physics/fix-your-timestep/)
		Default 0.5?
	*/
	float perFrameSmoothness;
	/// Active frustum to be compared with.
	Frustum viewFrustum;
	/// Rendered amount of objects for this frame.
	int renderedObjects;

	/// Current lighting setup
	Lighting lighting;
	/// Lights which move o-o
	List<Light*> dynamicLights;
	/// Active shaderProgram -> Moved to be stored in ShaderManager::ActiveShader()
//	Shader * activeShader;
	/// List of all graphical effects that are to be rendered. Can be stocked up during initial culling!
	List<GraphicEffect*> graphicEffectsToBeRendered;
	List<std::shared_ptr<ParticleSystem> > particleEffectsToBeRendered;
	/// Entities that require depth-sorting before they can be rendered..!
	Entities entitiesRequiringSorting;

	/// The current model matrix in floats.
#define modelMatrixF modelMatrix
#define modelMatrixD modelMatrix
	Matrix4f modelMatrix;
	/// The current model matrix in doubles.
//	Matrix4d modelMatrixD;

	/// The current view matrix in floats.
	Matrix4f viewMatrixF;
	/// The current view matrix in doubles.
	Matrix4d viewMatrixD;

	/// The current projection matrix in floats.
	Matrix4f projectionMatrixF;
	/// The current projection matrix in doubles.
	Matrix4d projectionMatrixD;

	/** Settings, flagged bits determine rendering behavior, if any.
		0x00000001 - RENDER_LIGHT_POSITION - Render light position with 7 GL_POINTS
		0x00000002 - VIEW_FRUSTUM_CULLING -
		0x00000004 - USE_VFC_OCTREE -
		0x00000008 - USE_LEGACY_GL - Render entities using legacy GL
		0x00000010 - RENDER_SORTED_ENTITIES	- For when toggling when to render the entities that require sorting and not!
		0x00000020 - ENABLE_SPECIFIC_ENTITY_OPTIONS - Enables specific render options on a per-entity basis. Disable this for rendering in custom manner.
        0x00000040 - SCISSOR_DISABLED - For toggling scissor-functionality which is used by UILists et al.
	*/
	int settings;

	/// For rendering grid, distance between each tile's border to the next.
	float gridSpacing;
	/// Amount of grid-tiles.
	int gridSize;

	/// Active camera
	Camera * camera;
	/// Pointer to the latest used/active mesh. This could be used for TransformationLeafs among other nodes to render straight after transformation.
	Mesh * currentMesh;
	/// Currently bound texture.
	Texture * currentTexture;
	Texture * currentSpecularMap;
	Texture * currentNormalMap;
	/// Currently bound font texture
	TextFont * currentFont;
	String fontShaderName;
	/// Time consumed by the previous frame.
	float frameTime;
	/// Current time at the start of the frame, i.e. today at 04:60 for example, but in milliseconds probably.
	int64 frametimeStartMs;

    /// Level of optimization deemed necessary to keep framerate somewhat decent.
	int optimizationLevel;

    /// Scissor! In pixels. Reset before usage.
	Rect scissor;
   // float bottomScissor, topScissor, leftScissor, rightScissor;

	/// Start of the viewport. Default is 0 but should be adjusted for when rendering to multiple viewports for proper scissoring.
	float viewportX0, viewportY0;

	// If true, will take screenshot at the end of this frame.
	bool promptScreenshot;
	int screenshotsTaken;
	// If currently recording screenshots in succession
	bool recording;
	int framesRecorded;

	/// If the current pass is a shadow pass? Default false.
	bool shadowPass;
	/// Moved here from RenderSettings.
	/// ===============================
	/// Bitwise &-ed flags for setting only some things with the message? :)
	int flags;
	// Navmesh and pathfinding.
	List<Waypoint*> selectedWaypoints;
	Path pathToRender;
	/// Color used to clear the screen.
	Vector3f clearColor;

	// For foggy-fogsome.
	float fogBegin, fogEnd;


	/// Used to reduce amount of calls to GL by not re-binding and setting up all vertex buffer stats if the same buffer is still bound. GL_ARRAY_BUFFER type
	int BoundVertexArrayBuffer() const;
	/// Used to reduce amount of calls to GL by not re-binding and setting up all vertex buffer stats if the same buffer is still bound. GL_ARRAY_BUFFER type
	void BindVertexArrayBuffer(int vertexBufferID);
private:
	/// Used to reduce amount of calls to GL by not re-binding and setting up all vertex buffer stats if the same buffer is still bound. GL_ARRAY_BUFFER type
	int boundVertexArrayBuffer;
	RenderInstancingGroup * GetGroup(List<RenderInstancingGroup*> & fromListOfGroups, EntitySharedPtr basedOnEntity);

};

#endif
