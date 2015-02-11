// Emil Hedemalm
// 2013-07-03 Linuxification.

#include "ShaderManager.h"
#include "Globals.h"

#include "Graphics/OpenGL.h"
#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"

#include <fstream>
#include <cstring>
#include <cassert>


ShaderManager * ShaderManager::shaderManager = NULL;

ShaderManager::ShaderManager()
{
	activeShader = NULL;
}
ShaderManager::~ShaderManager()
{
	shaders.ClearAndDelete();
}
	
void ShaderManager::Allocate()
{
	assert(shaderManager == 0);
	shaderManager = new ShaderManager();
}
void ShaderManager::Deallocate()
{
	assert(shaderManager);
	delete shaderManager;
	shaderManager = NULL;
}
ShaderManager * ShaderManager::Instance()
{
	assert(shaderManager);
	return shaderManager;
}


/// Loads and compiles all relevant shaders
void ShaderManager::CreateDefaultShaders()
{
	/// Load Shaders! Either from file or built in the executable (?)
	CreateShader("UI", SHADER_REQUIRED);
	/*
	CreateShader("Flat", SHADER_REQUIRED);
	CreateShader("Depth");
	CreateShader("Normals");
	CreateShader("Lighting");
	CreateShader("Phong");
	CreateShader("Sprite");
	/// Don't even create the defered shader if it can't compile, yo..
	if (GL_VERSION_MAJOR >= 3)
		CreateShader("Deferred");
	CreateShader("Wireframe");
	*/

	RecompileAllShaders();
}


/// Creates a shader using specified path/name .vert and .frag and returns a pointer to it
Shader * ShaderManager::CreateShader(const char * name, int i_flags)
{
	for (int i = 0; i < shaders.Size(); ++i){
		if (shaders[i]->name == name){
            std::cout<<"\nShader already exists! Returning it ^^";
            return shaders[i];
		}
	}

    std::cout<<"\nCreating shader "<<name;
	Shader * shader = new Shader();
	shader->name = name;
	shader->flags = i_flags;

	String shaderDir = "shaders/";
	shader->vertexSource = shaderDir + name + ".vert";
	shader->fragmentSource = shaderDir + name + ".frag";

	shaders.Add(shader);
	return shader;
}

void ShaderManager::DeleteShaders()
{
	/// Unbind currently bound shader program so that it may be deallocated.
	ShadeMan.SetActiveShader(0);
	// Delete 'em.
    while(shaders.Size()){
        Shader * shader = shaders[0];
		shader->DeleteShader();
		shaders.RemoveIndex(0);
		delete shader;
    }
}


/// Returns a shader program with the given name
Shader * ShaderManager::GetShader(String name)
{
	for (int i = 0; i < shaders.Size(); ++i)
	{
	    Shader * shader = shaders[i];
		if (shader->name == 0)
			continue;
		if(name == shader->name)
			return shader;
	}
	// Try to load it if it doesn't exist.
	Shader * shader = CreateShader(name);
	if (RecompileShader(shader))
		return shader;
	if (shader == NULL)
	{
		assert(shader && "No such shader, yo?");
		std::cout<<"\nTried to find shader, but it was ze unsuccessful D:";
		return NULL;
	}
	return NULL;
}

/// Recompiles target shader
bool ShaderManager::RecompileShader(Shader * shader)
{
	return shader->Compile();
}

/// Attempts to recompile all shaders
void ShaderManager::RecompileAllShaders()
{
    if (GL_VERSION_MAJOR < 2){
        std::cout<<"\nOpenGL version below 2.0! Aborting recompilation procedure!";
        return;
    }
	for (int i = 0; i < shaders.Size(); ++i){
		RecompileShader(shaders[i]);
    }
}

/// Reload light data into all shaders that need the update.
bool informed = false;
void ShaderManager::ReloadLights(Lighting * newLighting)
{
	if (!informed){
		std::cout<<"\nWARNING: Implement ShaderManager::ReloadLights!";
		informed = true;
	}
	return;
	if (newLighting == NULL){
		std::cout<<"";
	}
}


/// Sets selected shader to be active. Prints out error information and does not set to new shader if it fails.
Shader * ShaderManager::SetActiveShader(String shaderName)
{
	Shader * shader = GetShader(shaderName);
	// Try to load it if it doesn't exist.
	if (shader == NULL){
		// Try and load it, yo.
		shader = CreateShader(shaderName);
		RecompileShader(shader);
		if (shader == NULL){
			assert(shader && "No such shader, yo?");
			std::cout<<"\nTried to find shader, but it was ze unsuccessful D:";
			return NULL;
		}
	}
	return SetActiveShader(shader);
}

/// Sets selected shader to be active. Prints out error information and does not set to new shader if it fails.
Shader * ShaderManager::SetActiveShader(Shader * shader)
{
	CheckGLError("Before ShaderManager::SetActiveShader");

	/// Same shader, no state changes required.
	if (shader == activeShader)
		return shader;
	
	/// If we had a previously active shader, disable their specific vertex attribute pointers.
	if (activeShader)
	{
		activeShader->OnMadeInactive();
	}

	if(GL_VERSION_MAJOR < 2){
		std::cout<<"\nERROR: Unable to set shader program as GL major version is below 2.";
        return NULL;
	}
	/// Can request default shader too, so ^^
	if (shader == NULL)
	{
		glUseProgram(0);
		activeShader = 0;
		return NULL;
	}
	else 
	{
		// Compile it if not done already?
		if (!shader->built)
		{
			Time mostRecentEdit = shader->MostRecentEdit();
			if (shader->lastCompileAttempt < mostRecentEdit)
				shader->Compile();
		}
	}
	// Ensure it was built correctly.
	if (shader && shader->built)
	{
		glUseProgram(shader->shaderProgram);
		CheckGLError("ShaderManager::glUseProgram");
		activeShader = shader;
		// Enable respective vertex array/attribute pointers.
		shader->OnMadeActive();
		CheckGLError("ShaderManager::SetActiveShader");
		return shader;
	}
	else {
		/// No shader was found, return NULL that default shader is now in use.
		glUseProgram(0);
		activeShader = 0;
	}
	return NULL;
}


/// Returns the active shader, or NULL if the default shading program is in use.
Shader * ShaderManager::ActiveShader()
{
	return activeShader;
}

