/// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#include "Contour.h"
#include "MathLib.h"

class Mesh;

namespace EmitterType {
enum emitterTypes
{
	DEFAULT,
	CONTOUR,
	POINT_DIRECTIONAL,
	POINT_CIRCLE,
}; 
};

class ParticleEmitter 
{
	friend class ParticleSystem;
public:
	ParticleEmitter(Contour contour);
    ParticleEmitter(Mesh * mesh);
    ParticleEmitter(int shape);
	/// Point-based directional emitter
	ParticleEmitter(Vector3f point, Vector3f direction);
	/// Point-based circular emitter
	ParticleEmitter(Vector3f point);
	/// Stuff.
    bool GetNewParticle(Vector3f & position, Vector3f & velocity);
private:
    int shapeType;
    Mesh * m;
	Contour contour;
	// See enum above.
	int type;
	/// For point-based emitters.
	Vector2f point, direction;
};
