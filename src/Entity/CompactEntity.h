#ifndef COMPACT_ENTITY_H
#define COMPACT_ENTITY_H

#include "MathLib.h"
#include <Util.h>
#include <fstream>

struct CompactGraphics;
struct CompactPhysics;

#define HAS_PHYSICS		0x00000001
#define HAS_AI			0x00000002
#define HAS_CHILDREN	0x00000004
#define HAS_GRAPHICS	0x00000008

/** Compact Entity struct that is a storage class for entities when saving/loading
	as well as when they are not active in an unloaded map to decrease memory usage but still enabling
	dynamic level loading.
*/
class CompactEntity {
public:
	CompactEntity();
	~CompactEntity();

	char name[240];
	char model[240];		// Name of the default model
	char diffuseMap[240];		// Name of the default texture
	char specularMap[240];		// Name of the default texture
	char normalMap[240];		// Name of the default texture
	Vector3f position;			// Position
	Vector3f scale;				// Scale
	Vector3f rotation;			// Rotation

	/// Getter functions for total dependencies
	List<String> GetModelDependencies();
	List<String> GetTextureDependencies();

	/// Holds flags for the other external properties and attributes
	int extras;
	/// Compact graphics data, beyond the default texture/model.
	CompactGraphics * cGraphics;
	/// Compact physics data
	CompactPhysics * cPhysics;
	
	// The radius should be automatically calculated by multiplying the mesh or model radius with the scale vector of the entity!
//	float radius;			// Radius used in Culling and physics operations.

	/// Reads data from file stream
	virtual bool ReadFrom(std::fstream& file);
	/// Write data to file stream
	virtual bool WriteTo(std::fstream& file);

	/// Calculates size the entity will take when saving, including any eventual sub-properties (if we place them here?)
	int Size();

private:



};

#endif