// Emil Hedemalm
// 2013-07-20

#include <List/List.h>
#include "MathLib.h"

struct CompactGraphicEffect;
struct CompactParticleSystem;
struct CompactLight;
struct GraphicsProperty;

// A compact data structure for saving/loading/storing data relevant for the GraphicsProperty
struct CompactGraphics {
	CompactGraphics();
	~CompactGraphics();
	CompactGraphics(GraphicsProperty * graphicsProperty);
	/// Main render-flags.
	int flags;

	/// Wosh.
	List<CompactGraphicEffect> * cGraphicEffects;
	List<CompactParticleSystem> * cParticleSystems;
	List<CompactLight> * cLights;

	/// Reads data from file stream
	bool ReadFrom(std::fstream& file);
	/// Write data to file stream
	bool WriteTo(std::fstream& file);
};