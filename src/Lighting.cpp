#include "Lighting.h"

#include <GL/glew.h>

#include <iostream>
#include <ctime>
#include <cstring>
#include <fstream>

#include "Shader.h"
#include "GraphicsState.h"

/// Default constructor, sets all light pointers and other variables to 0/NULL
Lighting::Lighting(){
	global_ambient = Vector4f(0.1f,0.1f,0.1f,0.1f);
	for (int i = 0; i < MAX_LIGHTS; ++i){
		light[i] = new Light();
		light[i]->currentlyActive = false;
	}
	this->activeLights = 0;
	this->lastUpdate = GetTime();
	this->activeLight = light[0];
	this->activeLightIndex = 0;
	this->lightCounter = 0;
	VerifyData();
}
Lighting::~Lighting(){
	for (int i = 0; i < MAX_LIGHTS; ++i){
		if (light[i])
			delete light[i];
		light[i] = NULL;
	}
}

// Verifies that some basic looks good.
void Lighting::VerifyData() const {
	assert(abs(light[0]->lastUpdate) < 500000);
}

/// Copy constructor ^^
Lighting::Lighting(const Lighting& otherLighting){
	otherLighting.VerifyData();
	for (int i = 0; i < MAX_LIGHTS; ++i){
		Light * otherLight = otherLighting.light[i];
		assert(abs(otherLight->lastUpdate) < 500000);
		light[i] = new Light(*otherLight);
	}
	int sizeofLight = sizeof(light);
	global_ambient = otherLighting.global_ambient;
	this->activeLights = otherLighting.activeLights;
	this->lastUpdate = otherLighting.lastUpdate;
	this->activeLight = NULL;	// Pointer, set to NULL!!!
	this->activeLightIndex = 0;	// Default to null
	this->lightCounter = otherLighting.lightCounter;
	VerifyData();
}

/// Assignment operator...
const Lighting * Lighting::operator = (const Lighting & otherLighting){
	otherLighting.VerifyData();
	for (int i = 0; i < MAX_LIGHTS; ++i){
		Light * otherLight = otherLighting.light[i];
		assert(abs(otherLight->lastUpdate) < 500000);
		if (light[i])
			delete light[i];
		light[i] = new Light(*otherLight);
	}
	int sizeofLight = sizeof(light);
	global_ambient = otherLighting.global_ambient;
	this->activeLights = otherLighting.activeLights;
	this->lastUpdate = otherLighting.lastUpdate;
	this->activeLight = NULL;	// Pointer, set to NULL!!!
	this->activeLightIndex = 0;	// Default to null
	this->lightCounter = otherLighting.lightCounter;
	VerifyData();
	return this;
}

/// Adds light to the lighting, returns it's index on success, -1 on failure. o-o
int Lighting::Add(Light & newLight){
	for (int i = 0; i < MAX_LIGHTS; ++i){
		if (light[i]->currentlyActive)
			continue;
		light[i]->currentlyActive = true;
		*light[i] = Light(newLight);
		activeLightIndex = i;
		activeLight = light[i];
		/// Set default values
		this->lastUpdate = GetTime();
		++activeLights;
		return i;
	}
	return -1;
}

/// Adds light to the lighting, returns it's old index on success, -1 on failure. o-o
int Lighting::Remove(Light & toRemove){
	for (int i = 0; i < MAX_LIGHTS; ++i){
		if (!light[i]->Name().Equals(toRemove.Name()))
			continue;
		SelectLight(i);
		DeleteLight();
		return i;
	}
	return -1;
}

/// Creates a default setup of 3-4 lights for testing purposes.
void Lighting::CreateDefaultSetup(){
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
}
/// Fills the whole light-array, attempting to  test the limits of the hardware with as many lights as possible.
void Lighting::Overload(){
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
int Lighting::CreateLight(){
	for (int i = 0; i < MAX_LIGHTS; ++i){
		if (light[i]->currentlyActive)
			continue;
		light[i]->currentlyActive = true;
		activeLightIndex = i;
		activeLight = light[i];
		/// Set default values
		activeLight->diffuse = Vector4f(1,1,1,1);
		activeLight->specular = Vector4f(1,1,1,1);
		activeLight->attenuation = Vector3f(1.0f,0.0f,0.0f);
		// Set name
		String name = "Light " + String::ToString(lightCounter);
		++this->lightCounter;
		activeLight->SetName(name);
		this->lastUpdate = GetTime();
		++activeLights;
		return i;
	}
	return -1;
}

/// Returns a pointer to selected light. USE WITH CAUTION.
Light * Lighting::GetLight(int index){
	assert(index >= 0 && index < this->activeLights && "Invalid light-index!");
	return light[index];
}

/// Deletes active light source. Returns false if no light is selected.
bool Lighting::DeleteLight(){
	if (activeLight){
		activeLight->currentlyActive = false;
		for (int i = 0; i < this->activeLights; ++i){
			// Move front all the remaining lights
			if (activeLight == light[i]){
				Light * tmp = light[i];
				for (int j = i; j < this->activeLights-1; ++j){
					light[j] = light[j+1];
				}
				light[this->activeLights-1] = tmp;
				--this->activeLights;
				lastUpdate = clock();
				return true;
			}
		}
	}
	return false;
}

/// Deletes all light sources contained within.
int Lighting::DeleteAllLights(){
	for (int i = 0; i < this->activeLights; ++i){
		light[i]->currentlyActive = false;
	}
	int lightsDeleted = this->activeLights;
	this->activeLights = 0;
	lastUpdate = clock();
	return lightsDeleted;
}


/// Selects indexed light. Returns 1 upon success, -1 upon failure.
int Lighting::SelectLight(int index){
	if (light[index]->currentlyActive){
		activeLightIndex = index;
		activeLight = light[index];
		return index;
	}
	return -1;
}

/// Selects light by name. Returns it's index upon success, -1 upon failure.
int Lighting::SelectLight(const char * name){
	for (int i = 0; i < MAX_LIGHTS; ++i){
		int result = strcmp(name, light[i]->Name());
		if (result == 0){
			activeLightIndex = i;
			activeLight = light[i];
			return i;
		}
	}
	return -1;
}

/// Sets diffuse values for selected light
void Lighting::SetDiffuse(float r, float g, float b, float a){
	activeLight->diffuse = Vector4f(r,g,b,a);
	this->lastUpdate = GetTime();
}
/// Sets specular values for selected light
void Lighting::SetSpecular(float r, float g, float b, float a){
	activeLight->specular = Vector4f(r,g,b,a);
	this->lastUpdate = GetTime();
}
/// Sets diffuse & specular values for active light
void Lighting::SetColor(float r, float g, float b, float a){
	activeLight->diffuse = Vector4f(r,g,b,a);
	activeLight->specular = Vector4f(r,g,b,a);
	this->lastUpdate = GetTime();
}

/// Sets attenuation factors for the selected light
void Lighting::SetAttenuation(float constant, float linear, float quadratic){
	ACTIVE_LIGHT->attenuation = Vector3f(constant, linear, quadratic);
	this->lastUpdate = GetTime();
}
/// Sets position for the light
void Lighting::SetPosition(float x, float y, float z){
	ACTIVE_LIGHT->position = Vector3f(x,y,z);
	this->lastUpdate = GetTime();
}

/// Sets spotlight direction in world coordinates
void Lighting::SetSpotDirection(float x, float y, float z){
	ACTIVE_LIGHT->spotDirection = Vector3f(x,y,z);
	this->lastUpdate = GetTime();
}
/// Sets spotlight cutoff in degrees and exponent for edge-fading.
void Lighting::SetSpotCutoffExponent(float cutoff, int exponent){
	activeLight->spotCutoff = cutoff;
	activeLight->spotExponent = exponent;
	this->lastUpdate = GetTime();
}

/// Sets light-type
void Lighting::SetType(int type){
	if (type <= LightType::NULL_TYPE || type >= LightType::LIGHT_TYPES){
		std::cout<<"\nWARNING: Invalid light-type provided!";
		return;
	}
	activeLight->type = type;
	this->lastUpdate = GetTime();
}

/// Sets new name for the active light
void Lighting::SetName(const char * newName){
	activeLight->SetName(newName);
}

/// Gets name of selected light.
const char * Lighting::GetName(int index){
	return light[index]->Name();
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
	file.write((char*)&activeLights, sizeof(int));
	for (int i = 0; i < activeLights; ++i){
		light[i]->WriteTo(file);
	}
}
/// Reads from file stream.
void Lighting::ReadFrom(std::fstream & file){
	// Write version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version == lightingVersion);
	// Write ambient
	global_ambient.ReadFrom(file);
	// Write number of lights
	file.write((char*)&activeLights, sizeof(int));
	for (int i = 0; i < activeLights; ++i){
		light[i]->ReadFrom(file);
	}
}


/// Loads selected lighting into the active shader program
bool LoadLighting(Lighting * lighting, GraphicsState &state){
	GLuint loc, error;
	if (!lighting)
		return false;
	if (!state.activeShader)
		return false;
//#define PRINT_DEBUG
	/// First get and set ambient
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "global_ambient");
	if (loc != -1){
		state.activeShader->uniformLight.ambientVec4 = loc;
		GLfloat ambient[3];
		ambient[0] = lighting->global_ambient.x;
		ambient[1] = lighting->global_ambient.y;
		ambient[2] = lighting->global_ambient.z;
		glUniform3fv(state.activeShader->uniformLight.ambientVec4, 1, ambient);
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
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_diffuse");
	if (loc != -1)
		state.activeShader->uniformLight.diffuseVec4 = loc;
	else
		std::cout<<"\nUnable to find uniform for light_diffuse.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_specular");
	if (loc != -1)
		state.activeShader->uniformLight.specularVec4 = loc;
	else
		std::cout<<"\nUnable to find uniform for light_specular.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_position");
	if (loc != -1)
		state.activeShader->uniformLight.positionVec3 = loc;
	else
		std::cout<<"\nUnable to find uniform for light_position.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_attenuation");
	if (loc != -1)
		state.activeShader->uniformLight.attenuationVec3 = loc;
	else
		std::cout<<"\nUnable to find uniform for light_attenuation.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_type");
	if (loc != -1)
		state.activeShader->uniformLight.typeInt = loc;
	else
		std::cout<<"\nUnable to find uniform for light_type.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_spotDirection");
	if (loc != -1)
		state.activeShader->uniformLight.spotDirectionVec3 = loc;
	else
		std::cout<<"\nUnable to find uniform for light_spotDirection.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_spotCutoff");
	if (loc != -1)
		state.activeShader->uniformLight.spotCutoffFloat = loc;
	else
		std::cout<<"\nUnable to find uniform for light_spotCutoff.";
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "light_spotExponent");
	if (loc != -1)
		state.activeShader->uniformLight.spotExponentInt = loc;
	else
		std::cout<<"\nUnable to find uniform for light_spotExponent.";


	/// Gather all data
	static GLfloat diffuse[MAX_LIGHTS * 4];
	static GLfloat specular[MAX_LIGHTS * 4];
	static GLfloat position[MAX_LIGHTS * 3];
	static GLfloat attenuation[MAX_LIGHTS * 3];
	static GLint type[MAX_LIGHTS];
	static GLfloat spotDirection[MAX_LIGHTS * 3];
	static GLfloat spotCutoff[MAX_LIGHTS];
	static GLint spotExponent[MAX_LIGHTS];
	for (int i = 0; i < lighting->activeLights; ++i){
		Light * light = lighting->light[i];
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
	if (state.activeShader->uniformLight.diffuseVec4 != -1){
		glUniform4fv(state.activeShader->uniformLight.diffuseVec4, lighting->activeLights, diffuse);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light diffuse properties";
		}
	}/// Set Specular
	if (state.activeShader->uniformLight.specularVec4 != -1){
		glUniform4fv(state.activeShader->uniformLight.specularVec4, lighting->activeLights, specular);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light specular properties";
		}
	}
	/// Set Position
	if (state.activeShader->uniformLight.positionVec3 != -1){
		glUniform3fv(state.activeShader->uniformLight.positionVec3, lighting->activeLights, position);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light position properties";
		}
	}
	/// Set attenuation
	if (state.activeShader->uniformLight.attenuationVec3 != -1){
		glUniform3fv(state.activeShader->uniformLight.attenuationVec3, lighting->activeLights, attenuation);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light attenuation properties";
		}
	}
	/// Set Type
	if (state.activeShader->uniformLight.typeInt != -1){
		glUniform1iv(state.activeShader->uniformLight.typeInt, lighting->activeLights, type);
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting light type";
		}
	}
	/// Set spotlight attributes
	if (state.activeShader->uniformLight.spotDirectionVec3 != -1){
		glUniform3fv(state.activeShader->uniformLight.spotDirectionVec3, lighting->activeLights, spotDirection);
	}
	if (state.activeShader->uniformLight.spotCutoffFloat != -1){
		glUniform1fv(state.activeShader->uniformLight.spotCutoffFloat, lighting->activeLights, spotCutoff);
	}
	if (state.activeShader->uniformLight.spotExponentInt != -1){
		glUniform1iv(state.activeShader->uniformLight.spotExponentInt, lighting->activeLights, spotExponent);
	}
	error = glGetError();
	if (error != GL_NO_ERROR){
		std::cout<<"\nGL_ERROR: Error setting spotlight attributes";
	}
	/// Set amount of lights
	loc = glGetUniformLocation(state.activeShader->shaderProgram, "activeLights");
	if (loc != -1) {
		error = glGetError();
		glUniform1i(loc, lighting->activeLights);	// 2 lights!
		error = glGetError();
		if (error != GL_NO_ERROR){
			std::cout<<"\nGL_ERROR: Error setting amount of active lights";
		}
	}
	else
		std::cout<<"\nUnable to find uniform for activeLights.";
	return true;
}
