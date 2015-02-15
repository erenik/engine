/// Emil Hedemalm
/// 2014-08-06
/// Enum over various shapes.

// Re-naming from ShapeType to PhysicsShape.
#define ShapeType PhysicsShape 

namespace PhysicsShape  {
	enum PhysicsShapes {
		SPHERE,		// Uses the Entity's internal [radius] and [position]
		PLANE,	// An unlimited plane!
		TRIANGLE,	// A trilangle o-o
		QUAD,		// A quad o-o;
		CYLINDER,
		MESH, // Usually refers to PhysicsMesh class.
		/// Don't mess with the order here, only insert new ones below as the save/load relies on this list for the physics. :)
		CUBE,
		AABB, // Refers to the entity's own AABB. 	
		NUM_TYPES,
		DEFAULT_TYPE = SPHERE,
	};
	const char PLANE_STR [] = {"Plane"};
	const char TRIANGLE_STR [] = {"Triangle"};
	const char QUAD_STR [] = {"Quad"};
	const char SPHERE_STR [] = {"Sphere"};
	const char MESH_STR [] = {"Mesh"};
};
