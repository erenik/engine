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
void ParticleSystem::Render(){
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
