#ifndef GRAPHICS_MESSAGES_H
#define GRAPHICS_MESSAGES_H

enum graphicsMessages {
	GM_NULL,
	// Pausing!
	GM_PAUSE_RENDERING,
	GM_RESUME_RENDERING,
	// Setters
	GM_SET,				// For setting default textures and stuff
	GM_SET_FLOAT,
	GM_SET_TEXTURE,
	GM_SET_STRING,
	GM_SET_UI,
	GM_SET_VIEWPORTS,
	GM_SET_OVERLAY,		// For setting overlay texture.. or Video!
	// Entity setters
	GM_SET_ENTITY,
	GM_SET_ENTITY_TEXTURE,
	// Stuff
	GM_CLEAR_OVERLAY_TEXTURE,
	GM_DELETE_UI,
	GM_DELETE_VBOS,
	GM_RESIZE,			// Contains width in parameter 1 and height in parameter 2.
	GM_FULLSCREEN,		// Toggles full-screen mode.
	GM_BUFFER_MESH,		// Bufferizes target Mesh
	GM_BUFFER_TEXTURE,	// Queries buffering of target texture provided in parameter 1
	GM_BUFFER_UI,		// Bufferizes target UI
	GM_RELOAD_UI,			// Reloads the active state's UI and re-bufferizes it
	GM_RELOAD_MODELS,		// Needed in order to clear any active model bindings and gl indices before everything is reloaded!
	GM_RECOMPILE_SHADERS,	// Recompiles all shaders. Made for debugging

	GM_REGISTER_ENTITY,		// Registers an entity for rendering
	GM_REGISTER_ENTITIES,	// Registers entities in the selection for rendering.
	GM_UNREGISTER_ENTITY,	// Unregisters an entity from rendering
	GM_UNREGISTER_ENTITIES,	// Unregisters all entities in the selection from rendering
	GM_UNREGISTER_ALL_ENTITIES,	// Unregisters all entities, no exceptionsh

	GM_REGISTER, // General register-message for various things!
	GM_CLEAR,	// General unregister message for various things!

	GM_SET_LIGHTING,	// Updates lighting model (when loaded from map for example)

	GM_SET_GRAPHIC_EFFECT,	// For setting variables to any and all graphic effects, see GMSetGraphicEffect.h for details.

    GM_RENDER, // For queueing simple objects for temporary rendering, like rays.
    GM_RENDER_FRUSTUM,

    /// Camera control
    GM_RESET_CAMERA,

	// Particle effects,
	GM_GENERATE_PARTICLES,

	// UI messages
	GM_SET_UI_FLOAT,
	GM_SET_UI_VECTOR,
	GM_SET_UI_VECB,
	GM_SET_UI_VEC2I,
	GM_SET_UI_VEC3F,
	GM_SET_UI_VEC4F,
	GM_SET_UI_BOOLEAN,
	GM_SET_UI_TEXT,
	GM_CLEAR_UI, // Deleting contents
	GM_SCROLL_UI, // For lists
	GM_ADD_UI, // For adding children.
	GM_REMOVE_UI,

	// For stack-operations
	GM_PUSH_UI,
	GM_POP_UI,

	// 2D-maps
	GM_CLEAR_ACTIVE_2D_MAP,

	/// CBA, just one category ID for all navmesh-messages.
	GM_NAVMESH,

	GM_MESSAGES
};

/// Targets for some of the prior messages,
enum graphicsTargets{
	NULL_GRAPHIC_TARGET,

	OVERLAY_TEXTURE,
	ACTIVE_USER_INTERFACE,
	GRID_SPACING,
	GRID_SIZE,
	ACTIVE_2D_MAP_TO_RENDER,
	MAX_GRAPHICS_TARGETS,
	PARTICLE_SYSTEMS,

    // General stuff
	MAIN_CAMERA,	// To use when rendering scenes.
    CLEAR_COLOR,    // Color to clear screen/window with.
	FOG_BEGIN,		// Foggy fog-some. Fog color is defined be the CLEAR_COLOR by default.
	FOG_END,

	// Added with SetEntity

	// Added with GraphicsEffects.
	ALPHA,
	RELATIVE_SCALE,
	MODEL,
	ANIMATION_SET, // Should maybe use other enum?
	ANIMATION,
	QUEUED_ANIMATION,
	TEXTURE,


	MAX_TARGET,
};

#endif
