// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter(Contour contour)
	: contour(contour), type(EmitterType::CONTOUR)
{
}  

ParticleEmitter::ParticleEmitter(Vector3f point, Vector3f direction)
	: point(point), direction(direction), type(EmitterType::POINT)
{
	this->direction.Normalize();
}

/// Stuff.
bool ParticleEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity)
{
	if (type == EmitterType::CONTOUR)
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
	}
	else if (type == EmitterType::POINT)
	{
		position = this->point;
		velocity = this->direction;
		// Randomize it a bit.
		Vector3f upVec(0,0,-1);
		Vector3f crossProduct = velocity.CrossProduct(upVec);
		crossProduct.Normalize();
		velocity += crossProduct * (rand()% 100)*0.01f; 
	
	}
	else
	{
		std::cout<<"\nINVALID PARTICLE EMITTER TYPE";
	}
	return true;
}
