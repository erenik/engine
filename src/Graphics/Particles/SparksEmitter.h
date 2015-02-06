/// Emil Hedemalm
/// 2014-09-19
/// Emitter dedicated to creating "spark"-like particles, as if something is on fire or creating heat-related sparks

#include "ParticleEmitter.h"

class SparksEmitter : public ParticleEmitter
{
public:
	SparksEmitter();
	SparksEmitter(const Vector3f & point);
	virtual ~SparksEmitter();

	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity);
	/// Extended particle emission.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color);

	void SetRatioRandomVelocity(float part);

private:
	float velRandPart, velConstPart;
	static Random velocityRandom;
	static Random lifeTimeRandom;
};




