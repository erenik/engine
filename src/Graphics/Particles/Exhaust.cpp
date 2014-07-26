// Emil Hedemalm
// 2013-07-14

#include "Exhaust.h"
#include "GL/glew.h"
#include "GraphicsState.h"
#include "Entity/Entity.h"
#include "Graphics/Camera/Camera.h"
#include "TextureManager.h"
#include "../GraphicsManager.h"
#include "../FrameStatistics.h"

Exhaust::Exhaust(Entity * reference)
: ParticleSystem("PointEmitter")
{
    pointsOnly = false;
    diffuse = NULL;
    maxParticles = 10000;
    particlesToProcess = maxParticles;
    emissionsPerSecond = 10000;
    maxRange = 1000;
    maxLifeTime = 5.0f;
    /// Wosh!
    relativeTo = reference;

    maxRangeSq = maxRange * maxRange;
    lifeDuration = new float[maxParticles];
	lifeTime = new float[maxParticles];
    positions = new Vector3f[maxParticles];
    velocities = new Vector3f[maxParticles];
    colors = new Vector4f[maxParticles];
	diffuse = TexMan.GetTextureBySource("Particles/Particle");
	assert(diffuse);
	Graphics.QueueMessage(new GMBufferTexture(diffuse));
    primaryVelocity = 200.0f;
    sideVelocityRange = 20.0f;
	emissionVelocity = 1.0f;

    for(int i = 0; i < maxParticles; ++i){
        velocities[i].x += sideVelocityRange * (rand()%201-100) * 0.01f;
        velocities[i].y += sideVelocityRange * (rand()%201-100) * 0.01f;
        velocities[i].z = primaryVelocity * (rand()%81 + 20) * 0.01f;
        lifeDuration[i] = maxLifeTime;
		lifeTime[i] = maxLifeTime;
        colors[i] = color;
    }
    color = Vector4f(0.1f, 0.2f, 0.4f, 1.0f);
}
Exhaust::~Exhaust()
{
    std::cout<<"\nExhaust Destructor.....";
}
void Exhaust::Process(float timeInSeconds)
{
    /// Prepare some data
    int spawnedThisFrame = 0;
	int toSpawnThisFrameTotal = (int)floor(emissionsPerSecond * timeInSeconds * emissionRatio+0.5f);
    int toSpawn = toSpawnThisFrameTotal;
    Matrix4f & modelMatrix = relativeTo->transformationMatrix;
    Matrix4d & rotationMatrix = relativeTo->rotationMatrix;
    Vector3f velocity;
	float velocityDecay = pow(0.55f, timeInSeconds);
	Vector3f newPosition = modelMatrix.Product(relativePosition);
	Vector3f newDirection = rotationMatrix.Product(Vector4f(0,0,1,0));

    /// Process and spawn new particles as needed
    for(int i = 0; i < particlesToProcess; ++i){
        if (lifeDuration[i] > lifeTime[i]){
            if (emissionPaused)
                continue;
            if (toSpawn){
				float positionRatio = ((float)toSpawn) / toSpawnThisFrameTotal;
				positionRatio = rand() % 100 * 0.01f;
                positions[i] = positionRatio * newPosition + (1 - positionRatio) * previousPosition;
		//		std::cout<<"\npositionRatio: "<<positionRatio;
                velocity = Vector3f();
				velocity.x += sideVelocityRange * (rand()%201-100) * 0.01f;
                velocity.y += sideVelocityRange * (rand()%201-100) * 0.01f;
                velocity.Normalize();
                velocity *= sideVelocityRange * (rand()%101) * 0.01f;
                velocity.z = primaryVelocity * (rand()%81 + 20) * 0.01f;
                velocity = rotationMatrix.Product(velocity);
				velocity *= emissionVelocity;
				/// Add vehicle velocity to total velocity.
			//	velocity = relativeTo->Velocity() * 0.5f;
			//	velocity = Vector3f();
                velocities[i] = velocity;
                /// Set life-time
                lifeTime[i] = maxLifeTime * (rand()&1001+1) * 0.001f;
                if (i%10 >= 7){
                    lifeTime[i] *= 0.1f;
                }
				else if (i%10 >= 5){
					lifeTime[i] *= 0.5f;
				}
				lifeTime[i] *= emissionRatio;
				lifeDuration[i] = 0;
                colors[i] = color;
                --toSpawn;
                ++spawnedThisFrame;
            }
        }
		/// Move alive particles
		positions[i] += velocities[i] * timeInSeconds;
		velocities[i] *= velocityDecay;
		lifeDuration[i] += timeInSeconds;
    }
	previousPosition = newPosition;
	previousDirection = newDirection;
}

void Exhaust::Render(GraphicsState * graphicsState)
{
    /// Based on the optimization level, will probably be pow(0.5, optimizationLevel);
    optimizationLevel = pow(0.5f, graphicsState->optimizationLevel);
    if (optimizationLevel == 0)
        return;
    assert(optimizationLevel > 0);
    /// Calculate particles to process based on the graphicsState's optimization level.
    particlesToProcess = (int) (optimizationLevel * maxParticles);

    glUseProgram(0);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
    glMatrixMode(GL_MODELVIEW);
    Matrix4f viewMatrix = graphicsState->viewMatrixF.getPointer();
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
	if (pointsOnly){
		glBegin(GL_POINTS);
		for (int i = 0; i < particlesToProcess; ++i){
			if (lifeDuration[i] >= lifeTime[i])
				continue;
			glColor4f(colors[i].x, colors[i].y, colors[i].z, colors[i].w * lifeDuration[i] / lifeTime[i]);
			Vector3f & p = positions[i];
			glVertex3f(p.x, p.y, p.z);
		}
		glEnd();
	}
	else {
		/// Set mipmap level too?
		int value;
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &value);
		// 9987 9729
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, diffuse->glid);
		Vector3f leftBase, upBase, left, up;
		leftBase = graphicsState->camera->LeftVector() * particleSize;
		upBase = graphicsState->camera->UpVector() * particleSize;
		glBegin(GL_QUADS);
		float optimizedAlpha = 1 / optimizationLevel;
		for (int i = 0; i < particlesToProcess; ++i){
			if (lifeDuration[i] >= lifeTime[i])
				continue;
			glColor4f(colors[i].x, colors[i].y, colors[i].z, 0.75f * optimizedAlpha * colors[i].w * 0.8f * pow((1.0f - lifeDuration[i] / lifeTime[i]), 4));
			float sizeRatio = pow(lifeDuration[i]+1.0f, 3.0f);
		//	if (lifeDuration[i] > 1.0f)
		//		sizeRatio = pow(5.0f, lifeDuration[i]-1.0f);
			left = leftBase * sizeRatio;
			up = upBase * sizeRatio;
			Vector3f & p = positions[i];

			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(p.x + left.x + up.x, p.y + left.y + up.y, p.z + left.z + up.z);

		//	glColor4f(colors[i].x , colors[i].y - 1.0f, colors[i].z - 1.0f, colors[i].w * lifeDuration[i] / lifeTime);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(p.x + left.x - up.x, p.y + left.y - up.y, p.z + left.z - up.z);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(p.x - left.x - up.x, p.y - left.y - up.y, p.z - left.z - up.z);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(p.x - left.x + up.x, p.y - left.y + up.y, p.z - left.z + up.z);
		}
		glEnd();
	}
}

void Exhaust::PrintData(){
}


void Exhaust::AttachTo(Entity * entity, Matrix4f relativePosition){
	assert(false);
}
