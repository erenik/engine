#include "Shader.h"

#include "Globals.h"
#include <iostream>
//#include "Macros.h"

Shader::Shader(){
	/// GL ids
	shaderProgram = vertexShader = fragmentShader = id = 0;

	source[0] = source[1] = name = NULL;
	built = false;
	uniformModelMatrix = -1;
	uniformViewMatrix = -1;
	uniformProjectionMatrix = -1;

	uniformEyePosition = -1;

	for (int i = 0; i < 10; ++i)
		sampler[i] = -1;
	
	uniformBaseTexture = -1;
	uniformSpecularMap = -1;
	uniformNormalMap = -1;

	/// Set the using uniforms to -1.
	uniformUseDiffuseMap = uniformUseSpecularMap = uniformUseNormalMap = -1;

	// Foggy fog-some!
	uniformFogBeginDistance = uniformFogEndDistance = uniformFogColor = -1;
}

Shader::~Shader(){
	if (shaderProgram){
		std::cout<<"WARNING: "<<name<<" Shader program not deallocated!";
		assert(false && "shaderProgram not deallocated!");
	}
	if (vertexShader){
		std::cout<<"WARNING: "<<name<<" Vertex shader not deallocated!";
		assert(false && "vertexShader not deallocated!");
	}
	if (fragmentShader){
		std::cout<<"WARNING: "<<name<<" Fragment shader not deallocated!";
		assert(false && "fragmentShader not deallocated!");
	}
}

void Shader::DeleteShader()
{
	if (shaderProgram){
		int deleteStatus = -1;
		glGetProgramiv(shaderProgram, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
    		glDeleteProgram(shaderProgram);
		shaderProgram = NULL;
	}
	if (vertexShader){
		int deleteStatus = -1;
		glGetShaderiv(vertexShader, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
			glDeleteShader(vertexShader);
		vertexShader = NULL;
	}
	if (fragmentShader){
		int deleteStatus = -1;
		glGetShaderiv(fragmentShader, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
			glDeleteShader(fragmentShader);
		fragmentShader = NULL;
	}
}
