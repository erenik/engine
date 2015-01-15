/// Emil Hedemalm
/// 2014-09-16
/// Middle parent class for filters using particles?

#ifndef CV_PARTICLE_RENDER_FILTER_H
#define CV_PARTICLE_RENDER_FILTER_H

#include "CVRenderFilter.h"

/// Parent class for all render-filters which delve in rendering particles via an entity.
class CVParticleRenderFilter : public CVRenderFilter 
{
public:			

	CVParticleRenderFilter(int filterID);
	virtual ~CVParticleRenderFilter();
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
	/// Performs initial setup of the particle system if wanted.
	virtual int Process(CVPipeline * pipe);
	/// o.o
	virtual void ProcessMessage(Message * message);
	/// Wat..
	virtual List<Entity*> GetEntities();
protected:
	void StopEmission();
	/// The particle system
	ParticleSystem * particleSystem;
	/// We will probably have an entity which serves as a base for the particle emission.
	Entity * particleEmitterEntity;

	/// Filter settings.
	CVFilterSetting * color, * emissionVelocity, * particleLifetime, * particleSize, * instancedRendering,
		* emissionsPerSecond;
	/// Goes to use next texture found within the /img/Particles/ directory.
	CVFilterSetting * textureIndex;
};


class CVContourParticles : public CVParticleRenderFilter 
{
public:
	CVContourParticles();
	virtual int Process(CVPipeline * pipe);	
};

class CVFingerParticles : public CVParticleRenderFilter 
{
public:
	CVFingerParticles();
	virtual int Process(CVPipeline * pipe);	
private:
};

class CVHandParticles : public CVParticleRenderFilter 
{
public:
	CVHandParticles();
	virtual int Process(CVPipeline * pipe);
private:
};

class CVOFParticles : public CVParticleRenderFilter 
{
public:
	CVOFParticles();
	virtual int Process(CVPipeline * pipe);
private:

};


#endif
