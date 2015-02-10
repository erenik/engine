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
	Timer timer;
	timer.Start();
	ProcessParticles(timeInSeconds);
	timer.Stop();
	//	float particleProcessing, particleSpawning, particleBufferUpdate;
	FrameStats.particleProcessing = timer.GetMs();
	int timeInMs = timeInSeconds * 1000;
	timer.Start();
	SpawnNewParticles(timeInMs);
	timer.Stop();
	FrameStats.particleSpawning = timer.GetMs();
	// Update buffers to use when rendering.
	timer.Start();
	UpdateBuffers();
	timer.Stop();
	FrameStats.particleBufferUpdate = timer.GetMs();
}

/// Integrates all particles.
void ParticleSystem::ProcessParticles(float & timeInSeconds)
{
#ifdef SSE_PARTICLES
	/*
	/// Move/Process all alive particles
	for (int i = 0; i < aliveParticles; ++i)
	{
		positionsSSE[i] = _mm_add_ps(positionsSSE[i], _mm_mul_ps(sseTime, _mm_add_ps(velocitiesSSE[i], weather->globalWind.data)));
	}
	*/
#else // Not SSE_PARTICLES
	float velocityDecay = pow(0.55f, timeInSeconds);
	/// Move/Process all alive particles
	for (int i = 0; i < aliveParticles; ++i)
	{
		positions[i] += velocities[i] * timeInSeconds;
		velocities[i] *= velocityDecay;
		lifeDurations[i] += timeInSeconds;
		// If duration has elapsed life-time..
		if (lifeDurations[i] > lifeTimes[i])
		{
			int lastIndex = aliveParticles - 1;
			// Kill it, by moving in the last used data to replace it.
			positions[i] = positions[lastIndex];
			velocities[i] = velocities[lastIndex];
			lifeDurations[i] = lifeDurations[lastIndex];
			colors[i] = colors[lastIndex];
			lifeTimes[i] = lifeTimes[lastIndex];
			scales[i] = scales[lastIndex];

			// Decrement i so we don't skip processing of the one we moved back.
			--i;
			// Decrement alive particles.
			--aliveParticles;
		}
	}
#endif
}

/// Spawns new particles depending on which emitters are attached.
void ParticleSystem::SpawnNewParticles(int & timeInMs)
{
#ifdef SSE_PARTICLES
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
	
#else // Not SSE_PARTICLES

	float timeInSeconds = (timeInMs % 200) * 0.001f;
	/// Spawn new particles as wanted.
	for (int i = 0; i < emitters.Size(); ++i)
	{
		ParticleEmitter * emitter = emitters[i];
		emitter->elapsedDurationMs += timeInMs;
		// Spawn for max 0.2 seconds at a time.
		float timeInSecondsCulled = MinimumFloat(timeInSeconds, 0.2f);
		int particlesToEmit = emitter->ParticlesToEmit(timeInSeconds);
		emitter->Update();

		// Try and emit each particles that the emitter wants to emit.
		for (int j = 0; j < particlesToEmit; ++j)
		{
			// Grab free index.
			int freeIndex = aliveParticles;
			// Skip if reaching max.
			if (freeIndex >= this->maxParticles)
			{
//				std::cout<<"\nEmitter unable to spawn particle. Max particles reached.";
				break;
			}
			assert(false && "Fix GetNewParticle so that it supports the new Vector2f scale");
			//			emitter->GetNewParticle(positions[freeIndex], velocities[freeIndex], scales[freeIndex], lifeTimes[freeIndex], colors[freeIndex]);
			// Multiply velocity by our multiplier?
			Vector3f & velocity = velocities[freeIndex];
			velocity *= this->emissionVelocity;
			// Reset duration to 0 to signify that it is newly spawned.
			lifeDurations[freeIndex] = 0;
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
#endif
}	

/// Update buffers to use when rendering.
void ParticleSystem::UpdateBuffers()
{
#ifdef SSE_PARTICLES
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
	// Should be no update required.
#else // Not SSE_PARTICLES

#ifdef USE_SSE
	Vector4f colorMult;
	float m = 255.f;
	__m128 mul = _mm_load1_ps(&m);
#endif
	for (int i = 0; i < aliveParticles; ++i)
	{
		Vector3f & pos = positions[i];
		int index = i * 4;
		particlePositionSizeData[index] = pos[0];
		particlePositionSizeData[index+1] = pos[1];
		particlePositionSizeData[index+2] = pos[2];

		Vector4f & color = colors[i];
		// Multiply color?
#ifdef USE_SSE
		colorMult.data = _mm_mul_ps(color.data, mul);
		particleColorData[index] = colorMult.x;
		particleColorData[index+1] = colorMult.y;
		particleColorData[index+2] = colorMult.z;
		particleColorData[index+3] = colorMult.w;		
#else
		particleColorData[index] = color[0] * 255;
		particleColorData[index+1] = color[1] * 255;
		particleColorData[index+2] = color[2] * 255;
		particleColorData[index+3] = color[3] * 255;
#endif

//		int index2 = i * 2;
		particleLifeTimeDurationScaleData[index] = lifeTimes[i];
		particleLifeTimeDurationScaleData[index+1] = lifeDurations[i];
		particleLifeTimeDurationScaleData[index+2] = scales[i].x;
		particleLifeTimeDurationScaleData[index+3] = scales[i].y;
	}
#endif
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
   	if (!FetchTextures())
		return;
	if (useInstancedRendering && GL_VERSION_3_3_OR_HIGHER)
		RenderInstanced(graphicsState);
	else
		RenderOld(graphicsState);

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


/** Renders using instanced functions such as glDrawArraysInstanced and glVertexAttribDivisor, requiring GL versions
	3.1 and 3.3 respectively. Ensure these requirements are fulfilled before calling the function or the program will crash.
*/
void ParticleSystem::RenderInstanced(GraphicsState & graphicsState)
{
	/// Fetch Particle shader.
	Shader * shader = ShadeMan.GetShader("ParticleFlatColor");
	if (!shader)
	{
		std::cout<<"\nNo Particle shader available";
		return;
	}
	ShadeMan.SetActiveShader(shader);
	// Set projection and view matrices
	glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, graphicsState.projectionMatrixF.getPointer());
	glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, graphicsState.viewMatrixF.getPointer());
	Matrix4f modelMatrix;
	if (shader->uniformModelMatrix != -1)
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, modelMatrix.getPointer());

	Vector3f right = -graphicsState.camera->LeftVector();
	glUniform3f(shader->uniformCameraRightWorldSpace, right.x, right.y, right.z);
	Vector3f up = graphicsState.camera->UpVector();
	glUniform3f(shader->uniformCameraUpWorldSpace, up.x, up.y, up.z);
	glUniform2f(shader->uniformScale, scale.x, scale.y);

	// Set decay type.
	glUniform1i(shader->uniformParticleDecayAlphaWithLifeTime, decayAlphaWithLifeTime);

	if (false)
	{
		shader->PrintAttributes();
		shader->PrintUniforms();
	}

	// Set blend equation
	glBlendEquation(blendEquation);

	/// Based on the optimization level, will probably be pow(0.5, optimizationLevel);
    optimizationLevel = pow(0.5f, graphicsState.optimizationLevel);
    if (optimizationLevel == 0)
        return;
    assert(optimizationLevel > 0);
    /// Calculate particles to process based on the graphicsState's optimization level.
    particlesToProcess = (int) (optimizationLevel * maxParticles);


	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	
	/*/// For instanced particle rendering. Some buffers.
	billboardVertexBuffer = -1;
	particlePositionScaleBuffer = -1;
	particleColorBuffer = -1;
	*/
	if (billboardVertexBuffer == -1)
	{	
		billboardVertexBuffer = GLBuffers::New();
		// Buffer vertices once.
		glBindBuffer(GL_ARRAY_BUFFER, billboardVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	}
 
	// The VBO containing the positions and sizes of the particles
	if (particlePositionScaleBuffer == -1)
	{
		particlePositionScaleBuffer = GLBuffers::New();
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBindBuffer(GL_ARRAY_BUFFER, particlePositionScaleBuffer);
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	}
	if (particleLifeTimeDurationScaleBuffer == -1)
	{
		particleLifeTimeDurationScaleBuffer = GLBuffers::New();
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBindBuffer(GL_ARRAY_BUFFER, particleLifeTimeDurationScaleBuffer);
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	}
	// The VBO containing the colors of the particles
	if (particleColorBuffer == -1)
	{
		particleColorBuffer = GLBuffers::New();
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer);
#ifdef SSE_PARTICLES
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
#endif
	}		

	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU,
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming

#ifdef SSE_PARTICLES
	// Buffer the actual data.
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionScaleBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, positionsSSE);
 
	glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, colorsSSE);

	glBindBuffer(GL_ARRAY_BUFFER, particleLifeTimeDurationScaleBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, ldsSSE);

#else // Not SSE_PARTICLES
	// Buffer the actual data.
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionScaleBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, particlePositionSizeData);
 
	glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * 4 * sizeof(GLubyte), particleColorData);

	glBindBuffer(GL_ARRAY_BUFFER, particleLifeTimeDurationScaleBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, aliveParticles * sizeof(GLfloat) * 4, particleLifeTimeDurationScaleData);
#endif

	// Bind attribute index 0 to vertex positions, attribute index 1 to incoming UV coordinates and index 2 to Normals. 
//	glBindAttribLocation(shader->shaderProgram, 0, "in_VertexPosition");
//	glBindAttribLocation(shader->shaderProgram, 1, "in_ParticlePositionScale");
//	glBindAttribLocation(shader->shaderProgram, 2, "in_Color");

	CheckGLError("ParticleSystem::RenderInstanced - buffering the data");

	// 1rst attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, billboardVertexBuffer);
	glVertexAttribPointer(
		shader->attributeVertexPosition, // attribute. Must match the layout in the shader.
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
 
	// 2nd attribute buffer : positions of particles' centers
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionScaleBuffer);
	glVertexAttribPointer(
		shader->attributeParticlePositionScale, // attribute. Must match the layout in the shader.
		4, // size : x + y + z + size => 4
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glBindBuffer(GL_ARRAY_BUFFER, particleColorBuffer);
	glVertexAttribPointer(
		shader->attributeColor, // attribute. Must match the layout in the shader.
		4, // size : r + g + b + a => 4
#ifdef SSE_PARTICLES
		GL_FLOAT, // already in float, from 0 to 1.
#else
		GL_UNSIGNED_BYTE, // type
#endif
		GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0, // stride
		(void*)0 // array buffer offset
	);

	// 4th attribute buffer : life time and current duration of particles
	glBindBuffer(GL_ARRAY_BUFFER, particleLifeTimeDurationScaleBuffer);
	glVertexAttribPointer(
		shader->attributeParticleLifeTimeDurationScale, // attribute. Must match the layout in the shader.
		4, // size : x + y + z + size => 4
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);



	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor[0]ml
	glVertexAttribDivisor(shader->attributeVertexPosition, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(shader->attributeParticlePositionScale, 1); // positions : one per quad (its center) -> 1
	glVertexAttribDivisor(shader->attributeColor, 1); // color : one per quad -> 1
	glVertexAttribDivisor(shader->attributeParticleLifeTimeDurationScale, 1); // color : one per quad -> 1

	// Bind texture!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, diffuse->glid);
		
	// Fill-mode!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	// Additive blending
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	

 
	// Draw the particules !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, aliveParticles);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Reset the instancing/divisor attributes or shading will fail on other shaders after this!
	glVertexAttribDivisor(shader->attributeVertexPosition, 0); 
	glVertexAttribDivisor(shader->attributeParticlePositionScale, 0); 
	glVertexAttribDivisor(shader->attributeColor, 0); 
	glVertexAttribDivisor(shader->attributeParticleLifeTimeDurationScale, 0);

	CheckGLError("ParticleSystem::RenderInstanced");
}

void ParticleSystem::RenderOld(GraphicsState & graphicsState)
{
#ifdef SSE_PARTICLES
//		positionsSSE[i] = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
	
#else // Not SSE_PARTICLES
	/// Based on the optimization level, will probably be pow(0.5, optimizationLevel);
    optimizationLevel = pow(0.5f, graphicsState.optimizationLevel);
    if (optimizationLevel == 0)
        return;
    assert(optimizationLevel > 0);
    /// Calculate particles to process based on the graphicsState's optimization level.
    particlesToProcess = (int) (optimizationLevel * maxParticles);


    ShadeMan.SetActiveShader(0);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(graphicsState.projectionMatrixF.getPointer());
    glMatrixMode(GL_MODELVIEW);
    Matrix4f viewMatrix = graphicsState.viewMatrixF.getPointer();
    Matrix4f modelMatrix;
  //  if (relativeTo)
  //      modelMatrix = relativeTo->transformationMatrix;
    Matrix4f modelView = viewMatrix * modelMatrix;
    glLoadMatrixf(modelView.getPointer());
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// PointsU ONLU!
	if (pointsOnly)
	{
		glBegin(GL_POINTS);
		for (int i = 0; i < aliveParticles; ++i)
		{
			if (lifeDurations[i] >= lifeTimes[i])
				continue;
			glColor4f(colors[i][0], colors[i][1], colors[i][2], colors[i][3] * lifeDurations[i] / lifeTimes[i]);
			Vector3f & p = positions[i];
			glVertex3f(p[0], p[1], p[2]);
		}
		glEnd();
	}
	else 
	{
		/// Set mipmap level too?
		int value;
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &value);
		// 9987 9729
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, diffuse->glid);
		Vector3f leftBase, upBase, left, up;
		leftBase = graphicsState.camera->LeftVector() * particleSize;
		upBase = graphicsState.camera->UpVector() * particleSize;
		glBegin(GL_QUADS);
		float optimizedAlpha = 1 / optimizationLevel + 2.f;
		for (int i = 0; i < aliveParticles; ++i)
		{
			if (lifeDurations[i] >= lifeTimes[i])
				continue;
			float alpha = 0.75f * optimizedAlpha * colors[i][3] * 0.8f * pow((1.0f - lifeDurations[i] / lifeTimes[i]), 4);
			Vector4f & color = colors[i];
			glColor4f(color[0], color[1], color[2], alpha);
			// Making size equal throughout the duration, to differentiate it from the Exhaust particle system.
			float sizeRatio = .5f; // pow(lifeDuration[i]+1.0f, 2.0f);
		//	if (lifeDuration[i] > 1.0f)
		//		sizeRatio = pow(5.0f, lifeDuration[i]-1.0f);
			left = leftBase * sizeRatio;
			up = upBase * sizeRatio;
			Vector3f & p = positions[i];

			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(p[0] + left[0] + up[0], p[1] + left[1] + up[1], p[2] + left[2] + up[2]);

		//	glColor4f(colors[i][0] , colors[i][1] - 1.0f, colors[i][2] - 1.0f, colors[i][3] * lifeDuration[i] / lifeTime);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(p[0] + left[0] - up[0], p[1] + left[1] - up[1], p[2] + left[2] - up[2]);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(p[0] - left[0] - up[0], p[1] - left[1] - up[1], p[2] - left[2] - up[2]);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(p[0] - left[0] + up[0], p[1] - left[1] + up[1], p[2] - left[2] + up[2]);
		}
		glEnd();
	}
#endif
}


