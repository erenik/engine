// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#include "ParticleEmitter.h"
#include "Random/Random.h"

Random random;

ParticleEmitter::ParticleEmitter(Contour contour)
	: contour(contour), type(EmitterType::CONTOUR)
{
}  

ParticleEmitter::ParticleEmitter(Vector3f point, Vector3f direction)
	: point(point), direction(direction), type(EmitterType::POINT_DIRECTIONAL)
{
	this->direction.Normalize();
}

/// Point-based circular emitter
ParticleEmitter::ParticleEmitter(Vector3f point)
	: point(point), type(EmitterType::POINT_CIRCLE)
{

}
/// Stuff.
bool ParticleEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity)
{
	switch(type)
	{
		case EmitterType::CONTOUR:
		{
			if (!contour.points.Size())
				return false;
			// Get a random index in the contour.
			int randomIndex = rand() % contour.points.Size();
			position = contour.points[randomIndex];
			// Get next point.
			Vector3f nextPoint = contour.points[(randomIndex+1) % contour.points.Size()];
			Vector3f pToNext = nextPoint - position;
			/// Set the point between this and the next point, actually!
			float r = (rand() % 100) * 0.01f;
			position += pToNext * r;
			// Use an arbitrary up-vector?
			Vector3f upVec(0,0,-1);
			Vector3f crossProduct = upVec.CrossProduct(pToNext);
			crossProduct.Normalize();
			velocity = crossProduct;
			break;
		}
		case EmitterType::POINT_DIRECTIONAL:
		{
			position = this->point;
			velocity = this->direction;
			// Randomize it a bit.
			Vector3f upVec(0,0,-1);
			Vector3f crossProduct = velocity.CrossProduct(upVec);
			crossProduct.Normalize();
			velocity += crossProduct * (rand()% 100)*0.01f; 
			break;
		}
		case EmitterType::POINT_CIRCLE:
		{
			position = this->point;
			// Random angle.
			float angle = random.Randf() * 2 * PI;
			float x = cos(angle);
			float y = sin(angle);
			velocity = Vector3f(x,y,0);
			break;
		}
		default:
			std::cout<<"\nINVALID PARTICLE EMITTER TYPE";
			assert(false);
	}
	return true;
}
