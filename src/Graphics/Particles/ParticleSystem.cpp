// Emil Hedemalm
// 2013-07-14

#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(String type)
: type(type), name("Undefined")
{
    lifeDuration = NULL;
    positions = NULL;
    relativeTo = NULL;
    colors = NULL;
    emissionPaused = false;
	particleSize = 1.0f;
	emissionRatio = 1.0f;
	registeredForRendering = false;
}
ParticleSystem::~ParticleSystem(){
    std::cout<<"\nParticleSystem Destructor.....";
    if (lifeDuration)
        delete[] lifeDuration;
        lifeDuration = NULL;
	if (lifeTime)
		delete[] lifeTime;
		lifeTime = NULL;	
    if (positions)
        delete[] positions;
        positions = NULL;
    if (velocities)
        delete[] velocities;
        velocities = NULL;
    if (colors)
        delete[] colors;
        colors = NULL;
}

void ParticleSystem::Process(float timeInSeconds){
    assert(false);
}
void ParticleSystem::Render(GraphicsState * graphicsState){
    assert(false);
}
void ParticleSystem::PrintData(){
    assert(false);
}
void ParticleSystem::AttachTo(Entity * entity, Matrix4f relativePosition){
    assert(false);
}
void ParticleSystem::SetPosition(Matrix4f relativePosition){
    assert(false);
}

void ParticleSystem::PauseEmission(){
    emissionPaused = true;
}
void ParticleSystem::ResumeEmission(){
    emissionPaused = false;
}

void ParticleSystem::SetColor(Vector4f icolor){
    color = icolor;
}

/// Sets the emitter to be a contour. Default before calling this is a point or a plane.
void ParticleSystem::SetEmitter(Contour contour)
{
	// Delete the old emitter
	std::cout<<"\nSetting new contour as emitter, with position "<<contour.centerOfMass.x<<", "<<contour.centerOfMass.y<<" and points: "<<contour.points.Size();	
	emitters.ClearAndDelete();
	ParticleEmitter * newEmitter = new ParticleEmitter(contour);
	emitters.Add(newEmitter);
}


void ParticleSystem::SetEmitter(List<ParticleEmitter*> newEmitters)
{
	// Delete the old emitter
	if (emitters)
		emitters.ClearAndDelete();
	emitters = newEmitters;
}
