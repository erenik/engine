/// Emil Hedemalm
/// 2014-09-16
/// Middle parent class for filters using particles?

#include "CVParticleRenderFilters.h"

#include "CV/CVPipeline.h"

#include "Maps/MapManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Particles/Sparks.h"
#include "Graphics/Particles/SparksEmitter.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Message/Message.h"

#include "TextureManager.h"

#include "File/FileUtil.h"

/// Parent class for all render-filters which delve in rendering particles via an entity.
CVParticleRenderFilter::CVParticleRenderFilter(int filterID)
	: CVRenderFilter(filterID)
{
	particleEmitterEntity = NULL;
	particleSystem = NULL;

	this->color = new CVFilterSetting("Color", Vector3f(1,1,1));
	this->emissionVelocity = new CVFilterSetting("Emission velocity", 1.f);
	particleSize = new CVFilterSetting("Particle size", 1.f);
	instancedRendering = new CVFilterSetting("Use instanced rendering", true);
	emissionsPerSecond = new CVFilterSetting("Emissions per second, per emitter", 30);
	particleLifetime = new CVFilterSetting("Particle life time", 3.f);
	textureIndex = new CVFilterSetting("Texture index", 0);

	settings.Add(7,  
		color, emissionVelocity, particleSize,
		instancedRendering, emissionsPerSecond, particleLifetime,
		textureIndex);
}

CVParticleRenderFilter::~CVParticleRenderFilter()
{
	OnDelete();
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVParticleRenderFilter::OnDelete()
{
	StopEmission();
	if (particleEmitterEntity)
		MapMan.DeleteEntity(particleEmitterEntity);
	particleEmitterEntity = NULL;

	// Unregister it!
	if (particleSystem)
		Graphics.QueueMessage(new GMUnregisterParticleSystem(particleSystem, true));
	particleSystem = NULL;
}

/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVParticleRenderFilter::SetEnabled(bool state)
{
	if (!state)
		StopEmission();	
	CVRenderFilter::SetEnabled(state);
}
List<Entity*> CVParticleRenderFilter::GetEntities()
{
	if (particleEmitterEntity)
		return particleEmitterEntity;
	// Return empty list if not.
	return List<Entity*>();
}
void CVParticleRenderFilter::StopEmission()
{
	if (particleEmitterEntity)
		Graphics.QueueMessage(new GMPauseEmission(particleEmitterEntity));
}

/// Performs initial setup of the particle system if wanted.
int CVParticleRenderFilter::Process(CVPipeline * pipe)
{
	if (!particleEmitterEntity)
	{
		// Create it
		particleEmitterEntity = MapMan.CreateEntity("ParticleRenderer", NULL, TexMan.GetTexture("NULL"));
		// Attach a particle-emitter.
		particleSystem = new Sparks(particleEmitterEntity);
		Graphics.QueueMessage(new GMRegisterParticleSystem(particleSystem, true));
	}
	if (this->color->HasChanged())
	{
		Vector3f newColor = color->GetVec3f();
		Graphics.QueueMessage(new GMSetParticleSystem(this->particleSystem, GT_PARTICLE_INITIAL_COLOR, newColor));
	}
	if (this->emissionVelocity->HasChanged())
	{
		float newVel = emissionVelocity->GetFloat();
		Graphics.QueueMessage(new GMSetParticleSystem(this->particleSystem, GT_PARTICLE_EMISSION_VEOCITY, newVel));
	}
	if (this->particleSize->HasChanged())
	{
		Graphics.QueueMessage(new GMSetParticleSystem(this->particleSystem, GT_PARTICLE_SCALE, particleSize->GetFloat()));
	}
	if (instancedRendering->HasChanged())
		Graphics.QueueMessage(new GMSetParticleSystem(particleSystem, GT_USE_INSTANCED_RENDERING, instancedRendering->GetBool()));
	if (emissionsPerSecond->HasChanged())
	{
		Graphics.QueueMessage(new GMSetParticleSystem(particleSystem, GT_EMISSIONS_PER_SECOND, emissionsPerSecond->GetInt()));
	}
	if (particleLifetime->HasChanged())
	{
		Graphics.QueueMessage(new GMSetParticleSystem(particleSystem, GT_PARTICLE_LIFE_TIME, particleLifetime->GetFloat()));
	}
	if (textureIndex->HasChanged())
	{
		// Get next te
		List<String> particles;
		String dir = "img/Particles/";
		GetFilesInDirectory(dir, particles);
		for (int i = 0; i < particles.Size(); ++i)
		{
			String & p = particles[i];
			if (!p.Contains(".png"))
			{
				particles.RemoveIndex(i);
				--i;
				continue;
			}
			p = dir + p;
		}
		
		int targetTextureIndex = (AbsoluteValue(textureIndex->GetInt())) % particles.Size();
		if (particles.Size())
		{
			String newParticleTextureSource = particles[targetTextureIndex];
			Graphics.QueueMessage(new GMSetParticleSystem(particleSystem, GT_PARTICLE_TEXTURE, newParticleTextureSource));
		}
	}
	return 0;
}

/// o.o
void CVParticleRenderFilter::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg.Contains("SetParticleBlendEquation"))
			{
				String mode = msg.Tokenize(":")[1];
				if (mode == "Additive")
					GraphicsMan.QueueMessage(new GMSetParticleSystem(particleSystem, GT_BLEND_EQUATION, GL_FUNC_ADD));
				else if (mode == "Subtractive")
					GraphicsMan.QueueMessage(new GMSetParticleSystem(particleSystem, GT_BLEND_EQUATION, GL_FUNC_REVERSE_SUBTRACT));
				else
					assert(false);
			}
			break;	
		}
	}
}



CVContourParticles::CVContourParticles()
: CVParticleRenderFilter(CVFilterID::CONTOUR_PARTICLES)
{
}
int CVContourParticles::Process(CVPipeline * pipe)
{
	CVParticleRenderFilter::Process(pipe);
	// Move it to the center of the contour..?
	if (pipe->contours.Size() == 0)
	{
		// Hide it!
		if (!particleSystem->emissionPaused)
			Graphics.QueueMessage(new GMPauseEmission(particleEmitterEntity));
		return CVReturnType::RENDER;
	}
	else 
	{
		if (particleSystem->emissionPaused)
			Graphics.QueueMessage(new GMResumeEmission(particleEmitterEntity));
	}	
//	pipe->cvContours

	Contour & contour = pipe->contours[0];

	Contour filteredContour;
	/// Remove each 3 out of 4 points, to get better results when rendering later on.
	int numPoints = contour.points.Size();
	int base = 5;
	for (int i = 0; i < numPoints; ++i)
	{
		if (i % base == 0)
			filteredContour.points.Add(contour.points[i]);
	}

	// Convert contour-coordinates to the screen-space being used (-halfWidth, +halfWidth)
	for (int i = 0; i < filteredContour.points.Size(); ++i)
	{
		Vector3f & point = filteredContour.points[i];
		point = pipe->InputSpaceToWorldSpace(point);
//		point.x -= pipe->initialInput->cols * 0.5f;
//		point.y = pipe->initialInput->rows * 0.5f - point.y;
	}

	Vector3f position = contour.centerOfMass;
	// half window space?
	position = pipe->InputSpaceToWorldSpace(position);
//	position.x -= pipe->initialInput->cols * 0.5;
//	position.y = pipe->initialInput->rows * 0.5 - position.y;

	Graphics.QueueMessage(new GMSetParticleEmitter(particleSystem, filteredContour));
	Physics.QueueMessage(new PMSetEntity(particleEmitterEntity, PT_SET_POSITION, position));
	
	// Update its "form" using the contour?

	// Update any variables to its attached particle emitter?

	return CVReturnType::RENDER;
}



CVFingerParticles::CVFingerParticles()
: CVParticleRenderFilter(CVFilterID::FINGER_PARTICLES)
{
	particleEmitterEntity = NULL;
}

int CVFingerParticles::Process(CVPipeline * pipe)
{
	CVParticleRenderFilter::Process(pipe);
	// Move it to the center of the contour..?
	if (pipe->hands.Size() == 0)
	{
		// Hide it!
		if (!particleSystem->emissionPaused)
			Graphics.QueueMessage(new GMPauseEmission(particleEmitterEntity));
		return CVReturnType::RENDER;
	}
	else 
	{
		if (particleSystem->emissionPaused)
			Graphics.QueueMessage(new GMResumeEmission(particleEmitterEntity));
	}	

	CVHand & hand = pipe->hands[0];
	
	// For each finger.. create an emitter
	List<ParticleEmitter*> newEmitters;
	Vector2f handCenter = hand.center;
	handCenter = pipe->InputSpaceToWorldSpace(handCenter);
//	handCenter.x -= pipe->initialInput->cols * 0.5f;
//	handCenter.y = pipe->initialInput->rows * 0.5 - handCenter.y;

	for (int i = 0; i < hand.fingers.Size(); ++i)
	{
		CVFinger finger = hand.fingers[i];
		Vector2f fingerPos = finger.point;
		fingerPos = pipe->InputSpaceToWorldSpace(fingerPos);
		// Recalculate position..
	//	fingerPos.x -= pipe->initialInput->cols * 0.5f;
	//	fingerPos.y = pipe->initialInput->rows * 0.5f - fingerPos.y;
		
		ParticleEmitter * emitter = new ParticleEmitter(fingerPos, fingerPos - handCenter);
		newEmitters.Add(emitter);

	}

	
	Graphics.QueueMessage(new GMSetParticleEmitter(particleSystem, newEmitters));
	Physics.QueueMessage(new PMSetEntity(particleEmitterEntity, PT_SET_POSITION, handCenter));
	
	// Update its "form" using the contour?

	// Update any variables to its attached particle emitter?

	return CVReturnType::RENDER;
}


CVHandParticles::CVHandParticles()
: CVParticleRenderFilter(CVFilterID::HAND_PARTICLES)
{
	particleEmitterEntity = NULL;
}
int CVHandParticles::Process(CVPipeline * pipe)
{
	CVParticleRenderFilter::Process(pipe);

	// Move it to the center of the contour..?
	if (pipe->hands.Size() == 0)
	{
		// Hide it!
		if (!particleSystem->emissionPaused)
			Graphics.QueueMessage(new GMPauseEmission(particleEmitterEntity));
		return CVReturnType::RENDER;
	}
	else 
	{
		if (particleSystem->emissionPaused)
			Graphics.QueueMessage(new GMResumeEmission(particleEmitterEntity));
	}	

	CVHand & hand = pipe->hands[0];
	
	// For each finger.. create an emitter
	List<ParticleEmitter*> newEmitters;
	Vector3f handCenter = hand.center;
	handCenter = pipe->InputSpaceToWorldSpace(handCenter);
//	handCenter.x -= pipe->initialInput->cols * 0.5f;
//	handCenter.y = pipe->initialInput->rows * 0.5 - handCenter.y;

	{
		// Recalculate position..
		
		SparksEmitter * emitter = new SparksEmitter(handCenter + Vector3f(0,0,2));
		emitter->SetEmissionVelocity(this->emissionVelocity->GetFloat());
		emitter->SetColor(color->GetVec3f());
		newEmitters.Add(emitter);

	}

	
	Graphics.QueueMessage(new GMSetParticleEmitter(particleSystem, newEmitters));
	Physics.QueueMessage(new PMSetEntity(particleEmitterEntity, PT_SET_POSITION, handCenter));
	
	return CVReturnType::RENDER;
}


CVOFParticles::CVOFParticles()
: CVParticleRenderFilter(CVFilterID::OPTICAL_FLOW_PARTICLES)
{
	particleEmitterEntity = NULL;
}

int CVOFParticles::Process(CVPipeline * pipe)
{
	CVParticleRenderFilter::Process(pipe);

	// Create an emitter for each point?
	List<ParticleEmitter*> newEmitters;

	// Move it to the center of the contour..?
	for (int i = 0; i < pipe->opticalFlowPoints.Size(); ++i)
	{
		OpticalFlowPoint & point = pipe->opticalFlowPoints[i];
		Vector3f position = point.position;
		Vector3f worldSpacePosition = pipe->InputSpaceToWorldSpace(position);
		Vector3f dir = point.offset.NormalizedCopy();
		Vector4f color = GetColorForDirection(dir);
		ParticleEmitter * emitter = new ParticleEmitter(worldSpacePosition, dir);
		emitter->particlesPerSecond = 120.f;
//		emitter->SetScale(3.f);
		emitter->SetEmissionVelocity(point.offset.Length());
		emitter->SetColor(color);
		newEmitters.Add(emitter);
	}
	// Set them.
	Graphics.QueueMessage(new GMSetParticleEmitter(particleSystem, newEmitters));

	// Hide it!
	return CVReturnType::RENDER;
}

