/// Emil Hedemalm
/// 2014-07-10
/// OpenGL-based Shader class.

#include "Shader.h"

#include "Graphics/OpenGL.h"
#include "Globals.h"

#include <iostream>
#include <fstream>
//#include "Macros.h"

/// Initialize a SHADARRR
const int LOG_MAX = 2048;
char shaderLog[LOG_MAX];
	
/// 
ShaderPart::ShaderPart(int type)
: type(type)
{
	shaderKernelPart = -1;
}

void ShaderPart::Delete()
{
	if (shaderKernelPart){
		int deleteStatus = -1;
		glGetShaderiv(shaderKernelPart, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
			glDeleteShader(shaderKernelPart);
		shaderKernelPart = NULL;
	}
}

// Loads from source.
bool ShaderPart::Load(String fromSource)
{
	CreateShaderKernelIfNeeded();
	source = fromSource;
	char * data;
	int size = 0, start = 0;
	String vertSource, fragSource;
	try {
		// Open file
		std::fstream file;
		file.open(source.c_str(), std::ios_base::in);
		if (!file.is_open()){
		    std::cout<<"\nUnable to open stream to file \""<<source<<"\"";
		    return false;
		}
		// Extract length of file.
		start = (int)file.tellg();
		file.seekg(0, std::ios::end);
		size = ((int)file.tellg()) - start;
		file.seekg(0, std::ios::beg);
		// Allocate and read data.
		data = new char [size+5];
		memset(data, 0, size+5);
		file.read(data, size);
		// Close file again
		file.close();

		/// Add a null-sign ffs!
		data[size] = 0;

		if (file.fail()){

			if (file.fail())
				file.clear(std::ios_base::failbit);
			if (file.eof())
				file.clear(std::ios_base::eofbit);
			if (file.bad())
				file.clear(std::ios_base::badbit);
			if (file.good())

				// Reset bits
				int state = file.exceptions();
		}

		/* Associate the source code buffers with each handle */
		glShaderSource(shaderKernelPart, 1, (const GLchar **) &data, 0);
        // Copy data first!
		vertSource = String(data);
		// Deallocate data again
		delete[] data;
		data = NULL;
	} catch (...) {
		std::cout<<"\nFile I/O Error: Failed to read shader source from "<<source;
		return false;
	}
	return true;
}

void ShaderPart::CreateShaderKernelIfNeeded()
{
	if (shaderKernelPart == -1)
	{
		switch(type)
		{
			case ShaderType::VERTEX_SHADER:
				shaderKernelPart = glCreateShader(GL_VERTEX_SHADER);
				break;
			case ShaderType::FRAGMENT_SHADER:
				shaderKernelPart = glCreateShader(GL_FRAGMENT_SHADER);
				break;
		}
	}
}
// Copmpiles it, returning the result.
bool ShaderPart::Compile()
{
	CreateShaderKernelIfNeeded();
	
	/* Compile our shader objects */
	glCompileShader(shaderKernelPart);
	
	// Check if compilation failed.
	int status = NULL;
	glGetShaderiv(shaderKernelPart, GL_COMPILE_STATUS, &status);
	// If compilation failed, extract info log o-o
	int resultLength = 0;
	if (status == GL_FALSE){
		glGetShaderInfoLog(shaderKernelPart, LOG_MAX, &resultLength, shaderLog);
		if (resultLength > 0){
			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nCompilation Error: ";
			cError += source;
			cError += ": Failed compiling vertex shader: \n";
			cError += shaderLog;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderCompilationError_VertexShader_";
			dumpFileName += name;
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();
			return false;
		}
	}
	return true;
}

Shader::Shader()
{
	/// GL ids
	shaderProgram = id = 0;
	vertexShader = fragmentShader = NULL;
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

Shader::~Shader()
{
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
	for (int i = 0; i < shaderParts.Size(); ++i)
	{
		ShaderPart * sp = shaderParts[i];
		sp->Delete();
	}
	if (shaderProgram){
		int deleteStatus = -1;
		glGetProgramiv(shaderProgram, GL_DELETE_STATUS, &deleteStatus);
		if (deleteStatus == GL_FALSE)
    		glDeleteProgram(shaderProgram);
		shaderProgram = NULL;
	}
	vertexShader = fragmentShader = 0;
}

// Attempts to compile this shader. Calls ExtractUniforms automatically upon completion.
bool Shader::Compile()
{
	int i = 0;

    if (GL_VERSION_MAJOR < 2){
        std::cout<<"\nOpenGL version below 2.0! Aborting recompilation procedure!";
        return false;
    }

	if (shaderParts.Size() == 0)
	{
		vertexShader = new ShaderPart(ShaderType::VERTEX_SHADER);
		shaderParts.Add(vertexShader);
		fragmentShader = new ShaderPart(ShaderType::FRAGMENT_SHADER);
		shaderParts.Add(fragmentShader);
	}

	std::cout<<"\nRecompiling shader: "<<name;

	// Check if shaders and program have to be created first
	//if (!vertexShader)
	//	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//if (!fragmentShader)
	//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!shaderProgram)
		shaderProgram = glCreateProgram();

	// First read in our vertex shader!
	if (!vertexShader->Load(vertexSource))
		return false;
	if (!fragmentShader->Load(fragmentSource))
		return false;
	
	vertexShader->Compile();
	fragmentShader->Compile();
	
	/* Attach our shaders to our program */
	glAttachShader(shaderProgram, vertexShader->shaderKernelPart);
	glAttachShader(shaderProgram, fragmentShader->shaderKernelPart);
	GLuint error = glGetError();
	
	/* Bind attribute index 0 to vertex positions, attribute index 1 to incoming UV coordinates and index 2 to Normals. */
	glBindAttribLocation(shaderProgram, 0, "in_Position");
	glBindAttribLocation(shaderProgram, 1, "in_UV");
	glBindAttribLocation(shaderProgram, 2, "in_Normal");
	glBindAttribLocation(shaderProgram, 3, "in_Tangent");

	GLuint attrib = glGetAttribLocation(shaderProgram, "in_Position");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_Position!";
	}
	attrib = glGetAttribLocation(shaderProgram, "in_UV");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_UV!";
	}
	attrib = glGetAttribLocation(shaderProgram, "in_Normal");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_Normal!";
	}
	attrib = glGetAttribLocation(shaderProgram, "in_Tangent");
	if (attrib == -1){
		std::cout<<"\nUnable to get attribute location for in_Tangent!";
	}

	error = glGetError();

	/* Link our program, and set it as being actively used */
	glLinkProgram(shaderProgram);
	// If compilation failed, extract info log o-o
	GLint linkStatus;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
	int resultLength = 0;
	if (linkStatus == GL_FALSE){

		glGetProgramInfoLog(shaderProgram, LOG_MAX, &resultLength, shaderLog);
		if (resultLength > 0){

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			String cError = "\nLinker Error: ";
			cError += name;
			cError += ": Failed linking shader: \n";
			cError += shaderLog;

			std::cout<<cError;
			glGetError();	// Release any eventual errors
			
			std::fstream dump;
			String dumpFileName = "ShaderLinkerError_FragmentShader_";
			dumpFileName += name;
			dump.open(dumpFileName.c_str(), std::ios_base::out);
			if (dump.is_open()){
				dump << cError;
			}
			dump.close();

			// Just return if compilation fails for now. Enter a warning here or debug breakpoint if needed ^^
			std::cout<<cError;

			int error  = glGetError();	// Release any eventual errors
			glDetachShader(shaderProgram, vertexShader->shaderKernelPart);
			glDetachShader(shaderProgram, fragmentShader->shaderKernelPart);
			return false;
		}
	}
	std::cout<<"\n"<<name<<" Shader Compilation Successful!\n";

	// Extract uniforms.
	ExtractUniforms();

	// Enable rendering now if it wasn't already
	built = true;
	return true;
}


/// Extracts all uniforms required to set/adjust settings in the shader later on.
void Shader::ExtractUniforms()
{
	// Matrices

	// Get Identifier pointers to the GL uniform projection/model matrices, once! o-o
	uniformModelMatrix = glGetUniformLocation(shaderProgram, "modelMatrix");
	uniformViewMatrix = glGetUniformLocation(shaderProgram, "viewMatrix");
	uniformProjectionMatrix = glGetUniformLocation(shaderProgram, "projectionMatrix");
	if (uniformProjectionMatrix == -1){
		std::cout<<"\nUnable to find uniform for Projection Matrix";
	}
	if (uniformViewMatrix == -1){
		std::cout<<"\nUnable to find uniform for View Matrix";
	}
	if (uniformModelMatrix == -1){
		std::cout<<"\nUnable to find uniform for Model Matrix";
	}

	// Get camera uniforms ^^
	uniformEyePosition = glGetUniformLocation(shaderProgram, "eyePosition");
	if (uniformEyePosition == -1){
		std::cout<<"\nUnable to find uniform for Eye Position";
	}

	// Get textures
	// First diffuse
	glActiveTexture(GL_TEXTURE0 + 0);
	uniformBaseTexture = glGetUniformLocation(shaderProgram, "baseImage");
	if (uniformBaseTexture == -1){
		uniformBaseTexture = glGetUniformLocation(shaderProgram, "diffuseMap");
		if (uniformBaseTexture == -1)
			std::cout<<"\nUniformBaseTexture \"baseImage/diffuseMap\" could not be located and set!";
	}
	// Specular map
	glActiveTexture(GL_TEXTURE0 + 1);
	uniformSpecularMap = glGetUniformLocation(shaderProgram, "specularMap");
	if (uniformSpecularMap == -1)
		std::cout<<"\nUniformSpecularMap \"specularMap\" could not be located and set!";
	// Normal map
	glActiveTexture(GL_TEXTURE0 + 2);
	uniformNormalMap = glGetUniformLocation(shaderProgram, "normalMap");
	if (uniformNormalMap == -1)
		std::cout<<"\nUniformNormalMap \"normalMap\" could not be located and set!";
	glActiveTexture(GL_TEXTURE0 + 0);

	/// Set the using uniforms to -1.
	uniformUseDiffuseMap = glGetUniformLocation(shaderProgram, "useDiffuseMap");
	uniformUseSpecularMap = glGetUniformLocation(shaderProgram, "useSpecularMap");
	uniformUseNormalMap = glGetUniformLocation(shaderProgram, "useNormalMap");

	/// "Primary color"
	uniformPrimaryColorVec4 = glGetUniformLocation(shaderProgram, "primaryColorVec4");
	if (uniformPrimaryColorVec4 == -1)
		std::cout<<"\nUniform \"primaryColorVec4\" could not be located and set.";
	// Highlight color. Mainly used for UI.
	uniformHighlightColorVec4 = glGetUniformLocation(shaderProgram, "highlightColorVec4");

	// Foggy fog-some!
	uniformFogBeginDistance = glGetUniformLocation(shaderProgram, "fogBegin");
	uniformFogEndDistance = glGetUniformLocation(shaderProgram, "fogEnd");
	uniformFogColor = glGetUniformLocation(shaderProgram, "fogColor");


	//// Material properties 
	uniformMaterial.ambientVec4 = glGetUniformLocation(shaderProgram, "materialAmbient");
	uniformMaterial.diffuseVec4 = glGetUniformLocation(shaderProgram, "materialDiffuse");
	uniformMaterial.specularVec4 = glGetUniformLocation(shaderProgram, "materialSpecular");
	uniformMaterial.shininessInt = glGetUniformLocation(shaderProgram, "materialShininess");

	// Lighting uniforms.
	uniformLight.ambientVec4 = glGetUniformLocation(shaderProgram, "global_ambient");
	if (uniformLight.ambientVec4 == -1)
		uniformLight.ambientVec4 = glGetUniformLocation(shaderProgram, "light_ambient");
	uniformLight.diffuseVec4 = glGetUniformLocation(shaderProgram, "light_diffuse");
	uniformLight.specularVec4 = glGetUniformLocation(shaderProgram, "light_specular");

	// Light detail uniforms
	uniformLight.diffuseVec4 = glGetUniformLocation(shaderProgram, "light_diffuse");
	uniformLight.specularVec4 = glGetUniformLocation(shaderProgram, "light_specular");
	uniformLight.positionVec3 = glGetUniformLocation(shaderProgram, "light_position");
	uniformLight.attenuationVec3 = glGetUniformLocation(shaderProgram, "light_attenuation");
	uniformLight.typeInt = glGetUniformLocation(shaderProgram, "light_type");
	uniformLight.spotDirectionVec3 = glGetUniformLocation(shaderProgram, "light_spotDirection");
	uniformLight.spotCutoffFloat = glGetUniformLocation(shaderProgram, "light_spotCutoff");
	uniformLight.spotExponentInt = glGetUniformLocation(shaderProgram, "light_spotExponent");
	
}

