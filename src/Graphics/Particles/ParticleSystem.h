// Emil Hedemalm
// 2013-07-14

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

class GraphicsState;
class Entity;
class Texture;
class Model;

#include "ParticleEmitter.h"
#include <String/AEString.h>
#include "Graphics/OpenGL.h"

class ParticleEmitter;


namespace DecayType {
enum {
	NONE,
	LINEAR,
	QUADRATIC,
	CUBIC
	};	
};

#define ParticleSystemSharedPtr std::shared_ptr<ParticleSystem>
#define ParticleSystemWeakPtr std::weak_ptr<ParticleSystem>

class alignas(16) ParticleSystem {
	friend class GraphicsManager;
	friend class GMAttachParticleEmitter;
	friend class GMDetachParticleEmitter;

	std::weak_ptr<ParticleSystem> selfPtr;
public:
	// Use only for sub-classing.
	ParticleSystem(String type, bool emitWithEmittersOnly);

	template<typename... Args>
	static std::shared_ptr<ParticleSystem> NewParticleSystem(Args... args);
	std::shared_ptr<ParticleSystem> GetSharedPtr();

    virtual ~ParticleSystem();

	/// Sets default values. Calls AllocateArrays.
	virtual void Initialize();
	/// Allocates them arrays!
	virtual void AllocateArrays();

	/** Processes the particles within the system, moving them wherever they are going or performing whatever transformation they should ungergo.
		If emitters are attached to this system they may also automatically create or emit new particles.

		Relies on ProcessParticles() and SpawnParticles() functions.
	*/
    virtual void Process(GraphicsState * graphicsState, float timeInSeconds);
	/// Integrates all particles.
	virtual void ProcessParticles(float & timeInSeconds);
	/// Spawns new particles depending on which emitters are attached.
	virtual void SpawnNewParticles(int & timeInMs);
	/// Update buffers to use when rendering.
	virtual void UpdateBuffers(GraphicsState* graphicsState);
	/// Clears all existing particles.
	virtual void ClearParticles(); 

	/** Fetches textures required for rendering. Should only be called from Render() or elsewhere in the render thread.
		Returns false if it failed to fetch any textures, meaning they may still be NULL.
	*/
	virtual bool FetchTextures();
	/** Finds and bufferizes model if needed. Should only be called from Render() or elsewherei n render thread.
	Returns false if it failed to find the model, meaning they may still be null. */
	virtual bool FetchModel();
	/// Renders the particles
    virtual void Render(GraphicsState * graphicsState);
    virtual void PrintData();
    virtual void AttachTo(EntitySharedPtr entity, ConstMat4r relativePosition);
    virtual void SetPosition(ConstMat4r  relativePosition);
    /// Emission control.
    virtual void PauseEmission();
    virtual void ResumeEmission();
    virtual void SetColor(const Vector4f & color);


	virtual void AddEmitter(std::shared_ptr<ParticleEmitter> particleEmitter);

	/// Sets the emitter to be a contour. Default before calling this is a point or a plane.
	virtual void SetEmitter(const Contour & contour);
	virtual void SetEmitter(List<std::shared_ptr<ParticleEmitter>> newEmitters);

	/// Sets emission velocity. This will be forward to any attached emitters as well.
	void SetEmissionVelocity(float vel);

	/// For setting alpha decay by life time.
	void SetAlphaDecay(int decayType);

	/// Name & type
	String type;
	String name;

	/// Time control
	bool emissionPaused;

	/// Applied to all particles.
	Vector2f scale;

	/// Model used for rendering the particles.
	String modelName;
	Model * model;
	/// Settings
	bool pointsOnly;
	Texture * diffuse;
	int maxParticles;
	// Per emitter..!
	int emissionsPerSecond;
	// To easily toggle between, say, 1.0 and 0.0, amount of emitted particles 
	float emissionRatio; 
	// Relative emission velocity from 0.0 to 1.0
	float emissionVelocity;

	/// Positioning
	/// If this is NULL, the relative model matrix will be in world coordinates. Simple as that!
	EntitySharedPtr relativeTo;
	Matrix4f relativeModelMatrix;
	/// Before respawning/dying
	float maxRange, maxRangeSq;
	float particleLifeTime;
	/// Initial scale/size.
	float particleSize;

	/// Primary colors
	Vector4f color;

	/// Setting for quality control stuffs. True by default.. maybe
	bool useInstancedRendering;
	/// If false, may opt to use default emitter when no emitters are present. Default is assigned in constructor.
	bool emitWithEmittersOnly;
	/// If true, will delete attached emitters on deletion. True by default.
	bool deleteEmittersOnDeletion;

	/// E.g. GL_ONE, GL_ONE_MINUS_SRC_ALPHA
	int blendFuncDest;
	// Should be one of GL_FUNC_ADD, GL_FUNC_SUBTRACT and GL_FUNC_REVERSE_SUBTRACT. Defualt is GL_FUNC_ADD.
	int blendEquation;
protected:
	/// For setting specific uniforms after most other properties have been set up.
	virtual void SetUniforms(GraphicsState* graphicsState);
	/// By default, "ParticleFlatColor"
	String shaderName;
	bool initialized;
	/** Renders using instanced functions such as glDrawArraysInstanced and glVertexAttribDivisor, requiring GL versions
		3.1 and 3.3 respectively. Ensure these requirements are fulfilled before calling the function or the program will crash.
	*/
	virtual void RenderInstanced(GraphicsState * graphicsState);
	virtual void RenderOld(GraphicsState * graphicsState);

	/// Number of alive particles.
	int aliveParticles;

    /// Based on the optimization level, will probably be pow(0.5, optimizationLevel);
    float optimizationLevel;
    /// Based on maxParticles and the current optimization level.
    int particlesToProcess;
#ifdef USE_SSE
#define SSE_PARTICLES
#ifdef SSE_PARTICLES
	SSEVec * positionsSSE,
		* velocitiesSSE,
		* colorsSSE,
		* ldsSSE // Lifetime, duration, scale.
		;
#endif // SSE_PARTICLES
#else // USE_SSE
	/// Raw data
    float * lifeDurations, * lifeTimes;
    Vector3f * positions;
    Vector3f * velocities;
    Vector4f * colors;
	Vector2f * scales;

	/// Data buffers used for instanced rendering.
	GLfloat * particlePositionSizeData;
	GLubyte * particleColorData;
	GLfloat * particleLifeTimeDurationScaleData;  // Total life time of this particle + duration lived so far. Used to manipulate other data depending on shader options.
#endif

	/// Settings in shader.
	int decayAlphaWithLifeTime;

    // For getting new spawn positions
    List<std::shared_ptr<ParticleEmitter>> emitters;

	/// For instanced particle rendering. Some buffers.
	GLuint billboardVertexBuffer;
	GLuint particlePositionScaleBuffer;
	GLuint particleColorBuffer;
	GLuint particleLifeTimeDurationScaleBuffer;

private:
	bool registeredForRendering;
};

#endif

