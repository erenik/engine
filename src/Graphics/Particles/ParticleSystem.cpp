// Emil Hedemalm
// 2013-07-14

#include "ParticleSystem.h"
#include "TextureManager.h"

#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "GraphicsState.h"
#include "Graphics/GLBuffers.h"

#include "Graphics/Camera/Camera.h"

#include "Shader.h"
#include "ShaderManager.h"

#include "Graphics/FrameStatistics.h"

#include "File/LogFile.h"

#include "Model/Model.h"
#include "Model/ModelManager.h"
#include "Mesh/Mesh.h"

ParticleSystem::ParticleSystem(String type, bool emitWithEmittersOnly)
	: type(type), name("Undefined"), emitWithEmittersOnly(emitWithEmittersOnly)
{
	scale = Vector2f(1,1);
#ifdef SSE_PARTICLES
	positionsSSE = velocitiesSSE = colorsSSE = ldsSSE = NULL;
#else // Not SSE_PARTICLES

	// Data arrays.
    lifeDurations = NULL;
	lifeTimes = NULL;
    positions = NULL;
	velocities = NULL;
	colors = NULL;
	scales = NULL;

	// Rendering arrays.
	particlePositionSizeData = NULL;
	particleLifeTimeDurationScaleData = NULL;
	particleColorData = NULL;
#endif


	shaderName = "ParticleFlatColor";
	maxParticles = 100000;

    relativeTo = NULL;
    emissionPaused = false;
	particleSize = 1.0f;
	emissionRatio = 1.0f;
	registeredForRendering = false;

	/// For instanced particle rendering. Some buffers.
	billboardVertexBuffer = -1;
	particlePositionScaleBuffer = -1;
	particleColorBuffer = -1;
	particleLifeTimeDurationScaleBuffer = -1;

	useInstancedRendering = true;
	deleteEmittersOnDeletion = true;

	model = NULL;
	blendFuncDest = GL_ONE;
	// Default? https://www.khronos.org/opengles/sdk/docs/man/xhtml/glBlendEquation[0]ml
	blendEquation = GL_FUNC_ADD;
	initialized = false;
	diffuse = NULL;

	decayAlphaWithLifeTime = DecayType::LINEAR;
	pointsOnly = false;
    diffuse = NULL;
    particlesToProcess = maxParticles;
    emissionsPerSecond = 10000;
    maxRange = 1000;
	particleLifeTime = 5.0f;
	/// For global system, no entity is linked.
	relativeTo = NULL;
    maxRangeSq = maxRange * maxRange;
	emissionVelocity = 1.0f;
	aliveParticles = 0;
    color = Vector4f(0.1f, 0.5f, 0.4f, 1.0f);
	modelName = "sprite";
}

ParticleSystem::~ParticleSystem()
{
    std::cout<<"\nParticleSystem Destructor.....";
#define DELETE_ARRAY(a) {if (a) { delete[] a; a = NULL; } }

#ifdef SSE_PARTICLES
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
	DELETE_ARRAY(positionsSSE);
	DELETE_ARRAY(velocitiesSSE);
	DELETE_ARRAY(colorsSSE);
	DELETE_ARRAY(ldsSSE);
#else // Not SSE_PARTICLES
	DELETE_ARRAY(lifeDurations);
	DELETE_ARRAY(lifeTimes);
	DELETE_ARRAY(positions);
	DELETE_ARRAY(velocities);
	DELETE_ARRAY(colors);
	DELETE_ARRAY(scales);

	/// Arrays used for instanced rendering.
	DELETE_ARRAY(particlePositionSizeData);
	DELETE_ARRAY(particleLifeTimeDurationScaleData);
	DELETE_ARRAY(particleColorData);
#endif

	if (deleteEmittersOnDeletion)
		emitters.ClearAndDelete();
}

/// Sets default values. Calls AllocateArrays.
void ParticleSystem::Initialize()
{
	// Grab model.
	model = ModelMan.GetModel(modelName);
	if (!model)
	{
		LogGraphics("Unable to initialize ParticleSystem. Bad model.", ERROR);
		return;
	}
	// Bufferize model if needed.
	model->BufferizeIfNeeded();
	assert(model);
	// Allocate the particle data arrays.
	AllocateArrays();
	initialized = true;
}


/// Allocates them arrays!
void ParticleSystem::AllocateArrays()
{
#ifdef SSE_PARTICLES
	positionsSSE = new SSEVec[maxParticles];
	velocitiesSSE = new SSEVec[maxParticles];
	colorsSSE = new SSEVec[maxParticles];
	ldsSSE = new SSEVec[maxParticles]; // Lifetime, duration, scale.
#else // Not SSE_PARTICLES
	if (!lifeDurations)
	{
		lifeDurations = new float[maxParticles];
		lifeTimes = new float[maxParticles];
		positions = new Vector3f[maxParticles];
		velocities = new Vector3f[maxParticles];
		colors = new Vector4f[maxParticles];
		scales = new Vector2f[maxParticles];
	}
	/// Hybrid data array which holds 3 floats for positional (x,y,z) and 1 float for size or scale data.
	if (particlePositionSizeData == NULL)
	{
		particlePositionSizeData = new GLfloat[maxParticles * 4];
		particleColorData = new GLubyte[maxParticles * 4];
		particleLifeTimeDurationScaleData = new GLfloat[maxParticles * 4];
	}
#endif
}


void ParticleSystem::Process(float timeInSeconds)
{
	if (!initialized)
		Initialize();
	if (!initialized)
	{
		LogGraphics("Particle system failed to initialize", ERROR);
		return;
	}
	assert(initialized);
	Timer timer;
	timer.Start();
	ProcessParticles(timeInSeconds);
	timer.Stop();
	//	float particleProcessing, particleSpawning, particleBufferUpdate;
	FrameStats.particleProcessing = (float)timer.GetMs();
	int timeInMs = (int) (timeInSeconds * 1000);
	timer.Start();
	SpawnNewParticles(timeInMs);
	timer.Stop();
	FrameStats.particleSpawning = (float) timer.GetMs();
	// Update buffers to use when rendering.
	timer.Start();
	UpdateBuffers();
	timer.Stop();
	FrameStats.particleBufferUpdate = (float) timer.GetMs();
}

/// Integrates all particles.
void ParticleSystem::ProcessParticles(float & timeInSeconds)
{
	Timer timer;
	timer.Start();
#ifdef USE_SSE
	__m128 sseTime = _mm_load1_ps(&timeInSeconds);
#endif
	/// Move/Process all alive particles
//	const Vector3f wind = weather->globalWind;
	for (int i = 0; i < aliveParticles; ++i)
	{
#ifdef SSE_PARTICLES
		positionsSSE[i].data = _mm_add_ps(positionsSSE[i].data, _mm_mul_ps(sseTime, velocitiesSSE[i].data));
#else // Not SSE_PARTICLES
		assert(false && "Implement");
#endif // SSE_PARTICLES
	}
	timer.Stop();
	FrameStats.particleProcessingIntegrate += timer.GetMs();

	/// Make them older.
	timer.Start();
	for (int i = 0; i < aliveParticles; ++i)
	{
#ifdef SSE_PARTICLES
		ldsSSE[i].y += timeInSeconds;	
#else // Not SSE_PARTICLES
		// No velocity decay.
		lifeDurations[i] += timeInSeconds;
#endif // SSE_PARTICLES
	}
	timer.Stop();
	FrameStats.particleProcessingOldify = timer.GetMs();


	/// Look for dead particles and move them out of our way.
	timer.Start();
	for (int i = 0; i < aliveParticles; ++i)
	{
#ifdef SSE_PARTICLES
		if (ldsSSE[i].y > ldsSSE[i].x)
		{
			int lastIndex = aliveParticles - 1;
			positionsSSE[i] = positionsSSE[lastIndex];
			velocitiesSSE[i] = velocitiesSSE[lastIndex];
			colorsSSE[i] = colorsSSE[lastIndex];
			ldsSSE[i] = ldsSSE[lastIndex];
			// Decrement i so we don't skip processing of the one we moved back.
			--i;
			// Decrement alive particles.
			--aliveParticles;
		}			
#else
		assert(false && "Implement?");
#endif
	}
	timer.Stop();
	FrameStats.particleProcessingRedead += timer.GetMs();
}

/// Spawns new particles depending on which emitters are attached.
void ParticleSystem::SpawnNewParticles(int & timeInMs)
{
	if (emissionPaused)
		return;
	float timeInSeconds = (timeInMs % 200) * 0.001f;
	// Optionally add global emitter as in the PrecipitationSystem?
//	SpawnNewGlobal(timeInMs);

#ifdef SSE_PARTICLES
	__m128 sse;
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));	
	/// Spawn new particles as wanted.
	for (int i = 0; i < emitters.Size(); ++i)
	{
		ParticleEmitter * emitter = emitters[i];
		emitter->elapsedDurationMs += timeInMs;
		// Spawn for max 0.2 seconds at a time.
		float timeInSecondsCulled = MinimumFloat(timeInSeconds, 0.2f);
		int particlesToEmit = emitter->ParticlesToEmit(timeInSeconds);
		// Update emitters, e.g. if they require any special positions based on other entities.
		emitter->Update();
		// Try and emit each particles that the emitter wants to emit.
		for (int j = 0; j < particlesToEmit; ++j)
		{
			// Grab free index.
			int freeIndex = aliveParticles;
			// Skip if reaching max.
			if (freeIndex >= this->maxParticles)
			{
				LogGraphics("Emitter unable to spawn particle. Max particles reached.", DEBUG);
				break;
			}
			// o.o
#ifdef USE_SSE
			emitter->GetNewParticle(positionsSSE[freeIndex], velocitiesSSE[freeIndex], colorsSSE[freeIndex], ldsSSE[freeIndex]);
			/// Translate the particle as if the emitter has been emitting throughout the whole duration.
			float factor = j / (float)particlesToEmit;
			sse = _mm_load1_ps(&factor);
			positionsSSE[freeIndex].data = _mm_add_ps(positionsSSE[freeIndex].data, _mm_mul_ps(velocitiesSSE[freeIndex].data, sse));
//			std::cout<<"\nNew particle at position: "<<positionsSSE[freeIndex].x<<" "<<positionsSSE[freeIndex].y;
#else
			emitter->GetNewParticle(positions[freeIndex], velocities[freeIndex], scales[freeIndex], lifeTimes[freeIndex], colors[freeIndex]);
			// Multiply velocity by our multiplier?
			Vector3f & velocity = velocities[freeIndex];
			velocity *= this->emissionVelocity;
			// Reset duration to 0 to signify that it is newly spawned.
			lifeDurations[freeIndex] = 0;
#endif
			// Increment amount of living particles.
			++aliveParticles;
		}
		/// Check if the emitter should be deleted after some time.
		if ((emitter->deleteAfterMs > 0 && emitter->elapsedDurationMs > emitter->deleteAfterMs) ||
			emitter->instantaneous)
		{
			// If so, delete it then!
			emitters.Remove(emitter);
			--i;
			delete emitter;
		}
	}
	/// If no emitters are present and default emitter is enabled..
	if (emitters.Size() == 0 && !this->emitWithEmittersOnly)
	{
		/// Prepare some data
		int spawnedThisFrame = 0;
		int toSpawnThisFrameTotal = (int)floor(emissionsPerSecond * timeInSeconds * emissionRatio+0.5f);
		int toSpawn = toSpawnThisFrameTotal;
	
		for (int i = 0; i < toSpawnThisFrameTotal; ++i)
		{
		}
	}
	
#else // Not SSE_PARTICLES
#endif
}	

/// Update buffers to use when rendering.
void ParticleSystem::UpdateBuffers()
{
	// The VBO containing the positions and sizes of the particles
	if (particlePositionScaleBuffer == -1)
	{
		particlePositionScaleBuffer = GLBuffers::New();
		graphicsState->BindVertexArrayBuffer(particlePositionScaleBuffer);
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	}
	if (particleLifeTimeDurationScaleBuffer == -1)
	{
		particleLifeTimeDurationScaleBuffer = GLBuffers::New();
		graphicsState->BindVertexArrayBuffer(particleLifeTimeDurationScaleBuffer);
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	}
	// The VBO containing the colors of the particles
	if (particleColorBuffer == -1)
	{
		particleColorBuffer = GLBuffers::New();
		graphicsState->BindVertexArrayBuffer(particleColorBuffer);
#ifdef SSE_PARTICLES
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
#endif
	}	

#ifdef SSE_PARTICLES
	// Buffer the actual data.
	graphicsState->BindVertexArrayBuffer(particlePositionScaleBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, positionsSSE);
 
	graphicsState->BindVertexArrayBuffer(particleColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, colorsSSE);

	graphicsState->BindVertexArrayBuffer(particleLifeTimeDurationScaleBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, ldsSSE);
#endif
	CheckGLError("ParticleSystem::UpdateBuffers");
}

/// Clears all existing particles.
void ParticleSystem::ClearParticles()
{
	this->aliveParticles = 0;
}


/** Fetches textures required for rendering. Should only be called from Render() or elsewhere in the render thread.
	Returns false if it failed to fetch any textures, meaning they may still be NULL.
*/
bool ParticleSystem::FetchTextures()
{
	if (!diffuse)
		diffuse = TexMan.GetTextureBySource("Particles/Particle");
	if (!diffuse)
	{
		std::cout<<"\nUnable fetch texture in ParticleSystem::FetchTextures";
		return false;
	}
	// Bufferize as needed.
	if (diffuse->glid == -1)
		diffuse->Bufferize();
	return true;
}

void ParticleSystem::Render(GraphicsState & graphicsState)
{
	CheckGLError("Before ParticleSystem::Render");
   	if (!FetchTextures())
		return;
	if (useInstancedRendering && GL_VERSION_3_3_OR_HIGHER)
		RenderInstanced(graphicsState);
	else
		RenderOld(graphicsState);
	CheckGLError("After ParticleSystem::Render");
}

void ParticleSystem::PrintData(){
    assert(false);
}
void ParticleSystem::AttachTo(Entity * entity, ConstMat4r relativePosition){
    assert(false);
}
void ParticleSystem::SetPosition(ConstMat4r relativePosition){
    assert(false);
}

void ParticleSystem::PauseEmission(){
    emissionPaused = true;
}
void ParticleSystem::ResumeEmission(){
    emissionPaused = false;
}

void ParticleSystem::SetColor(const Vector4f & icolor){
    color = icolor;
}

/// Sets the emitter to be a contour. Default before calling this is a point or a plane.
void ParticleSystem::SetEmitter(const Contour & contour)
{
	// Delete the old emitter
	std::cout<<"\nSetting new contour as emitter, with position "<<contour.centerOfMass[0]<<", "<<contour.centerOfMass[1]<<" and points: "<<contour.points.Size();	
	emitters.ClearAndDelete();
	ParticleEmitter * newEmitter = new ParticleEmitter(contour);
	emitters.Add(newEmitter);
}


void ParticleSystem::SetEmitter(List<ParticleEmitter*> newEmitters)
{
	// Delete the old emitter
	emitters.ClearAndDelete();
	emitters = newEmitters;
	for (int i = 0; i < newEmitters.Size(); ++i)
	{
		ParticleEmitter * emitter = newEmitters[i];
		emitter->AttachTo(this);
	}
}

/// Sets emission velocity. This will be forward to any attached emitters as well.
void ParticleSystem::SetEmissionVelocity(float vel)
{
	emissionVelocity = vel;
	for (int i = 0; i < emitters.Size(); ++i)
	{
		ParticleEmitter * emitter = emitters[i];
		emitter->SetEmissionVelocity(emissionVelocity);
	}
}

/// For setting alpha decay by life time.
void ParticleSystem::SetAlphaDecay(int decayType)
{
	decayAlphaWithLifeTime = decayType;
}

/// For setting specific uniforms after most other properties have been set up.
void ParticleSystem::SetUniforms()
{
	Shader * shader = ActiveShader();
	// Set decay type.
	if (shader->uniformParticleDecayAlphaWithLifeTime != -1)
		glUniform1i(shader->uniformParticleDecayAlphaWithLifeTime, decayAlphaWithLifeTime);

	Vector3f right = -graphicsState->camera->LeftVector();
	if (shader->uniformCameraRightWorldSpace != -1)
	{
		glUniform3f(shader->uniformCameraRightWorldSpace, right.x, right.y, right.z);
		Vector3f up = graphicsState->camera->UpVector();
		glUniform3f(shader->uniformCameraUpWorldSpace, up.x, up.y, up.z);
	}
	if (shader->uniformScale != -1)
		glUniform2f(shader->uniformScale, scale.x, scale.y);

}



/** Renders using instanced functions such as glDrawArraysInstanced and glVertexAttribDivisor, requiring GL versions
	3.1 and 3.3 respectively. Ensure these requirements are fulfilled before calling the function or the program will crash.
*/
void ParticleSystem::RenderInstanced(GraphicsState & graphicsState)
{
#define LogParticleSystem(text, errorLevel) LogGraphics(name+": "+text, errorLevel);
#define CheckGLErrorParticle(text) CheckGLError(type+": "+text);
	/// Fetch Particle shader.
	Shader * shader = ShadeMan.GetShader(shaderName);
	if (!shader)
	{
		std::cout<<"\nNo Particle shader available";
		return;
	}
	LogGraphics("ParticleSystem::RenderInstanced", EXTENSIVE_DEBUG);
	shader = ShadeMan.SetActiveShader(shader);
	if (!shader)
	{
		LogGraphics("Bad shader "+shaderName, ERROR);
		return;
	}
	// Set projection and view matrices
	if (shader->uniformViewProjectionMatrix != -1)
	{
		Matrix4f viewProjection = graphicsState.camera->ViewProjectionF();
		glUniformMatrix4fv(shader->uniformViewProjectionMatrix, 1, false, viewProjection.getPointer());
	}
	/// Obsolete, but may still want this to work for a while..
	else 
	{
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.camera->ViewMatrix4f().getPointer());	
		glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.camera->ProjectionMatrix4f().getPointer());	
	}
	Matrix4f modelMatrix;
	if (shader->uniformModelMatrix != -1)
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, modelMatrix.getPointer());

	// Set uniforms
	SetUniforms();
	CheckGLError("ParticleSystem::RenderInstanced - set uniforms");


	if (false)
	{
		shader->PrintAttributes();
		shader->PrintUniforms();
	}
	CheckGLErrorParticle("ParticleSystem::RenderInstanced - uniforms");

	// Set blend equation
	glBlendEquation(blendEquation);

	// Enable the necessary attributes?
	assert(model);
	if (!model)
	{
		LogGraphics("Lacking valid model in ParticleSystem::RenderInstanced", ERROR);
		return;
	}
	// Set up model properties first?
	model->mesh->BindVertexBuffer();
	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor[0]ml
	/// Mesh-data, re-use it all.



	if (shader->attributePosition == -1)
	{
		LogGraphics("Particle system shader "+shader->name+" lacking position attribute, yo?", ERROR);
		return;
	}
	glEnableVertexAttribArray(shader->attributePosition);
	glVertexAttribDivisor(shader->attributePosition, 0); 
	if (shader->attributeNormal != -1)
	{
		glEnableVertexAttribArray(shader->attributeNormal);
		glVertexAttribDivisor(shader->attributeNormal, 0); 
	}
	if (shader->attributeUV != -1)
	{
		glEnableVertexAttribArray(shader->attributeUV);
		glVertexAttribDivisor(shader->attributeUV, 0); 
	}

	/// Bind vertex array/attrib pointers.
	// 1rst attribute buffer : vertices
	if (shader->attributeParticlePositionScale != -1 && particlePositionScaleBuffer != -1)
	{
		graphicsState.BindVertexArrayBuffer(particlePositionScaleBuffer);
		glEnableVertexAttribArray(shader->attributeParticlePositionScale);
		glVertexAttribPointer(shader->attributeParticlePositionScale, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glVertexAttribDivisor(shader->attributeParticlePositionScale, 1); 
	}
	else {
		glDisableVertexAttribArray(shader->attributeParticlePositionScale);
	}
	if (shader->attributeColor != -1 && particleColorBuffer != -1)
	{
		graphicsState.BindVertexArrayBuffer(particleColorBuffer);
		glEnableVertexAttribArray(shader->attributeColor);
		glVertexAttribPointer(shader->attributeColor, 4, GL_FLOAT, GL_TRUE, 0, (void*)0);
		glVertexAttribDivisor(shader->attributeColor, 1);
	}
	else {	
		glDisableVertexAttribArray(shader->attributeColor);
	}

	/// Set up life time, duration and scale (used for rain).
	if (shader->attributeParticleLifeTimeDurationScale != -1 && particleLifeTimeDurationScaleBuffer != -1)
	{
		graphicsState.BindVertexArrayBuffer(particleLifeTimeDurationScaleBuffer);
		glEnableVertexAttribArray(shader->attributeParticleLifeTimeDurationScale);
		glVertexAttribPointer(shader->attributeParticleLifeTimeDurationScale, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glVertexAttribDivisor(shader->attributeParticleLifeTimeDurationScale, 1); 
	}
	else {
		glDisableVertexAttribArray(shader->attributeParticleLifeTimeDurationScale);
	}
	CheckGLErrorParticle("ParticleSystem::RenderInstanced - bound attribs");

	// Bind texture!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, diffuse->glid);
		
	// Fill-mode!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// Additive blending
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glBlendFunc(GL_SRC_ALPHA, blendFuncDest);
	
	// Enable depth-test?
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE); // Disable depth-write though, or other alpha-effects afterwards will fail.
 
	Mesh * mesh = model->GetTriangulatedMesh();
	assert(mesh->vertexDataCount < 300000);

	CheckGLError("ParticleSystem::RenderInstanced - before draw arrays");

	// Draw the particules !
	glDrawArraysInstanced(GL_TRIANGLES, 0, mesh->vertexDataCount, aliveParticles);

	graphicsState.BindVertexArrayBuffer(0);
	CheckGLErrorParticle("ParticleSystem::RenderInstanced - post draw");

	// Reset the instancing/divisor attributes or shading will fail on other shaders after this!
	if (shader->attributeParticlePositionScale != -1)
	{
		glVertexAttribDivisor(shader->attributeParticlePositionScale, 0); 
		glDisableVertexAttribArray(shader->attributeParticlePositionScale);
	}
	if (shader->attributeColor != -1)
	{
		glVertexAttribDivisor(shader->attributeColor, 0); 
		glDisableVertexAttribArray(shader->attributeColor);
	}
	// Same for optional arguments.
	if (shader->attributeParticleLifeTimeDurationScale != -1)
	{
		glVertexAttribDivisor(shader->attributeParticleLifeTimeDurationScale, 0);
		glDisableVertexAttribArray(shader->attributeParticleLifeTimeDurationScale);
	}
	
	CheckGLErrorParticle("ParticleSystem::RenderInstanced - unbind");
}

void ParticleSystem::RenderOld(GraphicsState & graphicsState)
{
	assert(false && "Deprecated.");
#ifdef SSE_PARTICLES
#else // Not SSE_PARTICLES
#endif
}


