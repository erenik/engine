#include "Lighting.h"

#include <GL/glew.h>

#include <iostream>
#include <ctime>
#include <cstring>
#include <fstream>

#include "Shader.h"
#include "GraphicsState.h"

/// Default constructor, sets all light pointers and other variables to 0/NULL
Lighting::Lighting()
{
	global_ambient = Vector4f(1.0f, 1.0f, 1.0f, 0.1f);
	this->lastUpdate = GetTime();
	this->activeLight = NULL;
	this->activeLightIndex = 0;
	this->lightCounter = 0;
}
Lighting::~Lighting()
{
	lights.ClearAndDelete();
}

/// Copy constructor ^^
Lighting::Lighting(const Lighting& otherLighting)
{
	Copy(&otherLighting);
}

/// Assignment operator...
const Lighting * Lighting::operator = (const Lighting & otherLighting)
{
	Copy(&otherLighting);
	return this;
}

/// Used for all copy-constructors.
void Lighting::Copy(const Lighting * fromThisLighting)
{
	lights.ClearAndDelete();
	for (int i = 0; i < fromThisLighting->lights.Size(); ++i){
		Light * otherLight = fromThisLighting->lights[i];
		Light * newLight = new Light(*otherLight);
		lights.Add(newLight);
	}
	global_ambient = fromThisLighting->global_ambient;
	this->lastUpdate = fromThisLighting->lastUpdate;
	this->activeLight = NULL;	// Pointer, set to NULL!!!
	this->activeLightIndex = 0;	// Default to null
	this->lightCounter = fromThisLighting->lightCounter;
}

/// Adds light to the lighting, returns it's index on success, -1 on failure. o-o
Light * Lighting::Add(Light * newLight)
{
	if (lights.Size() >= MAX_LIGHTS)
		return NULL;
	Light * light = new Light(*newLight);
	light->currentlyActive = true;
	activeLight = light;
	/// Set default values
	this->lastUpdate = GetTime();
	lights.Add(light);
	return light;
}

/// Removes target light. Returns false on failure.
bool Lighting::Remove(Light * toRemove)
{
	return lights.Remove(toRemove);
}

/// Creates a default setup of 3-4 lights for testing purposes.
void Lighting::CreateDefaultSetup()
{
/*
	/// Ambient clear blue sky
	SetAmbient(64/500.0f, 156/500.0f, 255/500.0f);
	// First general sunlight
	CreateLight();
	SetType(LightType::DIRECTIONAL);
//	SetDiffuse(255/255.0f, 255/255.0f, 251/255.0f);
//	SetSpecular(255/255.0f, 255/255.0f, 251/255.0f);
	SetColor(255/255.0f, 255/255.0f, 251/255.0f);
	SetPosition(0,1.0f, 0.2f);
	SetName("Sun light");

	// Then.. a light bulb o-o something
	CreateLight();
	SetType(LightType::POSITIONAL);
	SetPosition(20.0f, 15.0f, 10.2f);
	SetColor(0.5f, 1.0f, 1.0f);
//	SetDiffuse(0.5f, 1.0f, 1.0f);
//	SetSpecular(0.5f, 1.0f, 1.0f);
	SetAttenuation(1.0f, 0.0005f, 0.0f);
	SetName("Light bulb.. something");

	// Then.. a light bulb o-o something
	CreateLight();
	SetType(LightType::POSITIONAL);
	SetPosition(-20.0f, 35.0f, 10.2f);
	SetColor(1.5f, 0.5f, 0.5f);
//	SetDiffuse(1.5f, 1.0f, 1.0f);
//	SetSpecular(1.5f, 1.0f, 1.0f);
	SetAttenuation(1.0f, 0.005f, 0.00001f);
	SetName("Light bulb.. something2");

	// Then.. a light bulb o-o something
	CreateLight();
	SetType(LightType::SPOTLIGHT);
	SetPosition(-10.0f, 5.0f, -50.2f);
	SetColor(0.3f, 0.3f, 3.0f);
//	SetDiffuse(1.5f, 1.0f, 1.0f);
//	SetSpecular(1.0f, 1.0f, 3.0f);
	SetSpotDirection(0.02f,-0.001f,1.0f);
	SetSpotCutoffExponent(45.0f, 5);
	SetAttenuation(1.0f, 0.000f, 0.000f);
	SetName("Spot light.. o-o");

	std::cout<<"\nDefault lights created.";
	*/
}
/// Returns all lights in their current state.
List<Light*> Lighting::GetLights() const
{
	List<Light*> constLightList;
	for (int i = 0; i < lights.Size(); ++i)
	{
		constLightList.Add(lights[i]);
	}
	return constLightList;
}

/// Fills the whole light-array, attempting to  test the limits of the hardware with as many lights as possible.
void Lighting::Overload()
{
	/*
	for (int i = 0; i < MAX_LIGHTS; ++i){
		int index = CreateLight();
		if (index == -1){
			std::cout<<"\nERROR: Unable to create further lights. Maximum lights reached!";
			return;
		}
		float r, g, b;
		r = rand()%100 / 1.0f;
		g = rand()%100 / 1.0f;
		b = rand()%100 / 1.0f;
		SetDiffuse(r,g,b);
		SetSpecular(r,g,b);
		SetPosition((float)(rand()%2200 - 1100) ,(float)(rand()%2200 - 1100), (float)(rand()%2200 - 1100));
		SetAttenuation(1.0f, 0.1f, 0.001f);
		SetType(LightType::POSITIONAL);
	}
	*/
}

/// Sets ambient using 0-255 values
void Lighting::SetAmbient(int r, int g, int b, int a){
	this->global_ambient = Vector4f(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	this->lastUpdate = GetTime();
}
/// Sets ambient using floats.
void Lighting::SetAmbient(float r, float g, float b, float a){
	this->global_ambient = Vector4f(r,g,b,a);
	this->lastUpdate = GetTime();
}
/// Sets ambient values. Alpha defaults to 0.
void Lighting::SetAmbient(Vector3f values){
	this->global_ambient = Vector4f(values);
	this->lastUpdate = GetTime();
}

/// Creates a new light source, adding it to the array. Returns the newly created light's index and sets it as the currently editable light.
Light * Lighting::CreateLight()
{
	if (lights.Size() >= MAX_LIGHTS)
		return NULL;
	activeLight = new Light();
	/// Set default values
	activeLight->diffuse = Vector4f(1,1,1,1);
	activeLight->specular = Vector4f(1,1,1,1);
	activeLight->attenuation = Vector3f(1.0f,0.0f,0.0f);
	// Set name
	String name = "Light " + String::ToString(lightCounter);
	++this->lightCounter;
	activeLight->SetName(name);
	this->lastUpdate = GetTime();
	lights.Add(activeLight);
	return activeLight;
}

/// Returns a pointer to selected light. USE WITH CAUTION.
Light * Lighting::GetLight(int index){
	assert(index >= 0 && "Invalid light-index!");
	return lights[index];
}

/// Deletes active light source or target light if argument is provided. Returns false if no light is selected or hte light did not belong to this lighting.
bool Lighting::DeleteLight(Light * light)
{	
	return lights.Remove(light);
}

/// Deletes all light sources contained within.
int Lighting::DeleteAllLights()
{
	int lightsDeleted = lights.Size();
	lights.ClearAndDelete();
	lastUpdate = clock();
	return lightsDeleted;
}


// Versions
#define LIGHTING_VERSION_0 0// Initial version.
int lightingVersion = LIGHTING_VERSION_0;

/// Writes to file stream.
void Lighting::WriteTo(std::fstream & file){
	// Write version
	file.write((char*)&lightingVersion, sizeof(int));
	// Write ambient
	global_ambient.WriteTo(file);
	// Write number of lights
	int numLights = lights.Size();
	file.write((char*)&numLights, sizeof(int));
	for (int i = 0; i < lights.Size(); ++i){
		lights[i]->WriteTo(file);
	}
}
/// Reads from file stream.
void Lighting::ReadFrom(std::fstream & file)
{
	// Write version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version == lightingVersion);
	// Write ambient
	global_ambient.ReadFrom(file);
	// Write number of lights
	int numLights = 0;
	file.write((char*)&numLights, sizeof(int));
	for (int i = 0; i < numLights; ++i){
		Light * light = new Light();
		light->ReadFrom(file);
		lights.Add(light);
	}
}


/// Loads selected lighting into the active shader program
bool LoadLighting(Lighting * lighting, Shader * shader)
{
	GLuint loc, error;
	if (!lighting)
		return false;
	if (!shader)
		return false;
//#define PRINT_DEBUG
	/// First get and set ambient
	loc = glGetUniformLocation(shader->shaderProgram, "global_ambient");
	if (loc != -1){
		shader->uniformLight.ambientVec4 = loc;
		GLfloat ambient[3];
		ambient[0] = lighting->global_ambient.x;
		ambient[1] = lighting->global_ambient.y;
		ambient[2] = lighting->global_ambient.z;
		glUniform3fv(shader->uniformLight.ambientVec4, 1, ambient);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting global ambient luminosity";
		}
	}
	else {
		std::cout<<"\nUnable to find uniform for global_ambient.";
		return false;
    }

	// Get remaining uniforms
	loc = glGetUniformLocation(shader->shaderProgram, "light_diffuse");
	if (loc != -1)
		shader->uniformLight.diffuseVec4 = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_specular");
	if (loc != -1)
		shader->uniformLight.specularVec4 = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_position");
	if (loc != -1)
		shader->uniformLight.positionVec3 = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_attenuation");
	if (loc != -1)
		shader->uniformLight.attenuationVec3 = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_type");
	if (loc != -1)
		shader->uniformLight.typeInt = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_spotDirection");
	if (loc != -1)
		shader->uniformLight.spotDirectionVec3 = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_spotCutoff");
	if (loc != -1)
		shader->uniformLight.spotCutoffFloat = loc;
	loc = glGetUniformLocation(shader->shaderProgram, "light_spotExponent");
	if (loc != -1)
		shader->uniformLight.spotExponentInt = loc;


	/// Gather all data
	static GLfloat diffuse[MAX_LIGHTS * 4];
	static GLfloat specular[MAX_LIGHTS * 4];
	static GLfloat position[MAX_LIGHTS * 3];
	static GLfloat attenuation[MAX_LIGHTS * 3];
	static GLint type[MAX_LIGHTS];
	static GLfloat spotDirection[MAX_LIGHTS * 3];
	static GLfloat spotCutoff[MAX_LIGHTS];
	static GLint spotExponent[MAX_LIGHTS];
	int activeLights = 0;
	for (int i = 0; i < lighting->lights.Size(); ++i)
	{
		Light * light = lighting->lights[i];
		if (!light->currentlyActive)
			continue;
		activeLights++;
		int interval = 4;
		diffuse[i*interval] = light->diffuse.x;
		diffuse[i*interval+1] = light->diffuse.y;
		diffuse[i*interval+2] = light->diffuse.z;
		diffuse[i*interval+3] = light->diffuse.w;

		specular[i*interval] = light->specular.x;
		specular[i*interval+1] = light->specular.y;
		specular[i*interval+2] = light->specular.z;
		specular[i*interval+3] = light->specular.w;

		interval = 3;
		position[i*interval] = light->position.x;
		position[i*interval+1] = light->position.y;
		position[i*interval+2] = light->position.z;

		attenuation[i*interval] = light->attenuation.x;
		attenuation[i*interval+1] = light->attenuation.y;
		attenuation[i*interval+2] = light->attenuation.z;

		type[i] = light->type;

		spotDirection[i*interval] = light->spotDirection.x;
		spotDirection[i*interval+1] = light->spotDirection.y;
		spotDirection[i*interval+2] = light->spotDirection.z;
		/// Calcualte cutoff as a cosine value of the degrees converted to radians before we throw it in ^^
		spotCutoff[i] = cos(light->spotCutoff / 180.0f * PI);
		spotExponent[i] = light->spotExponent;
	}

	/// Set all data
	/// Set Diffuse
	if (shader->uniformLight.diffuseVec4 != -1){
		glUniform4fv(shader->uniformLight.diffuseVec4, activeLights, diffuse);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light diffuse properties";
		}
	}/// Set Specular
	if (shader->uniformLight.specularVec4 != -1){
		glUniform4fv(shader->uniformLight.specularVec4, activeLights, specular);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light specular properties";
		}
	}
	/// Set Position
	if (shader->uniformLight.positionVec3 != -1){
		glUniform3fv(shader->uniformLight.positionVec3, activeLights, position);
		error = glGetError();
		if (error != GL_NO_ERROR){
			// std::cout<<"\nGL_ERROR: Error setting light position properties";
		}
	}
	/// Set attenuation
	if (shader->uniformLight.attenuationVec3 != -1){
		glUniform3fv(shader->uniformLight.attenuationVec3, activeLights, attenuation);
		error = glGetError();
		if (error != GL_NO_ERROR){
			//std::cout<<"\nGL_ERROR: Error setting light attenuation properties";
		}
	}
	/// Set Type
	if (shader->uniformLight.typeInt != -1){
		glUniform1iv(shader->uniformLight.typeInt, activeLights, type);
		error = glGetError();
		if (error != GL_NO_ERROR){
			//std::cout<<"\nGL_ERROR: Error setting light type";
		}
	}
	/// Set spotlight attributes
	if (shader->uniformLight.spotDirectionVec3 != -1){
		glUniform3fv(shader->uniformLight.spotDirectionVec3, activeLights, spotDirection);
	}
	if (shader->uniformLight.spotCutoffFloat != -1){
		glUniform1fv(shader->uniformLight.spotCutoffFloat, activeLights, spotCutoff);
	}
	if (shader->uniformLight.spotExponentInt != -1){
		glUniform1iv(shader->uniformLight.spotExponentInt, activeLights, spotExponent);
	}
	error = glGetError();
	if (error != GL_NO_ERROR){
		// std::cout<<"\nGL_ERROR: Error setting spotlight attributes";
	}
	/// Set amount of lights
	loc = glGetUniformLocation(shader->shaderProgram, "activeLights");
	if (loc != -1) {
		error = glGetError();
		glUniform1i(loc, activeLights);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting amount of active lights";
		}
	}
	else
		std::cout<<"\nUnable to find uniform for activeLights.";
	return true;
}
