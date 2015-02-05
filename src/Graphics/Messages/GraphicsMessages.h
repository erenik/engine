#ifndef GRAPHICS_MESSAGES_H
#define GRAPHICS_MESSAGES_H

enum graphicsMessages {
	GM_NULL,

	GM_SHUTDOWN,

	// Entire pipeline configuration
	GM_CYCLE_RENDER_PIPELINE,
	GM_CYCLE_RENDER_PIPELINE_BACK,

	// Render-pass configuration
	GM_ADD_RENDER_PASS,

	// Pausing!
	GM_PAUSE_RENDERING,
	GM_RESUME_RENDERING,
	GM_PAUSE_PROCESSING, // Mainly affects processing of particles and similar. Rendering is continued as normal.
	GM_RESUME_PROCESSING,

	// Lighting
	GM_ADD_LIGHT,
	GM_SET_AMBIENCE,
	GM_CREATE_LIGHT,
	GM_DELETE_LIGHT,
	GM_SET_LIGHT,

	// Recording
	GM_RECORD_VIDEO,
	GM_PRINT_SCREENSHOT,
	
	// Animation
	GM_PLAY_ANIMATION,
	GM_QUEUE_ANIMATION,

	// Setters
	GM_SET,				// For setting default textures and stuff
	GM_SET_INTEGER,
	GM_SET_FLOAT,
	GM_SET_TEXTURE,
	GM_SET_STRING,
	GM_SET_UI,
	GM_SET_GLOBAL_UI,
	GM_SET_VIEWPORTS,
	GM_SET_OVERLAY,		// For setting overlay texture.. or Video!

	// Entity setters
	GM_SET_ENTITY,
	GM_SET_ENTITY_BOOLEAN,
	GM_SET_ENTITY_STRING,
	GM_SET_ENTITY_TEXTURE,
	GM_SET_ENTITY_FLOAT,
	GM_SET_ENTITY_INTEGER,
	GM_SET_ENTITY_VEC4F,
	/// Setting values to change over time.
	GM_SLIDE_ENTITY,

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
	GM_RECOMPILE_SHADERS,	// Recompiles all shaders and reloads the render configuration. Made for debugging

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
	GM_TRACK,
	GM_SET_CAMERA,

	// Particle effects,
	GM_ATTACH_PARTICLE_SYSTEM,	// Entity based systems.
	GM_REGISTER_PARTICLE_SYSTEM, // Global based systems, registering
	GM_UNREGISTER_PARTICLE_SYSETM, // Global based systems, unregistering and deletion
	GM_ATTACH_PARTICLE_EMITTER, // Attaching emitters to systems.
	GM_PAUSE_EMISSION,
	GM_RESUME_EMISSION,
	GM_SET_PARTICLE_EMITTER,
	GM_GENERATE_PARTICLES,
	GM_SET_PARTICLE_SYSTEM,

	// UI messages
	GM_SET_UI_INTEGER,
	GM_SET_UI_FLOAT,
	GM_SET_UI_VECTOR,
	GM_SET_UI_POINTER,
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
enum graphicsTargets
{
	GT_NULL_GRAPHIC_TARGET,

	// Parenting, for relative displacement/rotation, etc.
	GT_PARENTING_TARGETS = 0,
	GT_PARENT,

	/// Sleep- and thread-control for efficiency and regulation,
	GM_SET_OUT_OF_FOCUS_SLEEP_TIME,

	// Blending calculations
	GT_BLEND_EQUATION,

	// General stuff
	GT_GENERAL_TARGETS = GT_PARENTING_TARGETS + 100,
	GT_MAIN_CAMERA,	// To use when rendering scenes.
    GT_CLEAR_COLOR,    // Color to clear screen/window with.
	GT_FOG_BEGIN,		// Foggy fog-some. Fog color is defined be the GT_CLEAR_COLOR by default.
	GT_FOG_END,

	GT_OVERLAY_TEXTURE,
	GT_ACTIVE_USER_INTERFACE,
	GT_GRID_SPACING,
	GT_GRID_SIZE,
	GT_ACTIVE_2D_MAP_TO_RENDER,
	GT_MAX_GRAPHICS_TARGETS,

	// Particle systems
	GT_PARTICLE_TARGETS_0 = GT_GENERAL_TARGETS + 100,
	GT_PARTICLE_SYSTEMS = GT_PARTICLE_TARGETS_0,
	GT_PARTICLE_INITIAL_COLOR,
	GT_PARTICLE_EMISSION_VEOCITY,
	GT_PARTICLE_SCALE,
	GT_PARTICLE_TEXTURE, // Setting source or texture.
	GT_PARTICLE_LIFE_TIME,
	GT_USE_INSTANCED_RENDERING,
	GT_SET_PARTICLE_EMITTER_OF_PARTICLE_SYSTEM,
	GT_EMITTER_POSITION, // Position of emitters
	GT_EMITTER_DIRECTION, // Dir for point/directional emitters
	GT_EMITTER_ENTITY_TO_TRACK, // Entity to track
	GT_EMITTER_POSITION_OFFSET, // 
	GT_EMISSIONS_PER_SECOND, // Per emitter
	
	// Camera targets
	GT_CAMERA_TARGETS = GT_PARTICLE_TARGETS_0 + 100,
	GT_CAMERA_TARGET_0,
	GT_CAMERA_TARGET_20 = GT_CAMERA_TARGET_0 + 50,

	// Added with SetEntity
	GT_SET_ENTITY_TARGETS = GT_CAMERA_TARGETS + 100,
	GT_REQUIRE_DEPTH_SORTING, // So rendering things work properly, specifically for sprites.
	GT_BLEND_MODE_SRC, // E.g. GL_ONE for additive blending, or GL_ONE_MINUS_SRC_ALPHA for regular alpha-blending.
	GT_BLEND_MODE_DST,
	GT_DEPTH_TEST, // True to require depth test, false to skip.
	GT_VISIBILITY,
	GT_TEXT,
	GT_TEXT_COLOR,
	GT_TEXT_POSITION,
	GT_TEXT_SIZE_RATIO,
	GT_CAMERA_FILTER, // Filter to enable per-viewport disabled rendering of an entity for example.
	GT_ADD_CAMERA_FILTER, // Adds a camera filter.
	GT_REMOVE_CAMERA_FILTER,
	GT_CLEAR_CAMERA_FILTER, // For clearing said filter.
	GT_RENDER_OFFSET, // For rendering at a position slightly different than that stated by the physics and navMesh grid.
	GT_ANIMATE_SKIN_USING_SHADERS, // For toggling Skeleton-animation to be conducted on the CPU or using Shaders on the GPU.
	GT_PAUSE_ANIMATIONS, // For temporary pausing of all graphical animations.
	
	// Added with GraphicsEffects.
	GT_GRAPHIC_EFFECTS_TARGETS = GT_SET_ENTITY_TARGETS + 100,
	GT_ALPHA,
	GT_RELATIVE_SCALE,
	GT_MODEL,
	GT_ANIMATION_SET, // Should maybe use other enum?
	GT_ANIMATION,
	GT_QUEUED_ANIMATION,
	GT_TEXTURE,


	GT_MAX_TARGETS,
};

#endif
